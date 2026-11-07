// SPDX-License-Identifier: BSD-2-Clause
.section .text.indirect
.global __teak_indirect_call
__teak_indirect_call:
    push r0
    push r1
    ret always
