#include "lib86cpu/cpu.h"
#include "devices.h"
#include <cstdarg>


extern uint8_t dma_io_readb(uint32_t port, void *opaque);
extern void dma_io_writeb(uint32_t port, const uint8_t data, void *opaque);
extern uint8_t cmos_readb(uint32_t port, void *opaque);
extern void cmos_writeb(uint32_t port, const uint8_t data, void *opaque);
extern uint8_t fdc_read(uint32_t port, void *opaque);
extern void fdc_write(uint32_t port, const uint8_t data, void *opaque);
extern uint8_t pit_readb(uint32_t a, void *opaque);
extern void pit_writeb(uint32_t port, const uint8_t value, void *opaque);
extern uint8_t pit_speaker_readb(uint32_t port, void *opaque);
extern void pit_speaker_writeb(uint32_t port, const uint8_t data, void *opaque);
extern uint8_t pic_readb(uint32_t port, void *opaque);
extern void pic_writeb(uint32_t port, const uint8_t data, void *opaque);
extern uint8_t pic_elcr_read(uint32_t port, void *opaque);
extern void pic_elcr_write(uint32_t port, const uint8_t data, void *opaque);
extern uint8_t kbd_read(uint32_t port, void *opaque);
extern void kbd_write(uint32_t port, const uint8_t data, void *opaque);
extern uint16_t vbe_readw(uint32_t port, void *opaque);
extern void vbe_writew(uint32_t port, const uint16_t data, void *opaque);
extern void vga_writew(uint32_t port, const uint16_t data, void *opaque);
extern uint8_t vga_readb(uint32_t port, void *opaque);
extern void vga_writeb(uint32_t port, const uint8_t data, void *opaque);
extern uint8_t vga_mem_readb(uint32_t addr, void *opaque);
extern void vga_mem_writeb(uint32_t addr, const uint8_t data, void *opaque);
extern uint16_t vga_mem_readw(uint32_t addr, void *opaque);
extern void vga_mem_writew(uint32_t addr, const uint16_t data, void *opaque);
extern uint8_t vga_rom_readb(uint32_t addr, void *opaque);
extern void vga_rom_writeb(uint32_t addr, const uint8_t data, void *opaque);
extern uint8_t ide_read(uint32_t addr, void *opaque);
extern void ide_write(uint32_t addr, const uint8_t data, void *opaque);
extern uint8_t ide_pio_readb(uint32_t addr, void *opaque);
extern void ide_pio_writeb(uint32_t addr, const uint8_t data, void *opaque);
extern uint16_t ide_pio_readw(uint32_t addr, void *opaque);
extern void ide_pio_writew(uint32_t addr, const uint16_t data, void *opaque);
extern uint32_t ide_pio_readd(uint32_t addr, void *opaque);
extern void ide_pio_writed(uint32_t addr, const uint32_t data, void *opaque);
extern uint8_t pci_read(uint32_t addr, void *opaque);
extern void pci_write(uint32_t addr, const uint8_t data, void *opaque);
extern uint16_t pci_read16(uint32_t addr, void *opaque);
extern void pci_write16(uint32_t addr, const uint16_t data, void *opaque);
extern uint32_t pci_read32(uint32_t addr, void *opaque);
extern void pci_write32(uint32_t addr, const uint32_t data, void *opaque);
extern uint8_t mmio_readb(uint32_t addr, void *opaque);
extern void mmio_writeb(uint32_t addr, const uint8_t data, void *opaque);
extern uint16_t mmio_readw(uint32_t addr, void *opaque);
extern void mmio_writew(uint32_t addr, const uint16_t data, void *opaque);
extern uint32_t mmio_readd(uint32_t addr, void *opaque);
extern void mmio_writed(uint32_t addr, const uint32_t data, void *opaque);
extern uint8_t ioapic_readb(uint32_t addr, void *opaque);
extern void ioapic_writeb(uint32_t addr, const uint8_t data, void *opaque);
extern uint32_t ioapic_read(uint32_t addr, void *opaque);
extern void ioapic_write(uint32_t addr, const uint32_t data, void *opaque);
extern uint8_t bios_readb(uint32_t port, void *opaque);
extern void bios_writeb(uint32_t port, const uint8_t data, void *opaque);
extern uint16_t bios_readw(uint32_t port, void *opaque);
extern void bios_writew(uint32_t port, const uint16_t data, void *opaque);
extern uint32_t bios_readd(uint32_t port, void *opaque);
extern void bios_writed(uint32_t port, const uint32_t data, void *opaque);
extern uint8_t default_mmio_readb(uint32_t a, void *opaque);
extern void default_mmio_writeb(uint32_t a, const uint8_t b, void *opaque);


void cpu_destroy_io_region(uint32_t port, uint32_t size)
{
	mem_destroy_region(g_cpu, port, size, true);
}

lc86_status cpu_add_io_region(port_t port, uint32_t size, io_handlers_t handlers, void *opaque)
{
	return mem_init_region_io(g_cpu, port, size, true, handlers, opaque);
}

lc86_status cpu_add_mmio_region(addr_t addr, uint32_t size, io_handlers_t handlers, void *opaque)
{
	return mem_init_region_io(g_cpu, addr, size, false, handlers, opaque);
}

lc86_status cpu_add_ram_region(addr_t addr, uint32_t size)
{
	return mem_init_region_ram(g_cpu, addr, size);
}

lc86_status cpu_add_rom_region(addr_t addr, uint32_t size, uint8_t *buffer)
{
	return mem_init_region_rom(g_cpu, addr, size, buffer);
}

void cpu_pause()
{
	cpu_paused = 1;
	cpu_suspend(g_cpu, true);
}

void cpu_resume()
{
	cpu_paused = 0;
	cpu_resume(g_cpu);
}

static void
logger(log_level lv, const unsigned count, const char *msg, ...)
{
	std::string str;
	switch (lv)
	{
		case log_level::debug:
			str = std::string("DBG:   ") + msg + '\n';
			break;

		case log_level::info:
			str = std::string("INFO:  ") + msg + '\n';
			break;

		case log_level::warn:
			str = std::string("WARN:  ") + msg + '\n';
			break;

		case log_level::error:
			str = std::string("ERROR: ") + msg + '\n';
			break;

		default:
			str = std::string("UNK:   ") + msg + '\n';
	}

	if (count > 0) {
		va_list args;
		va_start(args, msg);
		vfprintf(stderr, str.c_str(), args);
		va_end(args);
	}
	else {
		fprintf(stderr, "%s", str.c_str());
	}
}

static uint8_t *ram_ptr;

static void copy_rom_to_ram(pc_settings *pc, uint32_t addr, uint32_t size, void *data)
{
	// the pci mmio handlers r/w from/to ram, so the rom must be copied to ram so that can work
	if (addr > pc->memory_size || (addr + size) > pc->memory_size) {
		return;
	}

	memcpy(ram_ptr + addr, data, size);
}

// Initializes CPU
int cpu_init(pc_settings *pc)
{
	if (!LC86_SUCCESS(cpu_new(pc->memory_size, g_cpu, pic_get_interrupt))) {
		fprintf(stderr, "Failed to initialize lib86cpu!\n");
		return -1;
	}

	if (!LC86_SUCCESS(cpu_set_flags(g_cpu, CPU_INTEL_SYNTAX))) {
		return -1;
	}

	// bios rom and pci mmio regions overlap on ram, and they take priority over it, so the ram region must be mapped first. Otherwise, the ram will destroy
	// the previously mapped regions, and bios rom and pci mmio accesses will go through ram!
	if (!LC86_SUCCESS(cpu_add_ram_region(0, pc->memory_size))) {
		return -1;
	}

	ram_ptr = get_ram_ptr(g_cpu);
	if (pc->pci_enabled) {
		pci_init_mem(ram_ptr);
	}

	// pci
	if (pc->pci_enabled) {
		io_handlers_t pci_handlers{ .fnr8 = pci_read, .fnr16 = pci_read16, .fnr32 = pci_read32,
	.fnw8 = pci_write, .fnw16 = pci_write16, .fnw32 = pci_write32 };
		if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0xCF8, 8, true, pci_handlers, nullptr))) {
			return -1;
		}

		// HACK: don't map the pci mmio region at 0xC0000. This because the bios will copy the vga bios rom at 0xC0000 - 0xEFFFF, and the bios rom covers the
		// remaining 0x20000 bytes of the region. This means that accessing the pci mmio is the same as accessing the two roms. Also, lib86cpu doesn't support
		// executing code from mmio, which will happen at runtime when the bios calls the vga bios entry point, and it will terminate the emulation instead.
		// NOTE: this will make the vga bios writable
#if 0
		if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0xC0000, 0x40000, false, io_handlers_t{ .fnr8 = mmio_readb, .fnr16 = mmio_readw, .fnr32 = mmio_readd,
			.fnw8 = mmio_writeb, .fnw16 = mmio_writew, .fnw32 = mmio_writed }, nullptr))) {
			return -1;
		}
#endif
	}

	// Check if BIOS and VGABIOS have sane values
	if (((uintptr_t)pc->bios.data | (uintptr_t)pc->vgabios.data) & 0xFFF) {
		fprintf(stderr, "BIOS and VGABIOS need to be aligned on a 4k boundary\n");
		return -1;
	}
	if (!pc->bios.length || !pc->vgabios.length) {
		fprintf(stderr, "BIOS/VGABIOS length is zero\n");
		return -1;
	}

	if (!LC86_SUCCESS(cpu_add_rom_region(0x100000 - pc->bios.length, pc->bios.length, static_cast<uint8_t *>(pc->bios.data)))) {
		return -1;
	}
	copy_rom_to_ram(pc, 0x100000 - pc->bios.length, pc->bios.length, pc->bios.data);

	if (!LC86_SUCCESS(cpu_add_rom_region(-pc->bios.length, pc->bios.length, static_cast<uint8_t *>(pc->bios.data)))) {
		return -1;
	}
	copy_rom_to_ram(pc, -pc->bios.length, pc->bios.length, pc->bios.data);

	if (!pc->pci_vga_enabled) {
		if (!LC86_SUCCESS(cpu_add_rom_region(0xC0000, pc->vgabios.length, static_cast<uint8_t *>(pc->vgabios.data)))) {
			return -1;
		}
		copy_rom_to_ram(pc, 0xC0000, pc->vgabios.length, pc->vgabios.data);
	}

	// dma
	io_handlers_t dma_handlers{ .fnr8 = dma_io_readb, .fnw8 = dma_io_writeb };
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0, 16, true, dma_handlers, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0xC0, 32, true, dma_handlers, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x480, 8, true, dma_handlers, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x80, 16, true, dma_handlers, nullptr))) {
		return -1;
	}

	// cmos
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x70, 2, true, io_handlers_t{ .fnr8 = cmos_readb, .fnw8 = cmos_writeb }, nullptr))) {
		return -1;
	}

	// floppy
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x3F0, 6, true, io_handlers_t{ .fnr8 = fdc_read, .fnw8 = fdc_write }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x3F7, 1, true, io_handlers_t{ .fnr8 = fdc_read, .fnw8 = fdc_write }, nullptr))) {
		return -1;
	}

	// pit
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x40, 4, true, io_handlers_t{ .fnr8 = pit_readb, .fnw8 = pit_writeb }, nullptr))) {
		return -1;
	}

	// pc speaker
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x61, 1, true, io_handlers_t{ .fnr8 = pit_speaker_readb, .fnw8 = pit_speaker_writeb }, nullptr))) {
		return -1;
	}

	// keyboard
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x60, 1, true, io_handlers_t{ .fnr8 = kbd_read, .fnw8 = kbd_write }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x64, 1, true, io_handlers_t{ .fnr8 = kbd_read, .fnw8 = kbd_write }, nullptr))) {
		return -1;
	}

	// pic
	io_handlers_t pic_handlers{ .fnr8 = pic_readb, .fnw8 = pic_writeb };
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x20, 2, true, pic_handlers, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0xA0, 2, true, pic_handlers, nullptr))) {
		return -1;
	}

	if (pc->pci_enabled) {
		if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x4D0, 2, true, io_handlers_t{ .fnr8 = pic_elcr_read, .fnw8 = pic_elcr_write }, nullptr))) {
			return -1;
		}
	}

	// vga
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x3B0, 48, true, io_handlers_t{ .fnr8 = vga_readb, .fnw8 = vga_writeb, .fnw16 = vga_writew }, nullptr))) {
		return -1;
	}

	if (pc->vbe_enabled) {
		if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x1CE, 2, true, io_handlers_t{ .fnr16 = vbe_readw, .fnw16 = vbe_writew }, nullptr))) {
			return -1;
		}
	}

	io_handlers_t vga_handlers{ .fnr8 = vga_mem_readb, .fnr16 = vga_mem_readw, .fnw8 = vga_mem_writeb, .fnw16 = vga_mem_writew };
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0xA0000, 0x20000, false, vga_handlers, nullptr))) {
		return -1;
	}

	uint32_t vga_mem_size = pc->vga_memory_size < (256 << 10) ? 256 << 10 : pc->vga_memory_size;
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0xE0000000, vga_mem_size, false, vga_handlers, nullptr))) {
		return -1;
	}

	if (pc->pci_vga_enabled) {
		if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0xFEB00000, 0x20000, false, io_handlers_t{ .fnr8 = vga_rom_readb, .fnw8 = vga_rom_writeb }, nullptr))) {
			return -1;
		}
	}

	// ide
	io_handlers_t ide_handlers{ .fnr8 = ide_pio_readb, .fnr16 = ide_pio_readw, .fnr32 = ide_pio_readd,
		.fnw8 = ide_pio_writeb, .fnw16 = ide_pio_writew, .fnw32 = ide_pio_writed };
	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x1F0, 1, true, ide_handlers, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x170, 1, true, ide_handlers, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x376, 1, true, io_handlers_t{ .fnr8 = ide_read, .fnw8 = ide_write }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x3F6, 1, true, io_handlers_t{ .fnr8 = ide_read, .fnw8 = ide_write }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x1F1, 7, true, io_handlers_t{ .fnr8 = ide_read, .fnw8 = ide_write }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0x171, 7, true, io_handlers_t{ .fnr8 = ide_read, .fnw8 = ide_write }, nullptr))) {
		return -1;
	}

	//ioapic
	if (pc->apic_enabled) {
		if (!LC86_SUCCESS(mem_init_region_io(g_cpu, 0xFEC00000, 4096, false, io_handlers_t{ .fnr8 = ioapic_readb, .fnr32 = ioapic_read,
			.fnw8 = ioapic_writeb, .fnw32 = ioapic_write }, nullptr))) {
			return -1;
		}
	}

	if (!LC86_SUCCESS(cpu_add_io_region(0xB3, 1, io_handlers_t{ .fnr8 = bios_readb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x511, 1, io_handlers_t{ .fnr8 = bios_readb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x510, 1, io_handlers_t{ .fnw16 = bios_writew }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x8900, 1, io_handlers_t{ .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(cpu_add_io_region(0x400, 4, io_handlers_t{ .fnw8 = bios_writeb, .fnw16 = bios_writew }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x500, 1, io_handlers_t{ .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(cpu_add_io_region(0x378, 8, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x278, 8, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x3f8, 8, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x2f8, 8, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x3e8, 8, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x2e8, 8, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x92, 1, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x510, 2, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(cpu_add_io_region(0x3e0, 8, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x360, 16, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x1e0, 16, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x160, 16, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(cpu_add_io_region(0x2f0, 8, io_handlers_t{ .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x270, 8, io_handlers_t{ .fnr8 = bios_readb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x6f0, 8, io_handlers_t{ .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x420, 2, io_handlers_t{ .fnr8 = bios_readb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0x4a0, 2, io_handlers_t{ .fnr8 = bios_readb }, nullptr))) {
		return -1;
	}
	if (!LC86_SUCCESS(cpu_add_io_region(0xa78, 2, io_handlers_t{ .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}

	if (!LC86_SUCCESS(cpu_add_io_region(0x22, 4, io_handlers_t{ .fnr8 = bios_readb, .fnw8 = bios_writeb }, nullptr))) {
		return -1;
	}

	if (!pc->pci_enabled) {
		io_handlers_t pci_handlers{ .fnr8 = bios_readb, .fnr16 = bios_readw, .fnr32 = bios_readd,
	.fnw8 = bios_writeb, .fnw16 = bios_writew, .fnw32 = bios_writed };
		if (!LC86_SUCCESS(cpu_add_io_region(0xCF8, 8, pci_handlers, nullptr))) {
			return -1;
		}
	}

	if (!pc->apic_enabled) {
		if (!LC86_SUCCESS(cpu_add_mmio_region(0xFEE00000, 1 << 20, io_handlers_t{ .fnr8 = default_mmio_readb, .fnw8 = default_mmio_writeb }, nullptr))) {
			return -1;
		}
	}

	// Check writes to ROM
	if (!pc->pci_enabled) {
		// PCI can control ROM areas using the Programmable Attribute Map Registers, so this will be handled there
		if (!LC86_SUCCESS(cpu_add_mmio_region(0xC0000, 0x40000, io_handlers_t{ .fnw8 = default_mmio_writeb }, nullptr))) {
			return -1;
		}
	}

    return 0;
}
