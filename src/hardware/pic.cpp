#ifndef LIB86CPU
#include "cpuapi.h"
#else
#include "lib86cpu/cpu.h"
#endif
#include "devices.h"
#include "state.h"

// Emulation of an Intel 8259 PIC.
// http://www.thesatya.com/8259.html
// https://pdos.csail.mit.edu/6.828/2016/readings/hardware/8259A.pdf

// Sequence of actions for interrupts when the I/O APIC is unavailable
//  - Device raises IRQ
//  - PIC sets IRR bit accordingly
//  - If IRQ was sent to slave PIC, and slave ISR is clear, then raise IRQ2 on master PIC*
//  - If ISR is empty and interrupt is not masked, then raise INTR line to signal CPU
//  - When CPU is ready to accept the interrupt (i.e. IF=1), it will send an IAC
// * Note: masking IRQ2 on the master PIC prevents slave from handling interrupts

#define PIC_LOG(x, ...) LOG("PIC", x, ##__VA_ARGS__)
#define PIC_FATAL(x, ...)          \
    do {                           \
        PIC_LOG(x, ##__VA_ARGS__); \
        ABORT();                   \
    } while (0)

struct pic_controller {
    // <<< BEGIN STRUCT "struct" >>>
    uint8_t vector_offset;
    uint8_t imr, irr, isr;

    // Represents the 8 input pins of the PIC.
    uint8_t pin_state;

    uint8_t icw[5], // should be 4
        icw_index;
    uint8_t ocw[4]; // ocw0 unused

    uint8_t read_isr;

    uint8_t autoeoi, rotate_on_autoeoi;

    uint8_t priority_base; // lowest priority base

    uint8_t in_initialization;

    uint8_t highest_priority_irq_to_send;

    // Has the INTR line been raised?
    uint8_t raised_intr_line;

    // PCI stuff
    uint8_t elcr;
    // <<< END STRUCT "struct" >>>
};

struct
{
    int irq_bus_value; // irq value to send to CPU
    struct pic_controller ctrl[2];
} pic;

// Rotates a byte "count" bits to the right. This function helps make things simpler
static inline uint8_t rol(uint8_t value, uint8_t priority_base)
{
    // 0 1 2 3 4 5 6 7
    //               ^-- If 7 is priority base, then...
    // 0 1 2 3 4 5 6 7
    // ^-- 0 is highest priority interrupt
    // Rotate count: 7 ^ 7 = 0

    // 0 1 2 3 4 5 6 7
    //             ^-- If 6 is priority base, then...
    // 0 1 2 3 4 5 6 7
    //               ^-- 7 is highest priority interrupt
    // Normalized:
    // 7 0 1 2 3 4 5 6
    // Rotate count: 6 ^ 7 = 1
    uint8_t count = priority_base ^ 7;
    return (value << count) | (value >> (8 - count));
}

static inline int is_master(struct pic_controller* ctrl){
    return ctrl == &pic.ctrl[0];
}

static void pic_state(void)
{
    // <<< BEGIN AUTOGENERATE "state" >>>
    struct bjson_object* obj = state_obj("pic", (16 + 1) * 2);
    state_field(obj, 1, "pic.ctrl[0].vector_offset", &pic.ctrl[0].vector_offset);
    state_field(obj, 1, "pic.ctrl[1].vector_offset", &pic.ctrl[1].vector_offset);
    state_field(obj, 1, "pic.ctrl[0].imr", &pic.ctrl[0].imr);
    state_field(obj, 1, "pic.ctrl[1].imr", &pic.ctrl[1].imr);
    state_field(obj, 1, "pic.ctrl[0].irr", &pic.ctrl[0].irr);
    state_field(obj, 1, "pic.ctrl[1].irr", &pic.ctrl[1].irr);
    state_field(obj, 1, "pic.ctrl[0].isr", &pic.ctrl[0].isr);
    state_field(obj, 1, "pic.ctrl[1].isr", &pic.ctrl[1].isr);
    state_field(obj, 1, "pic.ctrl[0].pin_state", &pic.ctrl[0].pin_state);
    state_field(obj, 1, "pic.ctrl[1].pin_state", &pic.ctrl[1].pin_state);
    state_field(obj, 5, "pic.ctrl[0].icw", &pic.ctrl[0].icw);
    state_field(obj, 5, "pic.ctrl[1].icw", &pic.ctrl[1].icw);
    state_field(obj, 1, "pic.ctrl[0].icw_index", &pic.ctrl[0].icw_index);
    state_field(obj, 1, "pic.ctrl[1].icw_index", &pic.ctrl[1].icw_index);
    state_field(obj, 4, "pic.ctrl[0].ocw", &pic.ctrl[0].ocw);
    state_field(obj, 4, "pic.ctrl[1].ocw", &pic.ctrl[1].ocw);
    state_field(obj, 1, "pic.ctrl[0].read_isr", &pic.ctrl[0].read_isr);
    state_field(obj, 1, "pic.ctrl[1].read_isr", &pic.ctrl[1].read_isr);
    state_field(obj, 1, "pic.ctrl[0].autoeoi", &pic.ctrl[0].autoeoi);
    state_field(obj, 1, "pic.ctrl[1].autoeoi", &pic.ctrl[1].autoeoi);
    state_field(obj, 1, "pic.ctrl[0].rotate_on_autoeoi", &pic.ctrl[0].rotate_on_autoeoi);
    state_field(obj, 1, "pic.ctrl[1].rotate_on_autoeoi", &pic.ctrl[1].rotate_on_autoeoi);
    state_field(obj, 1, "pic.ctrl[0].priority_base", &pic.ctrl[0].priority_base);
    state_field(obj, 1, "pic.ctrl[1].priority_base", &pic.ctrl[1].priority_base);
    state_field(obj, 1, "pic.ctrl[0].in_initialization", &pic.ctrl[0].in_initialization);
    state_field(obj, 1, "pic.ctrl[1].in_initialization", &pic.ctrl[1].in_initialization);
    state_field(obj, 1, "pic.ctrl[0].highest_priority_irq_to_send", &pic.ctrl[0].highest_priority_irq_to_send);
    state_field(obj, 1, "pic.ctrl[1].highest_priority_irq_to_send", &pic.ctrl[1].highest_priority_irq_to_send);
    state_field(obj, 1, "pic.ctrl[0].raised_intr_line", &pic.ctrl[0].raised_intr_line);
    state_field(obj, 1, "pic.ctrl[1].raised_intr_line", &pic.ctrl[1].raised_intr_line);
    state_field(obj, 1, "pic.ctrl[0].elcr", &pic.ctrl[0].elcr);
    state_field(obj, 1, "pic.ctrl[1].elcr", &pic.ctrl[1].elcr);
// <<< END AUTOGENERATE "state" >>>
    FIELD(pic.irq_bus_value);
}

static void pic_reset(void)
{
    for (int i = 0; i < 2; i++) {
        struct pic_controller* ctrl = &pic.ctrl[i];
        ctrl->vector_offset = 0;
        ctrl->imr = 0xFF;
        ctrl->irr = 0;
        ctrl->isr = 0;
        ctrl->in_initialization = 0;
        ctrl->read_isr = 0;
        ctrl->elcr = 0; // Is this right? Doesn't matter since the BIOS resets it anyways
    }
}
#ifndef LIB86CPU
static void pic_elcr_write(uint32_t addr, uint32_t data)
#else
void pic_elcr_write(uint32_t addr, const uint8_t data, void *opaque)
#endif
{
    pic.ctrl[addr & 1].elcr = data;
}
#ifndef LIB86CPU
static uint32_t pic_elcr_read(uint32_t addr)
#else
uint8_t pic_elcr_read(uint32_t addr, void *opaque)
#endif
{
    return pic.ctrl[addr & 1].elcr;
}

static void pic_internal_update(struct pic_controller* ctrl)
{
    //if (ctrl->raised_intr_line) // Wait for current interrupt to be serviced
    //    return;
    int unmasked, isr;

    // Check if any interrupts are unmasked
    if (!(unmasked = ctrl->irr & ~ctrl->imr)) {
        //PIC_LOG("No unmasked interrupts\n");
        return;
    }

    // Rotate IRR around so that the interrupts are located in decreasing priority (0 1 2 3 4 5 6 7)
    unmasked = rol(unmasked, ctrl->priority_base);
    PIC_LOG("Rotated: %02x Unmasked: %02x ISR: %02x\n", unmasked, ctrl->irr & ~ctrl->imr, ctrl->isr);
    // Do the same to the ISR
    isr = rol(ctrl->isr, ctrl->priority_base);

    if ((ctrl->ocw[3] & 0x60) == 0x60) {
        // Special mask mode -- ignore all values in the ISR
        unmasked &= ~isr;

        // Go through the remaining unmasked bits in order of descending priority and see if they are set.
        for (int i = 0; i < 8; i++) {
            if (unmasked & (1 << i)) {
                //PIC_LOG("IRQ to send: %d\n", (ctrl->priority_base + 1 + i) & 7);
                // An interrupt is unmasked here. Store it and tell the CPU to exit as fast as possible.
                ctrl->highest_priority_irq_to_send = (ctrl->priority_base + 1 + i) & 7;

                if(is_master(ctrl)){
#ifndef LIB86CPU
                cpu_raise_intr_line();
                cpu_request_fast_return(EXIT_STATUS_IRQ);
#else
                cpu_raise_hw_int_line(g_cpu);
#endif
                }else{
                    // Pulse INT line so that the slave PIC gets our message
                    pic_lower_irq(2);
                    pic_raise_irq(2);
                }
                return;
            }
        }
    } else {
        // If we are to send an interrupt, the priority of the value in the IRR must be greater than that in the ISR.
        for (int i = 0; i < 8; i++) {
            int mask = 1 << i;
            if (isr & mask)
                return; // Nope, no requested interrupt has a higher priority than a servicing interrupt
            if (unmasked & (1 << i)) {
                PIC_LOG("IRQ to send: %d irr=%02x pri=%02x rot=%02x\n", (ctrl->priority_base + 1 + i) & 7, ctrl->irr, ctrl->priority_base, unmasked);
                ctrl->highest_priority_irq_to_send = (ctrl->priority_base + 1 + i) & 7;

                if(is_master(ctrl)){
#ifndef LIB86CPU
                    cpu_raise_intr_line();
                    cpu_request_fast_return(EXIT_STATUS_IRQ);
#else
                    cpu_raise_hw_int_line(g_cpu);
#endif
                }else{
                    pic_lower_irq(2);
                    pic_raise_irq(2);
                }
                return;
            }
        }
    }
    // No interrupt :(
    return;
}

static uint8_t pic_internal_get_interrupt(struct pic_controller* ctrl)
{
    int irq = ctrl->highest_priority_irq_to_send, irq_mask = 1 << irq;
    // Sanity checks -- make sure that the highest priority interrupt is still within the IRR
    if (!(ctrl->irr & irq_mask)){
        return ctrl->vector_offset | 7;
    }

#if 0 // XXX -- this is needed for PCI interrupts, but we simulate level-triggered with edge-triggered. 
    // If edge triggered, then clear bit
    if (!(ctrl->elcr & irq_mask))
#endif
        ctrl->irr ^= irq_mask;

    // Set ISR if not in Automatic EOI mode
    if (ctrl->autoeoi) {
        if (ctrl->rotate_on_autoeoi)
            ctrl->priority_base = irq;
    } else
        ctrl->isr |= irq_mask;
    
    if(is_master(ctrl) && irq == 2)
        return pic_internal_get_interrupt(&pic.ctrl[1]);
    else return ctrl->vector_offset + irq;
}

#ifdef LIB86CPU
uint16_t pic_get_interrupt()
#else
uint8_t pic_get_interrupt(void)
#endif
{   
    // If APIC is enabled in PC settings and it has an interrupt, get the interrupt!
    if(apic_has_interrupt())
        return apic_get_interrupt();
    
    // This is our version of an IAC... the processor has indicated that it is ready to execute the interrupt.
    // All we have to do is fix up some state
#ifndef LIB86CPU
    cpu_lower_intr_line();
#else
    cpu_lower_hw_int_line(g_cpu);
#endif
    int x = pic_internal_get_interrupt(&pic.ctrl[0]);
    return x;
}

static void pic_internal_raise_irq(struct pic_controller* ctrl, int irq)
{
    int mask = 1 << irq;
    // Check for level/edge triggered interrupts
    if (!(ctrl->pin_state & mask)) {
        // Only edge triggered interrupts supported at the moment
        ctrl->pin_state |= mask;
        ctrl->irr |= mask;
        pic_internal_update(ctrl);
    }
}
static void pic_internal_lower_irq(struct pic_controller* ctrl, int irq)
{
    int mask = 1 << irq;
    ctrl->irr &= ~mask;
    ctrl->pin_state &= ~mask;
    if(!is_master(ctrl) &&!ctrl->irr) pic_lower_irq(2);
}

void pic_raise_irq(int a)
{
    PIC_LOG("Raising IRQ %d\n", a);
    // Send to I/O APIC if needed. 
    // The signal is ignored if APIC is disabled
    ioapic_raise_irq(a);

    pic_internal_raise_irq(&pic.ctrl[a > 7], a & 7);
}
void pic_lower_irq(int a)
{
    //PIC_LOG("Lowering IRQ %d\n", a);
    ioapic_lower_irq(a);

    pic_internal_lower_irq(&pic.ctrl[a > 7], a & 7);
    //if(!(pic.ctrl[0].irr | pic.ctrl[1].irr)) cpu_lower_intr_line();
}

static inline void pic_clear_specific(struct pic_controller* ctrl, int irq){
    ctrl->isr &= ~(1 << irq);
}
static inline void pic_set_priority(struct pic_controller* ctrl, int irq){
    ctrl->priority_base = irq;
}
static inline void pic_clear_highest_priority(struct pic_controller* ctrl)
{
    uint8_t highest = (ctrl->priority_base + 1) & 7;
    for (int i = 0; i < 8; i++) {
        uint8_t mask = 1 << ((highest + i) & 7);
        if (ctrl->isr & mask) {
            ctrl->isr ^= mask;
            return;
        }
    }
}

static void pic_write_icw(struct pic_controller* ctrl, int id, uint8_t value)
{
    //PIC_LOG("write to icw%d: %02x\n", id, value);
    switch (id) {
    case 1: // ICW1
        ctrl->icw_index = 2;
        ctrl->icw[1] = value;

        ctrl->imr = 0;
        ctrl->isr = 0;
        ctrl->irr = 0;
        ctrl->priority_base = 7; // Make IRQ0 have highest priority
        break;
    case 2:
        ctrl->vector_offset = value & ~7;
        ctrl->icw[2] = value;
        if (ctrl->icw[1] & 2) {
            // single pic
            if (ctrl->icw[1] & 1)
                ctrl->icw_index = 4;
            else
                ctrl->icw_index = 5;
        } else
            ctrl->icw_index = 3;
        break;
    case 3:
        ctrl->icw[3] = value;
        ctrl->icw_index = 5 ^ (ctrl->icw[1] & 1);
        break;
    case 4:
        ctrl->icw[4] = value;
        ctrl->autoeoi = value & 2;
        ctrl->icw_index = 5;
    }
    ctrl->in_initialization = ctrl->icw_index != 5;
}

static void pic_write_ocw(struct pic_controller* ctrl, int index, int data){
    ctrl->ocw[index] = data;
    switch (index) {
    case 1: // OCW1: Interrupt mask register
        ctrl->imr = data;
        // Resetting the IMR may result in an interrupt line finally being able to deliver interrupts.
        // This may happen when two devices give off interrupts during the same cpu_run frame.
        // Necessary for Win95 protected mode IDE driver. 
        pic_internal_update(ctrl);
        break;
    case 2: { // OCW2: EOI and rotate bits
        int rotate = data & 0x80, specific = data & 0x40, eoi = data & 0x20, l = data & 7;
        if (eoi) {
            if (specific) {
                // Specific EOI command
                pic_clear_specific(ctrl, l);
                if (rotate)
                    pic_set_priority(ctrl, l);
            }
            else {
                // Non Specific EOI
                pic_clear_highest_priority(ctrl);
                if (rotate)
                    pic_set_priority(ctrl, l);
            }
            pic_internal_update(ctrl);
        }
        else {
            if (specific) {
                if (rotate)
                    pic_set_priority(ctrl, l);
                // Otherwise, NOP
            }
            else
                ctrl->rotate_on_autoeoi = rotate > 0; // note: does not set priority
        }
    }
    break;
    case 3: { // Various other features
        if (data & 2)
            ctrl->read_isr = data & 1;
        else if (data & 0x44)
            PIC_LOG("Unknown feature: %02x\n", data);
    }
    }
}

#ifndef LIB86CPU
static void pic_writeb(uint32_t addr, uint32_t data)
#else
void pic_writeb(uint32_t addr, const uint8_t data, void *opaque)
#endif
{
    struct pic_controller* ctrl = &pic.ctrl[addr >> 7 & 1];
    if((addr & 1) == 0){
        switch(data >> 3 & 3){
        case 0:
            pic_write_ocw(ctrl, 2, data);
            break;
        case 1:
            pic_write_ocw(ctrl, 3, data);
            break;
        default: // case 2 or 3
            // initialization
            ctrl->in_initialization = 1;
            ctrl->imr = ctrl->isr = ctrl->irr = 0;
            ctrl->priority_base = 7;
            ctrl->autoeoi = ctrl->rotate_on_autoeoi = 0;
#ifndef LIB86CPU
            cpu_lower_intr_line();
#else
            cpu_lower_hw_int_line(g_cpu);
#endif
            pic_write_icw(ctrl, 1, data);
            break;
        }
    } else {
        // OCW
        if (ctrl->in_initialization)
            pic_write_icw(ctrl, ctrl->icw_index, data);
        else
            pic_write_ocw(ctrl, 1, data);
    }
}
#ifndef LIB86CPU
static uint32_t pic_readb(uint32_t port)
#else
uint8_t pic_readb(uint32_t port, void *opaque)
#endif
{
    struct pic_controller* ctrl = &pic.ctrl[port >> 7 & 1];
    if (port & 1)
        return ctrl->imr;
    else
        return ctrl->read_isr ? ctrl->isr : ctrl->irr;
}

void pic_init(struct pc_settings* pc)
{
#ifndef LIB86CPU
    io_register_write(0x20, 2, pic_writeb, NULL, NULL);
    io_register_read(0x20, 2, pic_readb, NULL, NULL);
    io_register_write(0xA0, 2, pic_writeb, NULL, NULL);
    io_register_read(0xA0, 2, pic_readb, NULL, NULL);

    if (pc->pci_enabled) {
        // Part of the 82371SB ISA controller, but would be more convieniently located here
        io_register_write(0x4D0, 2, pic_elcr_write, NULL, NULL);
        io_register_read(0x4D0, 2, pic_elcr_read, NULL, NULL);
    }
#endif
    io_register_reset(pic_reset);
    state_register(pic_state);

    pic.irq_bus_value = -1;
}