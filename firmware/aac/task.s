// SPDX-License-Identifier: BSD-2-Clause
// Stackful cooperative task; context switches never span a live multiplier.
.text
.global aac_timer_now
aac_timer_now:
    push r0
    push r1
.Ltimer_read:
    mov 0x783a, r0
    mov [r0], r1
    mov 0x7838, r0
    mov [r0], a0
    and 0xffff, a0
    mov 0x783a, r0
    mov [r0], a1
    cmp r1, a1
    br .Ltimer_read, neq
    shfi a1, a1, +16
    or a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    pop r1
    pop r0
    ret always
.global aac_timer_elapsed
aac_timer_elapsed:
    mov a0, b0
    call aac_timer_now, always
    sub a0, b0
    mov b0, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    ret always

.global aac_task_step
.global aac_timer_delta
aac_timer_delta:
    sub a1, a0
    // Normalize unsigned u32 wrap before a saturating 32-bit store/return.
    shfi a0, a0, +8
    shfi a0, a0, -8
    ret always

aac_task_step:
    push stt0
    dint
    push stt1
    push mod0
    set 0x0001, mod0
    push a0e
    pusha a0
    push a1e
    pusha a1
    push b0e
    pusha b0
    push b1e
    pusha b1
    push x0
    push x1
    push y0
    push y1
    push sv
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push cfgi
    push cfgj
    push mod1
    push mod2
    push ar0
    push ar1
    mov 0x0f81, r0
    mov sp, a0
    mov a0l, [r0]
    mov 0x0f80, r0
    mov [r0], a0
    br .Ltask_first, eq
    mov a0l, sp
    br .Ltask_restore, always
.Ltask_first:
    mov 0x0f83, r0
    mov [r0], a0
    mov a0l, sp
    rst 0x0c81, mod0
    eint
    call aac_task_main, always
.Ltask_dead:
    br .Ltask_dead, always

.global aac_yield
aac_yield:
    push stt0
    dint
    push stt1
    push mod0
    set 0x0001, mod0
    push a0e
    pusha a0
    push a1e
    pusha a1
    push b0e
    pusha b0
    push b1e
    pusha b1
    push x0
    push x1
    push y0
    push y1
    push sv
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push cfgi
    push cfgj
    push mod1
    push mod2
    push ar0
    push ar1
    mov 0x0f80, r0
    mov sp, a0
    mov a0l, [r0]
    mov 0x0f81, r0
    mov [r0], a0
    mov a0l, sp
.Ltask_restore:
    pop ar1
    pop ar0
    pop mod2
    pop mod1
    pop cfgj
    pop cfgi
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop sv
    pop y1
    pop y0
    pop x1
    pop x0
    popa b1
    pop b1e
    popa b0
    pop b0e
    popa a1
    pop a1e
    popa a0
    pop a0e
    pop mod0
    pop stt1
    pop stt0
    eint
    ret always

.global aac_pack_pcm
aac_pack_pcm:
    push r0
    push r1
    push r3
    push r4
    mov a0l, r0
    mov a1l, r4
    mov b0l, r1
    mov 0x7f, r3
    cmpv 0x2, b1l
    br .Lpack_stereo, eq
    bkrep r3, .Lpack_mono_end
    mov [r0++], a0l
    mov a0l, [r1++]
.Lpack_mono_end:
    br .Lpack_done, always
.Lpack_stereo:
    bkrep r3, .Lpack_stereo_end
    mov [r0++], a0l
    mov [r4++], a1l
    mov a0l, [r1++]
    mov a1l, [r1++]
.Lpack_stereo_end:
.Lpack_done:
    pop r4
    pop r3
    pop r1
    pop r0
    ret always

// Saturated M/S over a bounded band segment, without the multiplier.
.global aac_ms_band
aac_ms_band:
    push r0
    push r2
    push r3
    push cfgi
    mov a0l, r0
    mov a1l, r2
    mov b0l, r3
    modr [r3--]
    mov 0x2, cfgi
    bkrep r3, .Lms_end
    mov [arrn0+ars0], a0
    mov [arrn2+ars0], a1
    mov a0, b0
    add a1, a0
    sub a1, b0
    lim a0, a0
    neg a0, always
    lim a0, a0
    neg a0, always
    mov a0, [arrn0+ars0]
    mov b0, a1
    lim a1, a1
    neg a1, always
    lim a1, a1
    neg a1, always
    mov a1, [arrn2+ars0]
    modr [r0++s]
    modr [r2++s]
.Lms_end:
    pop cfgi
    pop r3
    pop r2
    pop r0
    ret always
