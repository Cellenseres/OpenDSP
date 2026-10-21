// SPDX-License-Identifier: BSD-2-Clause
// Countdown deadline: 1 continues, 0 yields.
// Signed 32-bit delta handles wrap; quantum < 2^31.
// R/Y preserved; IRQs enabled, no multiplier state.
.text
.global mp3_slice_ready
mp3_slice_ready:
    push r0
    push r1
    mov a0l, r0
    mov a1l, r1
    mov [r1], a0
    cmp 0x0u8, a0
    br .Lslice_due, neq
    call aac_timer_now, always
    mov [arrn0+ars0], a1
    sub a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    cmp 0x0u8, a0
    br .Lslice_due, le
    mov 0x1, a0
    br .Lslice_return, always
.Lslice_due:
    clr a0, always
.Lslice_return:
    pop r1
    pop r0
    ret always
