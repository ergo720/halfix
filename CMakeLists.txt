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
 "${HALFIX_ROOT_DIR}/src/display-win32.cpp"
)
else ()
message(FATAL_ERROR "Only Windows builds with msvc are supported for now")
endif()

message("Building halfix")
include_directories(${HALFIX_ROOT_DIR}/include ${ZLIB_INCLUDE_DIRS} ${HALFIX_ROOT_DIR}/import/lib86cpu/include)

set(HEADERS
 "${HALFIX_ROOT_DIR}/include/devices.h"
 "${HALFIX_ROOT_DIR}/include/display.h"
 "${HALFIX_ROOT_DIR}/include/drive.h"
 "${HALFIX_ROOT_DIR}/include/io2.h"
 "${HALFIX_ROOT_DIR}/include/net.h"
 "${HALFIX_ROOT_DIR}/include/pc.h"
 "${HALFIX_ROOT_DIR}/include/platform.h"
 "${HALFIX_ROOT_DIR}/include/state.h"
 "${HALFIX_ROOT_DIR}/include/util.h"
 "${HALFIX_ROOT_DIR}/include/lib86cpu/cpu.h"
)

set(SOURCES
 "${HALFIX_ROOT_DIR}/src/main.cpp"
 "${HALFIX_ROOT_DIR}/src/pc.cpp"
 "${HALFIX_ROOT_DIR}/src/util.cpp"
 "${HALFIX_ROOT_DIR}/src/state.cpp"
 "${HALFIX_ROOT_DIR}/src/io.cpp"
 "${HALFIX_ROOT_DIR}/src/drive.cpp"
 "${HALFIX_ROOT_DIR}/src/ini.cpp"
 "${HALFIX_ROOT_DIR}/src/host/net-none.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/dma.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/cmos.cpp" 
 "${HALFIX_ROOT_DIR}/src/hardware/pit.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/pic.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/kbd.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/vga.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/ide.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/pci.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/apic.cpp" 
 "${HALFIX_ROOT_DIR}/src/hardware/ioapic.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/fdc.cpp"
 "${HALFIX_ROOT_DIR}/src/hardware/acpi.cpp"
 "${HALFIX_ROOT_DIR}/src/lib86cpu/cpu.cpp"

  ${PLATFORM_SRC}
)

source_group(TREE ${HALFIX_ROOT_DIR} PREFIX header FILES ${HEADERS})
source_group(TREE ${HALFIX_ROOT_DIR} PREFIX source FILES ${SOURCES})

add_definitions(-DLIB86CPU -DREALTIME_TIMING)

if (MSVC)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

add_executable(halfix ${HEADERS} ${SOURCES})
target_link_libraries(halfix PUBLIC ${ZLIB_LIBRARIES} cpu)

if (MSVC)
set_property(DIRECTORY "${HALFIX_ROOT_DIR}" PROPERTY VS_STARTUP_PROJECT halfix)
endif()
