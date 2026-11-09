// SPDX-License-Identifier: BSD-2-Clause
// CELT PVQ loops, bit-exact with libopus fixed point.

// CELT stereo_merge output loop
.section .text.opus_k_merge
.global opus_k_merge
opus_k_merge:
    push mod0
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r1
    addv 0x0002, r7
    mov r7, r2
    mov r7, a0
    add 0x0002u8, a0
    mov a0l, r3
    add 0x0002u8, a0
    mov a0l, r4
    addv 0x0006, r7
    mov [r7], a0
    mov a0l, r5
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r6
    addv 0xfff8, r7
    mov [r7], a0
    mov a0l, r7
.Lmerge_chunk:
    mov r7, a0
    sub 0x0030u8, a0
    brr .Lmerge_last, le
    mov a0l, r7
    mov 0x002f, a0
    brr .Lmerge_run, always
.Lmerge_last:
    mov r7, a0
    sub 0x0001u8, a0
    mov 0x0000, r7
.Lmerge_run:
    push mod3
    dint
    bkrep a0l, .Lmerge_end
    mov [r2++], y0
    modr [r0++]
    clr a0, always
    mpysu y0, [r0--], a0
    maa y0, [r0], a0
    mov [r0], y0
    macsu y0, [r2], a0
    maa y0, [r2--], a0
    mov a0, b0
    subh [r1++], a0
    subl [r1--], a0
    mov [r3++], y0
    clr a1, always
    mpysu y0, a0l, a1
    maa y0, a0h, a1
    mov a0h, y0
    macsu y0, [r3], a1
    maa y0, [r3--], a1
    shfi a1, a1, +8
    mov r5, sv
    shfc a1, a1, always
    mov a1h, [r0++]
    mov a1l, [r0++]
    mov b0, a0
    addh [r1++], a0
    addl [r1--], a0
    mov [r4++], y0
    clr a1, always
    mpysu y0, a0l, a1
    maa y0, a0h, a1
    mov a0h, y0
    macsu y0, [r4], a1
    maa y0, [r4--], a1
    shfi a1, a1, +8
    mov r6, sv
    shfc a1, a1, always
    mov a1h, [r1++]
    mov a1l, [r1++]
.Lmerge_end:
    pop mod3
    cmpv 0x0000, r7
    br .Lmerge_chunk, neq
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop mod0
    ret always

// CELT haar1 lane
.section .text.opus_k_haar
.global opus_k_haar
opus_k_haar:
    push mod0
    push cfgi
    push cfgj
    push y0
    push r0
    push r3
    push r4
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r4
    addv 0x0002, r7
    mov [r7], cfgi
    mov [r7], cfgj
    addv 0x0001, r7
    mov r7, r3
    addv 0xfffe, r7
    mov [r7], a0
    sub 0x0001u8, a0
    push mod3
    dint
    bkrep a0l, .Lhaar_end
    mov 0x5a82, y0
    modr [r0++]
    clr a0, always
    mpysu y0, [r0--], a0
    maa y0, [r0], a0
    mov [r0], y0
    macsu y0, [r3], a0
    maa y0, [r3], a0
    mov 0x5a82, y0
    modr [r4++]
    clr a1, always
    mpysu y0, [r4--], a1
    maa y0, [r4], a1
    mov [r4], y0
    macsu y0, [r3], a1
    maa y0, [r3], a1
    mov a0, b0
    add a1, a0
    mov a0h, [r0++]
    mov a0l, [r0++s]
    mov b0, a0
    sub a1, a0
    mov a0h, [r4++]
    mov a0l, [r4++s]
.Lhaar_end:
    pop mod3
    pop r7
    pop r4
    pop r3
    pop r0
    pop y0
    pop cfgj
    pop cfgi
    pop mod0
    ret always

// CELT normalise_residual loop
.section .text.opus_k_normres
.global opus_k_normres
opus_k_normres:
    push mod0
    push y0
    push r0
    push r1
    push r2
    push r3
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r1
    addv 0x0002, r7
    mov r7, r2
    addv 0x0001, r7
    mov r7, r3
    addv 0x0001, r7
    mov [r7], sv
    addv 0xfffd, r7
    mov [r7], a0
    sub 0x0001u8, a0
    push mod3
    dint
    bkrep a0l, .Lnormres_end
    mov [r0++], y0
    clr a0, always
    mpysu y0, [r3], a0
    maa y0, [r2], a0
    add p*, a0
    shfi a0, a0, +8
    shfc a0, a0, always
    mov a0h, [r1++]
    mov a0l, [r1++]
.Lnormres_end:
    pop mod3
    pop r7
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop mod0
    ret always

// CELT exp_rotation lane
.section .text.opus_k_exprot
.global opus_k_exprot
opus_k_exprot:
    push mod0
    push cfgi
    push cfgj
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    rst 0x0c00, mod0
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    mov 0x0200, r3
    bkrep a0l, .Lscd_end
    mov [r0++], a1h
    or [r0], a1
    add r3, a1
    shfi a1, a1, -10
    mov a1l, [r0++]
.Lscd_end:
    mov 0x4000, r3
    mov r3, b1
    addv 0x0001, r7
    mov [r7], a0
    cmp 0x0000u8, a0
    br .Lrot_no2, eq
    addv 0x0002, r7
    mov [r7], a0
    mov a0l, r5
    addv 0xffff, r7
    mov [r7], a0
    mov a0l, r6
    mov r6, a0
    neg a0, always
    mov a0l, r1
    addv 0xffff, r7
    mov [r7], a0
    mov a0l, r2
    mov 0x0002, a0
    mov a0l, cfgi
    mov a0l, cfgj
    addv 0xfffe, r7
    mov [r7], a0
    mov a0l, r0
    modr [r0++]
    mov r0, a0
    add r2, a0
    add r2, a0
    mov a0l, r4
    addv 0x0001, r7
    mov [r7], a0
    sub r2, a0
    sub 0x0001u8, a0
    push mod3
    dint
    bkrep a0l, .Lrotfa_end
    clr a0, always
    clr a1, always
    mov r5, y0
    mpy y0, [r4], a0
    mov r6, y0
    mac y0, [r0], a0
    mov r5, y0
    mac y0, [r0], a0
    mov r1, y0
    mac y0, [r4], a1
    add p*, a1
    add b1, a0
    add b1, a1
    shfi a0, a0, -15
    shfi a1, a1, -15
    mov a0l, [r4++s]
    mov a1l, [r0++s]
.Lrotfa_end:
    pop mod3
    mov [r7], a0
    sub r2, a0
    sub r2, a0
    br .Lrota_done, le
    mov a0, b0
    mov 0xfffe, a0
    mov a0l, cfgi
    mov a0l, cfgj
    mov b0, a0
    sub 0x0001u8, a0
    shfi a0, a0, +1
    add 0x0001u8, a0
    addv 0xffff, r7
    add [r7], a0
    mov a0l, r0
    add r2, a0
    add r2, a0
    mov a0l, r4
    mov b0, a0
    sub 0x0001u8, a0
    push mod3
    dint
    bkrep a0l, .Lrotba_end
    clr a0, always
    clr a1, always
    mov r5, y0
    mpy y0, [r4], a0
    mov r6, y0
    mac y0, [r0], a0
    mov r5, y0
    mac y0, [r0], a0
    mov r1, y0
    mac y0, [r4], a1
    add p*, a1
    add b1, a0
    add b1, a1
    shfi a0, a0, -15
    shfi a1, a1, -15
    mov a0l, [r4++s]
    mov a1l, [r0++s]
.Lrotba_end:
    pop mod3
    addv 0x0001, r7
.Lrota_done:
    addv 0x0001, r7
.Lrot_no2:
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r5
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r6
    mov r6, a0
    neg a0, always
    mov a0l, r1
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r2
    mov 0x0002, a0
    mov a0l, cfgi
    mov a0l, cfgj
    addv 0xfffb, r7
    mov [r7], a0
    mov a0l, r0
    modr [r0++]
    mov r0, a0
    add r2, a0
    add r2, a0
    mov a0l, r4
    addv 0x0001, r7
    mov [r7], a0
    sub r2, a0
    sub 0x0001u8, a0
    push mod3
    dint
    bkrep a0l, .Lrotfb_end
    clr a0, always
    clr a1, always
    mov r5, y0
    mpy y0, [r4], a0
    mov r6, y0
    mac y0, [r0], a0
    mov r5, y0
    mac y0, [r0], a0
    mov r1, y0
    mac y0, [r4], a1
    add p*, a1
    add b1, a0
    add b1, a1
    shfi a0, a0, -15
    shfi a1, a1, -15
    mov a0l, [r4++s]
    mov a1l, [r0++s]
.Lrotfb_end:
    pop mod3
    mov [r7], a0
    sub r2, a0
    sub r2, a0
    br .Lrotb_done, le
    mov a0, b0
    mov 0xfffe, a0
    mov a0l, cfgi
    mov a0l, cfgj
    mov b0, a0
    sub 0x0001u8, a0
    shfi a0, a0, +1
    add 0x0001u8, a0
    addv 0xffff, r7
    add [r7], a0
    mov a0l, r0
    add r2, a0
    add r2, a0
    mov a0l, r4
    mov b0, a0
    sub 0x0001u8, a0
    push mod3
    dint
    bkrep a0l, .Lrotbb_end
    clr a0, always
    clr a1, always
    mov r5, y0
    mpy y0, [r4], a0
    mov r6, y0
    mac y0, [r0], a0
    mov r5, y0
    mac y0, [r0], a0
    mov r1, y0
    mac y0, [r4], a1
    add p*, a1
    add b1, a0
    add b1, a1
    shfi a0, a0, -15
    shfi a1, a1, -15
    mov a0l, [r4++s]
    mov a1l, [r0++s]
.Lrotbb_end:
    pop mod3
    addv 0x0001, r7
.Lrotb_done:
    addv 0xffff, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    modr [r0++]
    bkrep a0l, .Lscu_end
    mov [r0--], a1h
    shfi a1, a1, -6
    mov a1h, [r0++]
    mov a1l, [r0++]
    modr [r0++]
.Lscu_end:
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgj
    pop cfgi
    pop mod0
    ret always

// CELT PVQ codeword to pulses
.section .text.opus_k_cwrsi
.global opus_k_cwrsi
opus_k_cwrsi:
    push mod0
    push cfgj
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov 0x007d, cfgj
    mov a0l, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r1
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r2
    addv 0x0003, r7
    mov [r7], a0
    mov a0l, r5
    addv 0xfffe, r7
    mov [r7], a1l
    shfi a1, a1, +16
    addv 0x0001, r7
    or [r7], a1
.Lc_loop:
    mov r1, a0
    cmp 0x0002u8, a0
    br .Lc_two, le
    mov r2, a0
    cmp r1, a0
    br .Lc_dims, lt
    mov r1, a0
    add r5, a0
    mov a0l, r3
    mov [r3], a0
    mov a0l, r3
    mov r2, a0
    shfi a0, a0, +1
    add r3, a0
    add 0x0002u8, a0
    mov a0l, r4
    mov [r4++], a0l
    shfi a0, a0, +16
    or [r4], a0
    mov 0x0000, r6
    cmp a0, a1
    brr .Lc_p1, lt
    sub a0, a1
    mov 0xffff, r6
    .Lc_p1:
    mov r2, y0
    mov r1, a0
    shfi a0, a0, +1
    add r3, a0
    mov a0l, r4
    mov [r4++], a0l
    shfi a0, a0, +16
    or [r4], a0
    cmp a1, a0
    br .Lc_down, le
    mov r1, r2
    mov r2, a0
    add r5, a0
    mov a0l, r3
    br .Lc_search, always
.Lc_down:
    mov r2, a0
    shfi a0, a0, +1
    add r3, a0
    mov a0l, r4
    .Lc_dl:
    mov [r4++], a0l
    shfi a0, a0, +16
    or [r4++s], a0
    cmp a1, a0
    brr .Lc_dl, gt
    // index from the stepped pointer
    mov r4, b0
    mov r3, b1
    sub b1, b0
    shfi b0, b0, -1
    mov b0l, r2
    modr [r2++]
    br .Lc_emit, always
.Lc_dims:
    mov r2, a0
    add r5, a0
    mov a0l, r3
    modr [r3++]
    mov [r3], a0
    add r1, a0
    add r1, a0
    mov a0l, r4
    mov [r4++], a0l
    shfi a0, a0, +16
    or [r4], a0
    mov a0, b1
    modr [r3--]
    mov [r3], a0
    add r1, a0
    add r1, a0
    mov a0l, r4
    mov [r4++], a0l
    shfi a0, a0, +16
    or [r4], a0
    cmp a1, a0
    brr .Lc_nz, gt
    cmp b1, a1
    brr .Lc_nz, ge
    sub a0, a1
    clr a0, always
    mov a0l, [r0++]
    br .Lc_next, always
    .Lc_nz:
    mov 0x0000, r6
    cmp b1, a1
    brr .Lc_p2, lt
    sub b1, a1
    mov 0xffff, r6
    .Lc_p2:
    mov r2, y0
.Lc_search:
    modr [r2--]
    modr [r3--]
    mov [r3], a0
    add r1, a0
    add r1, a0
    mov a0l, r4
    mov [r4++], a0l
    shfi a0, a0, +16
    or [r4], a0
    cmp a1, a0
    br .Lc_search, gt
.Lc_emit:
    sub a0, a1
    mov y0, a0
    sub r2, a0
    tst0 0xffff, r6
    neg a0, neq
    mov a0l, [r0++]
    .Lc_next:
    modr [r1--]
    br .Lc_loop, always
.Lc_two:
    mov r2, y0
    mov r2, a0
    shfi a0, a0, +1
    add 0x0001u8, a0
    mov 0x0000, r6
    cmp a0, a1
    brr .Lc_p3, lt
    sub a0, a1
    mov 0xffff, r6
    .Lc_p3:
    mov a1, a0
    add 0x0001u8, a0
    shfi a0, a0, -1
    mov a0l, r2
    cmp 0x0000u8, a0
    brr .Lc_k0, eq
    shfi a0, a0, +1
    sub 0x0001u8, a0
    sub a0, a1
    .Lc_k0:
    mov y0, a0
    sub r2, a0
    tst0 0xffff, r6
    neg a0, neq
    mov a0l, [r0++]
    mov r2, a0
    cmp 0x0000u8, a1
    neg a0, neq
    mov a0l, [r0++]
    rst 0x0c00, mod0
    addv 0xfffc, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a1
    sub 0x0001u8, a1
    clr a0, always
    mov 0x0000, y0
    push mod3
    dint
    mpy y0, [r0], a0
    bkrep a1l, .Lyy_end
    mov [r0], y0
    mac y0, [r0++], a0
.Lyy_end:
    add p*, a0
    pop mod3
    addv 0x0005, r7
    mov a0h, [r7]
    addv 0x0001, r7
    mov a0l, [r7]
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgj
    pop mod0
    ret always
