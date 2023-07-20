#pragma once

#include "pc.h"
#include "stddef.h"
#include "lib86cpu.h"


int cpu_init(struct pc_settings *pc);
void cpu_destroy_io_region(uint32_t port, uint32_t size);
lc86_status cpu_add_io_region(port_t port, uint32_t size, io_handlers_t handlers, void *opaque);
lc86_status cpu_add_mmio_region(addr_t addr, uint32_t size, io_handlers_t handlers, void *opaque);
lc86_status cpu_add_ram_region(addr_t addr, uint32_t size);
lc86_status cpu_add_rom_region(addr_t addr, uint32_t size, uint8_t *buffer);
void cpu_pause();
void cpu_resume();

inline cpu_t *g_cpu;
inline int cpu_paused = 0;
