// SPDX-License-Identifier: BSD-2-Clause
// Exact block-floating scan/shift. No product register and no IRQ masking.
.text
.global mp3_maxbits
mp3_maxbits:
    push r0
    push r3
    push sv
    push cfgi
    push mod0
    set 0x0003, mod0
    mov a0l, r0
    mov a1l, r3
    clr a1, always
    cmpv 0x0, r3
    br .Lmp3_bits_done, eq
    modr [r3--]
    mov 0x2, cfgi
    bkrep r3, .Lmp3_bits_end
    mov [arrn0+ars0], a0
    neg a0, lt
    or a0, a1
    modr [r0++s]
.Lmp3_bits_end:
.Lmp3_bits_done:
    mov a1, b0
    clr a0, always
    cmp 0x0u8, a1
    br .Lmp3_bits_return, eq
    exp b0, a1
    mov 0x1f, a0
    sub a1, a0
.Lmp3_bits_return:
    pop mod0
    pop cfgi
    pop sv
    pop r3
    pop r0
    ret always

// PCM rounding/saturation for the block-floating exponent. All intermediates
// use the full 40-bit accumulator; no product register and no IRQ masking.
.global mp3_scale_pcm
mp3_scale_pcm:
    push r0
    push sv
    push mod0
    rst 0x0080, mod0
    set 0x0003, mod0
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov a1l, r0
    mov r0, a1
    cmp 0x0u8, a1
    br .Lpcm_right, lt
    cmp 0x0fu8, a1
    br .Lpcm_large, gt
    neg a1, always
    mov a1l, sv
    mov 0x7fff, b0
    shfc b0, b0, always
    cmp b0, a0
    br .Lpcm_positive, gt
    mov 0x8000, b0
    shfc b0, b0, always
    cmp b0, a0
    br .Lpcm_negative, lt
    mov r0, sv
    shfc a0, a0, always
    br .Lpcm_return, always
.Lpcm_large:
    cmp 0x0u8, a0
    br .Lpcm_positive, gt
    br .Lpcm_negative, lt
    br .Lpcm_return, always
.Lpcm_right:
    cmp 0xffe1, a1
    br .Lpcm_zero, lt
    neg a1, always
    sub 0x1, a1
    mov a1l, sv
    mov 0x1, a1
    shfc a1, a1, always
    add a1, a0
    mov r0, sv
    shfc a0, a0, always
    cmp 0x7fff, a0
    br .Lpcm_positive, gt
    cmp 0x8000, a0
    br .Lpcm_negative, lt
    br .Lpcm_return, always
.Lpcm_zero:
    clr a0, always
    br .Lpcm_return, always
.Lpcm_positive:
    mov 0x7fff, a0
    br .Lpcm_return, always
.Lpcm_negative:
    mov 0x8000, a0
.Lpcm_return:
    pop mod0
    pop sv
    pop r0
    ret always

.global mp3_rescale
mp3_rescale:
    push r0
    push r3
    push sv
    push cfgi
    push mod0
    rst 0x0080, mod0
    set 0x0003, mod0
    mov a0l, r0
    mov a1l, r3
    cmpv 0x0, r3
    br .Lmp3_scale_return, eq
    mov b0l, a0
    cmp 0x0u8, a0
    br .Lmp3_scale_return, eq
    cmp 0x1eu8, a0
    br .Lmp3_scale_zero, gt
    cmp 0xffe1, a0
    br .Lmp3_scale_set, ge
    mov 0xffe1, a0
.Lmp3_scale_set:
    mov a0l, sv
    mov 0x2, cfgi
    modr [r3--]
    bkrep r3, .Lmp3_scale_end
    mov [arrn0+ars0], a0
    shfc a0, a0, always
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov a0, [arrn0+ars0]
    modr [r0++s]
.Lmp3_scale_end:
    br .Lmp3_scale_return, always
.Lmp3_scale_zero:
    clr a0, always
    mov 0x2, cfgi
    modr [r3--]
    bkrep r3, .Lmp3_zero_end
    mov a0, [arrn0+ars0]
    modr [r0++s]
.Lmp3_zero_end:
.Lmp3_scale_return:
    pop mod0
    pop cfgi
    pop sv
    pop r3
    pop r0
    ret always

// Exact empty-subband test: 18 spectrum s32 + 9 overlap s32. Word loads are
// valid even at the odd-word second-channel overlap address. IRQs stay enabled.
.global mp3_hybrid_empty
mp3_hybrid_empty:
    push r0
    push r1
    push r3
    push mod0
    set 0x0003, mod0
    mov a0l, r0
    mov a1l, r1
    clr a1, always
    mov 0x23, r3
    bkrep r3, .Lhybrid_spec_end
    mov [r0++], a0
    or a0, a1
.Lhybrid_spec_end:
    mov 0x11, r3
    bkrep r3, .Lhybrid_overlap_end
    mov [r1++], a0
    or a0, a1
.Lhybrid_overlap_end:
    clr a0, always
    cmp 0x0u8, a1
    br .Lhybrid_nonzero, neq
    mov 0x1, a0
.Lhybrid_nonzero:
    pop mod0
    pop r3
    pop r1
    pop r0
    ret always
