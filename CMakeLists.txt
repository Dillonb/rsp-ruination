cmake_minimum_required(VERSION 3.10)
project(rsp-ruination)
enable_language(C ASM)

# Setup toolchain
if (DEFINED ENV{N64_INST})
    message("libdragon found using environment variable N64_INST=$ENV{N64_INST}")
    set(tools $ENV{N64_INST})
else()
    message("N64_INST environment variable not found, using /usr and hoping for the best...")
    set(tools /usr)
endif()

function(set_and_check var path)
    set(${var} ${path} PARENT_SCOPE)
    if (NOT EXISTS ${path})
        message(FATAL_ERROR "${var}=${path} not found!")
    endif()
endfunction()

set_and_check(CMAKE_C_COMPILER ${tools}/bin/mips64-elf-gcc)
set_and_check(CMAKE_C_LINKER ${tools}/bin/mips64-elf-ld)
set_and_check(CMAKE_CXX_COMPILER ${tools}/bin/mips64-elf-g++)
set_and_check(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set_and_check(CMAKE_OBJCOPY ${tools}/bin/mips64-elf-objcopy)
set_and_check(N64TOOL ${tools}/bin/n64tool)
set_and_check(CHKSUM64 ${tools}/bin/chksum64)

set(LINKER_SCRIPT n64.ld)
include_directories(${tools}/mips64-elf/include .)
add_compile_options(-std=gnu99 -march=vr4300 -mtune=vr4300 -DN64_BIG_ENDIAN)
set(LIBS_DIR ${tools}/mips64-elf/lib)

set(CMAKE_C_LINK_EXECUTABLE   "${CMAKE_C_LINKER} -o <TARGET> <OBJECTS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -L${LIBS_DIR} <LINK_LIBRARIES> -T${LINKER_SCRIPT}")

# Build RSP microcode
add_custom_command(OUTPUT text.section.bin data.section.bin
        COMMAND ${CMAKE_C_COMPILER} -c -o rsp.o ${CMAKE_CURRENT_SOURCE_DIR}/rsp.S
        COMMAND ${CMAKE_OBJCOPY} --dump-section .text=text.section.bin --dump-section .data=data.section.bin rsp.o
        BYPRODUCTS rsp.o
        MAIN_DEPENDENCY rsp.S)

# Build and link main code to ELF format
SET_SOURCE_FILES_PROPERTIES(loader.s OBJECT_DEPENDS "text.section.bin")
add_executable(code.elf
        rsp-ruination.c
        rsp_vector_instructions.c rsp_vector_instructions.h
        mips_instruction_decode.h
        n64_rsp_bus.h
        rsp_funct.h
        rsp_rom.h
        rsp_state.c rsp_state.h
        testcases.h
        loader.s)
target_compile_options(code.elf PUBLIC -Wall -Werror -O2)
target_link_libraries(code.elf PUBLIC dragon c m dragonsys)

# Package as a .z64 ROM
set(N64_HEADER ${tools}/mips64-elf/lib/header)
set(N64_FLAGS -l 2M -h ${N64_HEADER})
add_custom_command(OUTPUT rsp-ruination.z64
        COMMAND ${CMAKE_OBJCOPY} code.elf rsp-ruination.bin -O binary
        COMMAND ${N64TOOL} ${N64_FLAGS} -o rsp-ruination.z64 -t "RSP Ruination" rsp-ruination.bin
        COMMAND rm rsp-ruination.bin
        COMMAND ${CHKSUM64} rsp-ruination.z64
        DEPENDS code.elf)

add_custom_target(TARGET-rsp-ruination.z64 ALL DEPENDS rsp-ruination.z64)

add_custom_target(console UNFLoader -r rsp-ruination.z64
        DEPENDS rsp-ruination.z64)
