# ergo720 Copyright (c) 2022

cmake_minimum_required(VERSION 3.4.3)
project(halfix)

if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
 set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${halfix_BINARY_DIR}/bin")
endif()

set(HALFIX_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_package(ZLIB REQUIRED)

option(LIB86CPU_BUILD_SHARED_LIB "Build shared libraries" OFF)
add_subdirectory("import/lib86cpu")

if (WIN32 AND MSVC)
message("Building for Windows")
set(PLATFORM_SRC
 "${HALFIX_ROOT_DIR}/src/display-win32.c"
)
else ()
message(FATAL_ERROR "Only Windows builds with msvc are supported for now")
endif()

message("Building halfix")
include_directories(${HALFIX_ROOT_DIR}/include ${ZLIB_INCLUDE_DIRS} ${HALFIX_ROOT_DIR}/import/lib86cpu/lib86cpu)

set(HEADERS
 "${HALFIX_ROOT_DIR}/include/cpuapi.h"
 "${HALFIX_ROOT_DIR}/include/devices.h"
 "${HALFIX_ROOT_DIR}/include/display.h"
 "${HALFIX_ROOT_DIR}/include/drive.h"
 "${HALFIX_ROOT_DIR}/include/mmio.h"
 "${HALFIX_ROOT_DIR}/include/net.h"
 "${HALFIX_ROOT_DIR}/include/pc.h"
 "${HALFIX_ROOT_DIR}/include/platform.h"
 "${HALFIX_ROOT_DIR}/include/state.h"
 "${HALFIX_ROOT_DIR}/include/util.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/config.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/fpu-constants.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/softfloat-compare.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/softfloat-constants.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/softfloat-macros.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/softfloat-round-pack.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/softfloat-specialize.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/softfloat.h"
 "${HALFIX_ROOT_DIR}/include/softfloat/softfloatx80.h"
 "${HALFIX_ROOT_DIR}/include/cpu/cpu.h"
 "${HALFIX_ROOT_DIR}/include/cpu/fpu.h"
 "${HALFIX_ROOT_DIR}/include/cpu/instruction.h"
 "${HALFIX_ROOT_DIR}/include/cpu/instrument.h"
 "${HALFIX_ROOT_DIR}/include/cpu/libcpu.h"
 "${HALFIX_ROOT_DIR}/include/cpu/opcodes.h"
 "${HALFIX_ROOT_DIR}/include/cpu/ops.h"
 "${HALFIX_ROOT_DIR}/include/cpu/simd.h"
)

set(SOURCES
 "${HALFIX_ROOT_DIR}/src/main.c"
 "${HALFIX_ROOT_DIR}/src/pc.c"
 "${HALFIX_ROOT_DIR}/src/util.c"
 "${HALFIX_ROOT_DIR}/src/state.c"
 "${HALFIX_ROOT_DIR}/src/io.c"
 "${HALFIX_ROOT_DIR}/src/drive.c"
 "${HALFIX_ROOT_DIR}/src/ini.c"
 "${HALFIX_ROOT_DIR}/src/host/net-none.c"
 "${HALFIX_ROOT_DIR}/src/cpu/access.c"
 "${HALFIX_ROOT_DIR}/src/cpu/trace.c"
 "${HALFIX_ROOT_DIR}/src/cpu/seg.c"
 "${HALFIX_ROOT_DIR}/src/cpu/cpu.c"
 "${HALFIX_ROOT_DIR}/src/cpu/mmu.c"
 "${HALFIX_ROOT_DIR}/src/cpu/ops/ctrlflow.c"
 "${HALFIX_ROOT_DIR}/src/cpu/smc.c"
 "${HALFIX_ROOT_DIR}/src/cpu/decoder.c"
 "${HALFIX_ROOT_DIR}/src/cpu/eflags.c"
 "${HALFIX_ROOT_DIR}/src/cpu/prot.c"
 "${HALFIX_ROOT_DIR}/src/cpu/opcodes.c"
 "${HALFIX_ROOT_DIR}/src/cpu/ops/arith.c"
 "${HALFIX_ROOT_DIR}/src/cpu/ops/io.c"
 "${HALFIX_ROOT_DIR}/src/cpu/ops/string.c"
 "${HALFIX_ROOT_DIR}/src/cpu/ops/stack.c"
 "${HALFIX_ROOT_DIR}/src/cpu/ops/misc.c"
 "${HALFIX_ROOT_DIR}/src/cpu/ops/bit.c"
 "${HALFIX_ROOT_DIR}/src/cpu/softfloat.c" 
 "${HALFIX_ROOT_DIR}/src/cpu/fpu.c"
 "${HALFIX_ROOT_DIR}/src/cpu/ops/simd.c"
 "${HALFIX_ROOT_DIR}/src/hardware/dma.c"
 "${HALFIX_ROOT_DIR}/src/hardware/cmos.c" 
 "${HALFIX_ROOT_DIR}/src/hardware/pit.c"
 "${HALFIX_ROOT_DIR}/src/hardware/pic.c"
 "${HALFIX_ROOT_DIR}/src/hardware/kbd.c"
 "${HALFIX_ROOT_DIR}/src/hardware/vga.c"
 "${HALFIX_ROOT_DIR}/src/hardware/ide.c"
 "${HALFIX_ROOT_DIR}/src/hardware/pci.c"
 "${HALFIX_ROOT_DIR}/src/hardware/apic.c" 
 "${HALFIX_ROOT_DIR}/src/hardware/ioapic.c"
 "${HALFIX_ROOT_DIR}/src/hardware/fdc.c"
 "${HALFIX_ROOT_DIR}/src/hardware/acpi.c" 

  ${PLATFORM_SRC}
)

source_group(TREE ${HALFIX_ROOT_DIR} PREFIX header FILES ${HEADERS})
source_group(TREE ${HALFIX_ROOT_DIR} PREFIX source FILES ${SOURCES})

if (MSVC)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

add_executable(halfix ${HEADERS} ${SOURCES})
target_link_libraries(halfix PUBLIC ${ZLIB_LIBRARIES} cpu)

if (MSVC)
set_property(DIRECTORY "${HALFIX_ROOT_DIR}" PROPERTY VS_STARTUP_PROJECT halfix)
endif()
