/* SPDX-License-Identifier: BSD-2-Clause */
/* Program at 0, data at 0x10000000. */
OUTPUT_FORMAT("elf32-teak", "elf32-teak", "elf32-teak")
OUTPUT_ARCH(teak)
ENTRY(_start)
PHDRS { prog PT_LOAD; data PT_LOAD; ydata PT_LOAD; ybss PT_LOAD; aactables PT_LOAD; }
SECTIONS
{
    . = 0x0;
    .text : { KEEP(*(.text.start)) *(.text*) } :prog

    /* words 0x0000 stack, 0x0800 pipes, 0x1000 globals */
    . = 0x10000000 + 0x1000 * 2;
    .rodata : ALIGN(4) { *(.rodata*) } :data
    .data   : ALIGN(4) { *(.data*) } :data
    .bss    : ALIGN(4) { *(.bss* COMMON) } :data
    ASSERT(. <= 0x10000000 + 0x7000 * 2, "X memory overflow: output buffers at 0x7000")

    /* Y memory from word 0xC000 */
    . = 0x10000000 + 0xC000 * 2;
    .ydata  : ALIGN(4) { *(.yrodata*) *(.ydata*) } :ydata
    .ybss (NOLOAD) : ALIGN(4) { *(.ybss*) } :ybss
    ASSERT(. <= 0x10000000 + 0x10000 * 2, "Y memory overflow")

    .aac_input 0x1000E500 (NOLOAD) : { *(.aac_input) } :NONE
    ASSERT(SIZEOF(.aac_input) <= 0xB00, "AAC input overlaps MMIO")
    .aac_overlap0 0x10030000 (NOLOAD) : { *(.aac_overlap0) } :NONE
    ASSERT(SIZEOF(.aac_overlap0) == 0x1000, "AAC overlap0 size")
    .aac_overlap1 0x100360DA (NOLOAD) : { *(.aac_overlap1) } :NONE
    .aac_sf : ALIGN(2) { *(.aac_sf) } :NONE
    .aac_ms : ALIGN(2) { *(.aac_ms) } :NONE
    ASSERT(. <= 0x10038000, "AAC low bank1 overlaps tables")
    .aac_tables 0x10038000 : { *(.aac_tables) } :aactables
    ASSERT(SIZEOF(.aac_tables) == 22716, "AAC table ABI changed")
    .aac_time (NOLOAD) : ALIGN(2) { *(.aac_time) } :NONE
    .aac_types (NOLOAD) : ALIGN(2) { *(.aac_types) } :NONE
    ASSERT(. <= 0x10040000, "AAC bank1 overflow")
    ASSERT(SIZEOF(.text) <= 0x40000, "Program RAM overflow")

    /DISCARD/ : { *(.comment) *(.note*) }
}
