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
