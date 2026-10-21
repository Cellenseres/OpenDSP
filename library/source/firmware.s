@ SPDX-License-Identifier: BSD-2-Clause
@ Embeds firmware/opendsp.cdc into the library.
    .section .rodata.opendsp_firmware, "a"
    .balign 4
    .global opendsp_firmware
opendsp_firmware:
    .incbin "opendsp.cdc"
opendsp_firmware_end:
    .balign 4
    .global opendsp_firmware_size
opendsp_firmware_size:
    .word opendsp_firmware_end - opendsp_firmware
    .section .rodata.opendsp_aac_tables, "a"
    .balign 4
    .global opendsp_aac_tables
opendsp_aac_tables:
    .incbin "aac_tables.bin"
    .section .rodata.opendsp_mp3_tables, "a"
    .balign 4
    .global opendsp_mp3_tables
opendsp_mp3_tables:
    .incbin "mp3_tables.bin"
