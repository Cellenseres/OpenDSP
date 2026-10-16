/* SPDX-License-Identifier: BSD-2-Clause */
/* Program at 0, data at 0x10000000. */
OUTPUT_FORMAT("elf32-teak", "elf32-teak", "elf32-teak")
OUTPUT_ARCH(teak)
ENTRY(_start)
PHDRS { prog PT_LOAD; data PT_LOAD; ydata PT_LOAD; ybss PT_LOAD; }
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

    /DISCARD/ : { *(.comment) *(.note*) }
}
