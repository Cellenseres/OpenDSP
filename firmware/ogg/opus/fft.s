// SPDX-License-Identifier: BSD-2-Clause
// CELT kiss FFT butterflies, bit-exact with libopus fixed point.

// radix 4, twiddles all one
.section .text.opus_fft_bfly4m1
.global opus_fft_bfly4m1
opus_fft_bfly4m1:
    push mod0
    push cfgi
    push r0
    push r1
    push r2
    push r3
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    load 0x0047u8, page
    mov [r7], a1
    mov a1l, r0
    add 0x0004u8, a1
    mov a1l, r1
    add 0x0004u8, a1
    mov a1l, r2
    add 0x0004u8, a1
    mov a1l, r3
    add 0x0004u8, a1
    mov 0x007d, cfgi
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    bkrep a0l, .Lb4m1_end
    mov [r0++], a0h
    or [r0++], a0
    subh [r2++], a0
    subl [r2++], a0
    mov a0h, [page:0x0000u8]
    mov a0l, [page:0x0001u8]
    mov [r0++], a0h
    or [r0++s], a0
    subh [r2++], a0
    subl [r2++s], a0
    mov a0h, [page:0x0002u8]
    mov a0l, [page:0x0003u8]
    mov [r0++], a0h
    or [r0--], a0
    addh [r2++], a0
    addl [r2++], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov [r0++], a0h
    or [r0--], a0
    addh [r2++], a0
    addl [r2++s], a0
    mov a0h, [r0++]
    mov a0l, [r0++s]
    mov [r1++], a0h
    or [r1++], a0
    addh [r3++], a0
    addl [r3++], a0
    mov a0h, [page:0x0004u8]
    mov a0l, [page:0x0005u8]
    mov [r1++], a0h
    or [r1++s], a0
    addh [r3++], a0
    addl [r3++s], a0
    mov a0h, [page:0x0006u8]
    mov a0l, [page:0x0007u8]
    mov [r1++], a0h
    or [r1++], a0
    subh [r3++], a0
    subl [r3++], a0
    mov a0h, [page:0x0008u8]
    mov a0l, [page:0x0009u8]
    mov [r1++], a0h
    or [r1++s], a0
    subh [r3++], a0
    subl [r3++s], a0
    mov a0h, [page:0x000au8]
    mov a0l, [page:0x000bu8]
    mov [r0++], a0h
    or [r0++], a0
    subh [page:0x0004u8], a0
    subl [page:0x0005u8], a0
    mov a0h, [r2++]
    mov a0l, [r2++]
    mov [r0++], a0h
    or [r0++s], a0
    subh [page:0x0006u8], a0
    subl [page:0x0007u8], a0
    mov a0h, [r2++]
    mov a0l, [r2]
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x0004u8], a0
    addl [page:0x0005u8], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x0006u8], a0
    addl [page:0x0007u8], a0
    mov a0h, [r0++]
    mov a0l, [r0]
    mov [page:0x0000u8], a0h
    or [page:0x0001u8], a0
    addh [page:0x000au8], a0
    addl [page:0x000bu8], a0
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov [page:0x0002u8], a0h
    or [page:0x0003u8], a0
    subh [page:0x0008u8], a0
    subl [page:0x0009u8], a0
    mov a0h, [r1++]
    mov a0l, [r1]
    mov [page:0x0000u8], a0h
    or [page:0x0001u8], a0
    subh [page:0x000au8], a0
    subl [page:0x000bu8], a0
    mov a0h, [r3++]
    mov a0l, [r3++]
    mov [page:0x0002u8], a0h
    or [page:0x0003u8], a0
    addh [page:0x0008u8], a0
    addl [page:0x0009u8], a0
    mov a0h, [r3++]
    mov a0l, [r3]
    addv 0x000d, r0
    addv 0x000d, r1
    addv 0x000d, r2
    addv 0x000d, r3
.Lb4m1_end:
    pop r7
    pop r3
    pop r2
    pop r1
    pop r0
    pop cfgi
    pop mod0
    ret always

// radix 2 after radix 4, m = 4
.section .text.opus_fft_bfly2
.global opus_fft_bfly2
opus_fft_bfly2:
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
    load 0x0047u8, page
    mov [r7], a1
    mov a1l, r0
    add 0x0004u8, a1
    mov a1l, r1
    add 0x0004u8, a1
    mov a1l, r2
    add 0x0004u8, a1
    mov a1l, r3
    add 0x0004u8, a1
    mov a1l, r4
    add 0x0004u8, a1
    mov a1l, r5
    add 0x0004u8, a1
    mov a1l, r6
    add 0x0004u8, a1
    mov 0x007d, cfgi
    mov 0x007d, cfgj
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    mov a1l, r7
    bkrep a0l, .Lb2_end
    push mod3
    dint
    mov [r0++], a0h
    or [r0++], a0
    subh [r4++], a0
    subl [r4++], a0
    mov a0h, [page:0x0000u8]
    mov a0l, [page:0x0001u8]
    mov [r0++], a0h
    or [r0++s], a0
    subh [r4++], a0
    subl [r4++s], a0
    mov a0h, [page:0x0002u8]
    mov a0l, [page:0x0003u8]
    mov [r0++], a0h
    or [r0--], a0
    addh [r4++], a0
    addl [r4++], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov [r0++], a0h
    or [r0--], a0
    addh [r4++], a0
    addl [r4++s], a0
    mov a0h, [r0++]
    mov a0l, [r0]
    mov [page:0x0000u8], a0h
    or [page:0x0001u8], a0
    mov a0h, [r4++]
    mov a0l, [r4++]
    mov [page:0x0002u8], a0h
    or [page:0x0003u8], a0
    mov a0h, [r4++]
    mov a0l, [r4]
    mov [r5++], a0h
    or [r5++], a0
    addh [r5++], a0
    addl [r5--], a0
    mov a0h, [page:0x0004u8]
    mov a0l, [page:0x0005u8]
    mov [r5++], a0h
    or [r5++s], a0
    subh [r5++], a0
    subl [r5--], a0
    mov a0h, [page:0x0006u8]
    mov a0l, [page:0x0007u8]
    mov 0x5a82, y0
    macsu y0, [page:0x0005u8], a0
    clr a0, always
    maa y0, [page:0x0004u8], a0
    add p*, a0
    mov a0h, [page:0x0008u8]
    mov a0l, [page:0x0009u8]
    mov 0x5a82, y0
    macsu y0, [page:0x0007u8], a0
    clr a0, always
    maa y0, [page:0x0006u8], a0
    add p*, a0
    mov a0h, [page:0x000au8]
    mov a0l, [page:0x000bu8]
    mov [r1++], a0h
    or [r1++], a0
    subh [page:0x0008u8], a0
    subl [page:0x0009u8], a0
    mov a0h, [r5++]
    mov a0l, [r5++]
    mov [r1++], a0h
    or [r1++s], a0
    subh [page:0x000au8], a0
    subl [page:0x000bu8], a0
    mov a0h, [r5++]
    mov a0l, [r5]
    mov [r1++], a0h
    or [r1--], a0
    addh [page:0x0008u8], a0
    addl [page:0x0009u8], a0
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov [r1++], a0h
    or [r1--], a0
    addh [page:0x000au8], a0
    addl [page:0x000bu8], a0
    mov a0h, [r1++]
    mov a0l, [r1]
    mov [r2++], a0h
    or [r2++], a0
    addv 0x0002, r6
    subh [r6++], a0
    subl [r6++s], a0
    mov a0h, [page:0x0000u8]
    mov a0l, [page:0x0001u8]
    mov [r2++], a0h
    or [r2++s], a0
    addh [r6++], a0
    addl [r6++], a0
    mov a0h, [page:0x0002u8]
    mov a0l, [page:0x0003u8]
    mov [r2++], a0h
    or [r2--], a0
    addh [r6++], a0
    addl [r6++s], a0
    mov a0h, [r2++]
    mov a0l, [r2++]
    mov [r2++], a0h
    or [r2--], a0
    subh [r6++], a0
    subl [r6--], a0
    mov a0h, [r2++]
    mov a0l, [r2]
    mov [page:0x0000u8], a0h
    or [page:0x0001u8], a0
    mov a0h, [r6++]
    mov a0l, [r6++]
    mov [page:0x0002u8], a0h
    or [page:0x0003u8], a0
    mov a0h, [r6++]
    mov a0l, [r6]
    addv 0x0002, r7
    mov [r7++], a0h
    or [r7++s], a0
    subh [r7++], a0
    subl [r7++], a0
    mov a0h, [page:0x0004u8]
    mov a0l, [page:0x0005u8]
    mov [r7++], a0h
    or [r7++s], a0
    addh [r7++], a0
    addl [r7--], a0
    neg a0, always
    mov a0h, [page:0x0006u8]
    mov a0l, [page:0x0007u8]
    mov 0x5a82, y0
    macsu y0, [page:0x0005u8], a0
    clr a0, always
    maa y0, [page:0x0004u8], a0
    add p*, a0
    mov a0h, [page:0x0008u8]
    mov a0l, [page:0x0009u8]
    mov 0x5a82, y0
    macsu y0, [page:0x0007u8], a0
    clr a0, always
    maa y0, [page:0x0006u8], a0
    add p*, a0
    mov a0h, [page:0x000au8]
    mov a0l, [page:0x000bu8]
    mov [r3++], a0h
    or [r3++], a0
    subh [page:0x0008u8], a0
    subl [page:0x0009u8], a0
    mov a0h, [r7++]
    mov a0l, [r7++]
    mov [r3++], a0h
    or [r3++s], a0
    subh [page:0x000au8], a0
    subl [page:0x000bu8], a0
    mov a0h, [r7++]
    mov a0l, [r7]
    mov [r3++], a0h
    or [r3--], a0
    addh [page:0x0008u8], a0
    addl [page:0x0009u8], a0
    mov a0h, [r3++]
    mov a0l, [r3++]
    mov [r3++], a0h
    or [r3--], a0
    addh [page:0x000au8], a0
    addl [page:0x000bu8], a0
    mov a0h, [r3++]
    mov a0l, [r3]
    pop mod3
    addv 0x001d, r0
    addv 0x001d, r1
    addv 0x001d, r2
    addv 0x001d, r3
    addv 0x001d, r4
    addv 0x001d, r5
    addv 0x001d, r6
    addv 0x001d, r7
.Lb2_end:
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

// radix 4 with twiddles
.section .text.opus_fft_bfly4
.global opus_fft_bfly4
opus_fft_bfly4:
    push mod0
    push cfgi
    push cfgj
    push y0
    push r0
    push r1
    push r2
    push r3
    push r5
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    load 0x0047u8, page
.Lopus_fft_bfly4_group:
    mov [r7], a0
    mov a0l, r0
    addv 0x0004, r7
    add [r7], a0
    mov a0l, r1
    add [r7], a0
    mov a0l, r2
    add [r7], a0
    mov a0l, r3
    addv 0xfffc, r7
    mov [r7], a0
    addv 0x0003, r7
    add [r7], a0
    addv 0xfffd, r7
    mov a0l, [r7]
    addv 0x0005, r7
    mov [r7], a0
    mov a0l, r5
    mov 0x0003, cfgi
    mov 0x007e, cfgj
    addv 0xfffc, r7
    mov [r7], a0
    sub 0x0001u8, a0
    bkrep a0l, .Lopus_fft_bfly4_end
    push mod3
    dint
    clr a0, always
    addv 0x0002, r5
    mov [r5++], y0
    addv 0x0001, r2
    mpysu y0, [r2--], a0
    maa y0, [r2++s], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    mpysu y0, [r2--], a1
    maa y0, [r2--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0000u8]
    mov a0l, [page:0x0001u8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r2--], a0
    maa y0, [r2++s], a0
    mov [r5++s], y0
    macsu y0, [r2--], a0
    maa y0, [r2], a0
    add p*, a0
    mov a0h, [page:0x0002u8]
    mov a0l, [page:0x0003u8]
    mov [r0++], a0h
    or [r0++], a0
    subh [page:0x0000u8], a0
    subl [page:0x0001u8], a0
    mov a0h, [page:0x0004u8]
    mov a0l, [page:0x0005u8]
    mov [r0++], a0h
    or [r0], a0
    subh [page:0x0002u8], a0
    subl [page:0x0003u8], a0
    mov a0h, [page:0x0006u8]
    mov a0l, [page:0x0007u8]
    addv 0xfffd, r0
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x0000u8], a0
    addl [page:0x0001u8], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x0002u8], a0
    addl [page:0x0003u8], a0
    mov a0h, [r0++]
    mov a0l, [r0]
    clr a0, always
    mov [r5++], y0
    addv 0x0001, r1
    mpysu y0, [r1--], a0
    maa y0, [r1++s], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    mpysu y0, [r1--], a1
    maa y0, [r1--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0010u8]
    mov a0l, [page:0x0011u8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r1--], a0
    maa y0, [r1++s], a0
    mov [r5], y0
    macsu y0, [r1--], a0
    maa y0, [r1], a0
    add p*, a0
    mov a0h, [page:0x0012u8]
    mov a0l, [page:0x0013u8]
    clr a0, always
    addv 0x0004, r5
    mov [r5++], y0
    addv 0x0001, r3
    mpysu y0, [r3--], a0
    maa y0, [r3++s], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    mpysu y0, [r3--], a1
    maa y0, [r3--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0014u8]
    mov a0l, [page:0x0015u8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r3--], a0
    maa y0, [r3++s], a0
    mov [r5], y0
    macsu y0, [r3--], a0
    maa y0, [r3], a0
    add p*, a0
    mov a0h, [page:0x0016u8]
    mov a0l, [page:0x0017u8]
    mov [page:0x0010u8], a0h
    or [page:0x0011u8], a0
    addh [page:0x0014u8], a0
    addl [page:0x0015u8], a0
    mov a0h, [page:0x0008u8]
    mov a0l, [page:0x0009u8]
    mov [page:0x0012u8], a0h
    or [page:0x0013u8], a0
    addh [page:0x0016u8], a0
    addl [page:0x0017u8], a0
    mov a0h, [page:0x000au8]
    mov a0l, [page:0x000bu8]
    mov [page:0x0010u8], a0h
    or [page:0x0011u8], a0
    subh [page:0x0014u8], a0
    subl [page:0x0015u8], a0
    mov a0h, [page:0x000cu8]
    mov a0l, [page:0x000du8]
    mov [page:0x0012u8], a0h
    or [page:0x0013u8], a0
    subh [page:0x0016u8], a0
    subl [page:0x0017u8], a0
    mov a0h, [page:0x000eu8]
    mov a0l, [page:0x000fu8]
    addv 0xfffd, r0
    mov [r0++], a0h
    or [r0++], a0
    subh [page:0x0008u8], a0
    subl [page:0x0009u8], a0
    addv 0xfffe, r2
    mov a0h, [r2++]
    mov a0l, [r2++]
    mov [r0++], a0h
    or [r0], a0
    subh [page:0x000au8], a0
    subl [page:0x000bu8], a0
    mov a0h, [r2++]
    mov a0l, [r2++]
    addv 0xfffd, r0
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x0008u8], a0
    addl [page:0x0009u8], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x000au8], a0
    addl [page:0x000bu8], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov [page:0x0004u8], a0h
    or [page:0x0005u8], a0
    addh [page:0x000eu8], a0
    addl [page:0x000fu8], a0
    addv 0xfffe, r1
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov [page:0x0006u8], a0h
    or [page:0x0007u8], a0
    subh [page:0x000cu8], a0
    subl [page:0x000du8], a0
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov [page:0x0004u8], a0h
    or [page:0x0005u8], a0
    subh [page:0x000eu8], a0
    subl [page:0x000fu8], a0
    addv 0xfffe, r3
    mov a0h, [r3++]
    mov a0l, [r3++]
    mov [page:0x0006u8], a0h
    or [page:0x0007u8], a0
    addh [page:0x000cu8], a0
    addl [page:0x000du8], a0
    mov a0h, [r3++]
    mov a0l, [r3++]
    pop mod3
    addv 0x0002, r5
.Lopus_fft_bfly4_end:
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    mov a0l, [r7]
    addv 0xfffe, r7
    cmp 0x0000u8, a0
    br .Lopus_fft_bfly4_group, gt
    pop r7
    pop r5
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgj
    pop cfgi
    pop mod0
    ret always

// radix 3
.section .text.opus_fft_bfly3
.global opus_fft_bfly3
opus_fft_bfly3:
    push mod0
    push cfgi
    push cfgj
    push y0
    push r0
    push r1
    push r2
    push r5
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    load 0x0047u8, page
.Lopus_fft_bfly3_group:
    mov [r7], a0
    mov a0l, r0
    addv 0x0004, r7
    add [r7], a0
    mov a0l, r1
    add [r7], a0
    mov a0l, r2
    addv 0xfffc, r7
    mov [r7], a0
    addv 0x0003, r7
    add [r7], a0
    addv 0xfffd, r7
    mov a0l, [r7]
    addv 0x0005, r7
    mov [r7], a0
    mov a0l, r5
    mov 0x0003, cfgi
    mov 0x0002, cfgj
    addv 0xfffc, r7
    mov [r7], a0
    sub 0x0001u8, a0
    bkrep a0l, .Lopus_fft_bfly3_end
    push mod3
    dint
    clr a0, always
    mov [r5++], y0
    addv 0x0001, r1
    mpysu y0, [r1--], a0
    maa y0, [r1++s], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    mpysu y0, [r1--], a1
    maa y0, [r1--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0000u8]
    mov a0l, [page:0x0001u8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r1--], a0
    maa y0, [r1++s], a0
    mov [r5++s], y0
    macsu y0, [r1--], a0
    maa y0, [r1], a0
    add p*, a0
    mov a0h, [page:0x0002u8]
    mov a0l, [page:0x0003u8]
    clr a0, always
    mov [r5++], y0
    addv 0x0001, r2
    mpysu y0, [r2--], a0
    maa y0, [r2++s], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    mpysu y0, [r2--], a1
    maa y0, [r2--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0004u8]
    mov a0l, [page:0x0005u8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r2--], a0
    maa y0, [r2++s], a0
    mov [r5++s], y0
    macsu y0, [r2--], a0
    maa y0, [r2], a0
    add p*, a0
    mov a0h, [page:0x0006u8]
    mov a0l, [page:0x0007u8]
    mov [page:0x0000u8], a0h
    or [page:0x0001u8], a0
    addh [page:0x0004u8], a0
    addl [page:0x0005u8], a0
    mov a0h, [page:0x0008u8]
    mov a0l, [page:0x0009u8]
    mov [page:0x0002u8], a0h
    or [page:0x0003u8], a0
    addh [page:0x0006u8], a0
    addl [page:0x0007u8], a0
    mov a0h, [page:0x000au8]
    mov a0l, [page:0x000bu8]
    mov [page:0x0000u8], a0h
    or [page:0x0001u8], a0
    subh [page:0x0004u8], a0
    subl [page:0x0005u8], a0
    mov a0h, [page:0x000cu8]
    mov a0l, [page:0x000du8]
    mov [page:0x0002u8], a0h
    or [page:0x0003u8], a0
    subh [page:0x0006u8], a0
    subl [page:0x0007u8], a0
    mov a0h, [page:0x000eu8]
    mov a0l, [page:0x000fu8]
    mov [r0++], a0h
    or [r0++], a0
    mov [page:0x0008u8], a1h
    or [page:0x0009u8], a1
    shfi a1, a1, -1
    sub a1, a0
    addv 0xfffe, r1
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov [r0++], a0h
    or [r0], a0
    mov [page:0x000au8], a1h
    or [page:0x000bu8], a1
    shfi a1, a1, -1
    sub a1, a0
    mov a0h, [r1++]
    mov a0l, [r1]
    mov 0x9126, y0
    macsu y0, [page:0x000du8], a0
    clr a0, always
    maa y0, [page:0x000cu8], a0
    add p*, a0
    mov a0h, [page:0x0010u8]
    mov a0l, [page:0x0011u8]
    mov 0x9126, y0
    macsu y0, [page:0x000fu8], a0
    clr a0, always
    maa y0, [page:0x000eu8], a0
    add p*, a0
    mov a0h, [page:0x0012u8]
    mov a0l, [page:0x0013u8]
    addv 0xfffd, r0
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x0008u8], a0
    addl [page:0x0009u8], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x000au8], a0
    addl [page:0x000bu8], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    addv 0xfffd, r1
    mov [r1++], a0h
    or [r1++], a0
    addh [page:0x0012u8], a0
    addl [page:0x0013u8], a0
    addv 0xfffe, r2
    mov a0h, [r2++]
    mov a0l, [r2++]
    mov [r1++], a0h
    or [r1], a0
    subh [page:0x0010u8], a0
    subl [page:0x0011u8], a0
    mov a0h, [r2++]
    mov a0l, [r2++]
    addv 0xfffd, r1
    mov [r1++], a0h
    or [r1--], a0
    subh [page:0x0012u8], a0
    subl [page:0x0013u8], a0
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov [r1++], a0h
    or [r1--], a0
    addh [page:0x0010u8], a0
    addl [page:0x0011u8], a0
    mov a0h, [r1++]
    mov a0l, [r1++]
    pop mod3
.Lopus_fft_bfly3_end:
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    mov a0l, [r7]
    addv 0xfffe, r7
    cmp 0x0000u8, a0
    br .Lopus_fft_bfly3_group, gt
    pop r7
    pop r5
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgj
    pop cfgi
    pop mod0
    ret always

// radix 5
.section .text.opus_fft_bfly5
.global opus_fft_bfly5
opus_fft_bfly5:
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
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    load 0x0047u8, page
.Lopus_fft_bfly5_group:
    mov [r7], a0
    mov a0l, r0
    addv 0x0004, r7
    add [r7], a0
    mov a0l, r1
    add [r7], a0
    mov a0l, r2
    add [r7], a0
    mov a0l, r3
    add [r7], a0
    mov a0l, r4
    addv 0xfffc, r7
    mov [r7], a0
    addv 0x0003, r7
    add [r7], a0
    addv 0xfffd, r7
    mov a0l, [r7]
    addv 0x0005, r7
    mov [r7], a0
    mov a0l, r5
    mov 0x0003, cfgi
    mov 0x0002, cfgj
    addv 0xfffc, r7
    mov [r7], a0
    sub 0x0001u8, a0
    bkrep a0l, .Lopus_fft_bfly5_end
    push mod3
    dint
    mov [r0++], a0h
    or [r0++], a0
    mov a0h, [page:0x0000u8]
    mov a0l, [page:0x0001u8]
    mov [r0++], a0h
    or [r0], a0
    mov a0h, [page:0x0002u8]
    mov a0l, [page:0x0003u8]
    clr a0, always
    mov [r5++], y0
    addv 0x0001, r1
    mpysu y0, [r1--], a0
    maa y0, [r1++s], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    mpysu y0, [r1--], a1
    maa y0, [r1--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0004u8]
    mov a0l, [page:0x0005u8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r1--], a0
    maa y0, [r1++s], a0
    mov [r5++s], y0
    macsu y0, [r1--], a0
    maa y0, [r1], a0
    add p*, a0
    mov a0h, [page:0x0006u8]
    mov a0l, [page:0x0007u8]
    clr a0, always
    mov [r5++], y0
    addv 0x0001, r2
    mpysu y0, [r2--], a0
    maa y0, [r2++s], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    mpysu y0, [r2--], a1
    maa y0, [r2--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0008u8]
    mov a0l, [page:0x0009u8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r2--], a0
    maa y0, [r2++s], a0
    mov [r5++s], y0
    macsu y0, [r2--], a0
    maa y0, [r2], a0
    add p*, a0
    mov a0h, [page:0x000au8]
    mov a0l, [page:0x000bu8]
    clr a0, always
    mov [r5++], y0
    addv 0x0001, r3
    mpysu y0, [r3--], a0
    maa y0, [r3++s], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    mpysu y0, [r3--], a1
    maa y0, [r3--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x000cu8]
    mov a0l, [page:0x000du8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r3--], a0
    maa y0, [r3++s], a0
    mov [r5++s], y0
    macsu y0, [r3--], a0
    maa y0, [r3], a0
    add p*, a0
    mov a0h, [page:0x000eu8]
    mov a0l, [page:0x000fu8]
    clr a0, always
    mov [r5++], y0
    addv 0x0001, r4
    mpysu y0, [r4--], a0
    maa y0, [r4], a0
    add p*, a0
    clr a1, always
    mov [r5], y0
    addv 0x0003, r4
    mpysu y0, [r4--], a1
    maa y0, [r4--], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0010u8]
    mov a0l, [page:0x0011u8]
    clr a0, always
    mov [r5--], y0
    mpysu y0, [r4--], a0
    maa y0, [r4], a0
    mov [r5++s], y0
    addv 0x0003, r4
    macsu y0, [r4--], a0
    maa y0, [r4], a0
    add p*, a0
    mov a0h, [page:0x0012u8]
    mov a0l, [page:0x0013u8]
    mov [page:0x0004u8], a0h
    or [page:0x0005u8], a0
    addh [page:0x0010u8], a0
    addl [page:0x0011u8], a0
    mov a0h, [page:0x001cu8]
    mov a0l, [page:0x001du8]
    mov [page:0x0006u8], a0h
    or [page:0x0007u8], a0
    addh [page:0x0012u8], a0
    addl [page:0x0013u8], a0
    mov a0h, [page:0x001eu8]
    mov a0l, [page:0x001fu8]
    mov [page:0x0004u8], a0h
    or [page:0x0005u8], a0
    subh [page:0x0010u8], a0
    subl [page:0x0011u8], a0
    mov a0h, [page:0x0028u8]
    mov a0l, [page:0x0029u8]
    mov [page:0x0006u8], a0h
    or [page:0x0007u8], a0
    subh [page:0x0012u8], a0
    subl [page:0x0013u8], a0
    mov a0h, [page:0x002au8]
    mov a0l, [page:0x002bu8]
    mov [page:0x0008u8], a0h
    or [page:0x0009u8], a0
    addh [page:0x000cu8], a0
    addl [page:0x000du8], a0
    mov a0h, [page:0x0020u8]
    mov a0l, [page:0x0021u8]
    mov [page:0x000au8], a0h
    or [page:0x000bu8], a0
    addh [page:0x000eu8], a0
    addl [page:0x000fu8], a0
    mov a0h, [page:0x0022u8]
    mov a0l, [page:0x0023u8]
    mov [page:0x0008u8], a0h
    or [page:0x0009u8], a0
    subh [page:0x000cu8], a0
    subl [page:0x000du8], a0
    mov a0h, [page:0x0024u8]
    mov a0l, [page:0x0025u8]
    mov [page:0x000au8], a0h
    or [page:0x000bu8], a0
    subh [page:0x000eu8], a0
    subl [page:0x000fu8], a0
    mov a0h, [page:0x0026u8]
    mov a0l, [page:0x0027u8]
    addv 0xfffd, r0
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x001cu8], a0
    addl [page:0x001du8], a0
    addh [page:0x0020u8], a0
    addl [page:0x0021u8], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov [r0++], a0h
    or [r0--], a0
    addh [page:0x001eu8], a0
    addl [page:0x001fu8], a0
    addh [page:0x0022u8], a0
    addl [page:0x0023u8], a0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov 0x278e, y0
    macsu y0, [page:0x001du8], a0
    mov [page:0x0000u8], a0h
    or [page:0x0001u8], a0
    maa y0, [page:0x001cu8], a0
    mov 0x9872, y0
    macsu y0, [page:0x0021u8], a0
    maa y0, [page:0x0020u8], a0
    add p*, a0
    mov a0h, [page:0x0014u8]
    mov a0l, [page:0x0015u8]
    mov 0x278e, y0
    macsu y0, [page:0x001fu8], a0
    mov [page:0x0002u8], a0h
    or [page:0x0003u8], a0
    maa y0, [page:0x001eu8], a0
    mov 0x9872, y0
    macsu y0, [page:0x0023u8], a0
    maa y0, [page:0x0022u8], a0
    add p*, a0
    mov a0h, [page:0x0016u8]
    mov a0l, [page:0x0017u8]
    mov 0x8644, y0
    macsu y0, [page:0x002bu8], a0
    clr a0, always
    maa y0, [page:0x002au8], a0
    mov 0xb4c3, y0
    macsu y0, [page:0x0027u8], a0
    maa y0, [page:0x0026u8], a0
    add p*, a0
    mov a0h, [page:0x0018u8]
    mov a0l, [page:0x0019u8]
    mov 0x8644, y0
    macsu y0, [page:0x0029u8], a0
    clr a0, always
    maa y0, [page:0x0028u8], a0
    mov 0xb4c3, y0
    macsu y0, [page:0x0025u8], a0
    maa y0, [page:0x0024u8], a0
    add p*, a0
    neg a0, always
    mov a0h, [page:0x001au8]
    mov a0l, [page:0x001bu8]
    mov [page:0x0014u8], a0h
    or [page:0x0015u8], a0
    subh [page:0x0018u8], a0
    subl [page:0x0019u8], a0
    addv 0xfffe, r1
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov [page:0x0016u8], a0h
    or [page:0x0017u8], a0
    subh [page:0x001au8], a0
    subl [page:0x001bu8], a0
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov [page:0x0014u8], a0h
    or [page:0x0015u8], a0
    addh [page:0x0018u8], a0
    addl [page:0x0019u8], a0
    addv 0xfffe, r4
    mov a0h, [r4++]
    mov a0l, [r4++]
    mov [page:0x0016u8], a0h
    or [page:0x0017u8], a0
    addh [page:0x001au8], a0
    addl [page:0x001bu8], a0
    mov a0h, [r4++]
    mov a0l, [r4++]
    mov 0x9872, y0
    macsu y0, [page:0x001du8], a0
    mov [page:0x0000u8], a0h
    or [page:0x0001u8], a0
    maa y0, [page:0x001cu8], a0
    mov 0x278e, y0
    macsu y0, [page:0x0021u8], a0
    maa y0, [page:0x0020u8], a0
    add p*, a0
    mov a0h, [page:0x002cu8]
    mov a0l, [page:0x002du8]
    mov 0x9872, y0
    macsu y0, [page:0x001fu8], a0
    mov [page:0x0002u8], a0h
    or [page:0x0003u8], a0
    maa y0, [page:0x001eu8], a0
    mov 0x278e, y0
    macsu y0, [page:0x0023u8], a0
    maa y0, [page:0x0022u8], a0
    add p*, a0
    mov a0h, [page:0x002eu8]
    mov a0l, [page:0x002fu8]
    mov 0x8644, y0
    macsu y0, [page:0x0027u8], a0
    clr a0, always
    maa y0, [page:0x0026u8], a0
    add p*, a0
    mov 0xb4c3, y0
    macsu y0, [page:0x002bu8], a1
    clr a1, always
    maa y0, [page:0x002au8], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0030u8]
    mov a0l, [page:0x0031u8]
    mov 0xb4c3, y0
    macsu y0, [page:0x0029u8], a0
    clr a0, always
    maa y0, [page:0x0028u8], a0
    add p*, a0
    mov 0x8644, y0
    macsu y0, [page:0x0025u8], a1
    clr a1, always
    maa y0, [page:0x0024u8], a1
    add p*, a1
    sub a1, a0
    mov a0h, [page:0x0032u8]
    mov a0l, [page:0x0033u8]
    mov [page:0x002cu8], a0h
    or [page:0x002du8], a0
    addh [page:0x0030u8], a0
    addl [page:0x0031u8], a0
    addv 0xfffe, r2
    mov a0h, [r2++]
    mov a0l, [r2++]
    mov [page:0x002eu8], a0h
    or [page:0x002fu8], a0
    addh [page:0x0032u8], a0
    addl [page:0x0033u8], a0
    mov a0h, [r2++]
    mov a0l, [r2++]
    mov [page:0x002cu8], a0h
    or [page:0x002du8], a0
    subh [page:0x0030u8], a0
    subl [page:0x0031u8], a0
    addv 0xfffe, r3
    mov a0h, [r3++]
    mov a0l, [r3++]
    mov [page:0x002eu8], a0h
    or [page:0x002fu8], a0
    subh [page:0x0032u8], a0
    subl [page:0x0033u8], a0
    mov a0h, [r3++]
    mov a0l, [r3++]
    pop mod3
.Lopus_fft_bfly5_end:
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    mov a0l, [r7]
    addv 0xfffe, r7
    cmp 0x0000u8, a0
    br .Lopus_fft_bfly5_group, gt
    pop r7
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

// max |x| and sum |x >> 11|
.section .text.opus_mdct_prescan
.global opus_mdct_prescan
opus_mdct_prescan:
    push mod0
    push cfgi
    push r0
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0002, r7
    mov [r7], cfgi
    addv 0xffff, r7
    mov [r7], a1
    sub 0x0001u8, a1
    clr a0, always
    clr b0, always
    bkrep a1l, .Lprescan_end
    mov [r0++], a1h
    or [r0++s], a1
    shfi a1, b1, -11
    neg a1, lt
    cmp a1, a0
    copy a0, lt
    mov b1, a1
    or 0x0000u8, a1
    neg a1, lt
    add a1, b0
.Lprescan_end:
    addv 0x0002, r7
    mov a0h, [r7]
    addv 0x0001, r7
    mov a0l, [r7]
    addv 0x0001, r7
    mov b0h, [r7]
    addv 0x0001, r7
    mov b0l, [r7]
    pop r7
    pop r0
    pop cfgi
    pop mod0
    ret always

// IMDCT pre-rotation into bit-reversed slots
.section .text.opus_mdct_pre
.global opus_mdct_pre
opus_mdct_pre:
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
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r4
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r5
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r2
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r3
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r1
    addv 0x0002, r7
    mov [r7], cfgi
    addv 0x0001, r7
    mov [r7], cfgj
    addv 0x0001, r7
    mov [r7], sv
    addv 0xfffd, r7
    mov [r7], r7
.Lpre_chunk:
    mov r7, a0
    sub 0x0030u8, a0
    brr .Lpre_last, le
    mov a0l, r7
    mov 0x002f, a0
    brr .Lpre_run, always
.Lpre_last:
    mov r7, a0
    sub 0x0001u8, a0
    mov 0x0000, r7
.Lpre_run:
    push mod3
    dint
    bkrep a0l, .Lpre_end
    mov [r3++], a0
    shl a0, always
    shl a0, always
    add r1, a0
    mov a0l, r6
    mov [r0++], a1h
    or [r0++s], a1
    shfc a1, a1, always
    mov a1, b0
    mov [r4++], a1h
    or [r4++s], a1
    shfc a1, a1, always
    mov a1, b1
    clr a0, always
    clr a1, always
    mov [r5++], y0
    mpysu y0, b1l, a0
    maa y0, b1h, a0
    add p*, a0
    mpysu y0, b0l, a1
    maa y0, b0h, a1
    add p*, a1
    mov [r2++], y0
    mpysu y0, b0l, a0
    maa y0, b0h, a0
    add p*, a0
    mov a0, b0
    clr a0, always
    mpysu y0, b1l, a0
    maa y0, b1h, a0
    add p*, a0
    sub a0, a1
    mov a1h, [r6++]
    mov a1l, [r6++]
    mov b0h, [r6++]
    mov b0l, [r6]
.Lpre_end:
    pop mod3
    cmpv 0x0000, r7
    brr .Lpre_chunk, neq
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

// IMDCT post-rotation, both ends inward
.section .text.opus_mdct_post
.global opus_mdct_post
opus_mdct_post:
    push mod0
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r1
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r2
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r3
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r4
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r5
    addv 0x0002, r7
    mov [r7], sv
    addv 0x0001, r7
    mov [r7], a0h
    addv 0x0001, r7
    or [r7], a0
    mov a0, b1
    addv 0xfffd, r7
    mov [r7], a0
    mov a0l, r7
.Lpost_chunk:
    mov r7, a0
    sub 0x0018u8, a0
    brr .Lpost_last, le
    mov a0l, r7
    mov 0x0017, a0
    brr .Lpost_run, always
.Lpost_last:
    mov r7, a0
    sub 0x0001u8, a0
    mov 0x0000, r7
.Lpost_run:
    push mod3
    dint
    bkrep a0l, .Lpost_end
    clr a0, always
    clr a1, always
    mov [r2++], y0
    addv 0x0003, r0
    mpysu y0, [r0--], a0
    maa y0, [r0--], a0
    add p*, a0
    mpysu y0, [r0--], a1
    maa y0, [r0++], a1
    add p*, a1
    mov [r3++], y0
    mpysu y0, [r0--], a0
    maa y0, [r0], a0
    add p*, a0
    neg a1, always
    addv 0x0003, r0
    mpysu y0, [r0--], a1
    maa y0, [r0], a1
    add p*, a1
    shfi a0, a0, +8
    add b1, a0
    shfc a0, a0, always
    shfi a1, a1, +8
    add b1, a1
    shfc a1, a1, always
    addv 0xfffe, r0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov a1, b0
    clr a0, always
    clr a1, always
    mov [r4--], y0
    addv 0x0003, r1
    mpysu y0, [r1--], a0
    maa y0, [r1--], a0
    add p*, a0
    mpysu y0, [r1--], a1
    maa y0, [r1++], a1
    add p*, a1
    mov [r5--], y0
    mpysu y0, [r1--], a0
    maa y0, [r1], a0
    add p*, a0
    neg a1, always
    addv 0x0003, r1
    mpysu y0, [r1--], a1
    maa y0, [r1], a1
    add p*, a1
    shfi a0, a0, +8
    add b1, a0
    shfc a0, a0, always
    shfi a1, a1, +8
    add b1, a1
    shfc a1, a1, always
    mov b0h, [r1++]
    mov b0l, [r1]
    addv 0xfffd, r1
    mov a0h, [r1++]
    mov a0l, [r1]
    mov a1h, [r0++]
    mov a1l, [r0++]
    addv 0xfffb, r1
.Lpost_end:
    pop mod3
    cmpv 0x0000, r7
    br .Lpost_chunk, neq
    pop r7
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop mod0
    ret always

// TDAC window over the overlap
.section .text.opus_mdct_mirror
.global opus_mdct_mirror
opus_mdct_mirror:
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
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r2
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r3
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    push mod3
    dint
    bkrep a0l, .Lmirror_end
    clr a0, always
    clr a1, always
    mov [r3--], y0
    addv 0x0001, r1
    mpysu y0, [r1--], a0
    maa y0, [r1++], a0
    add p*, a0
    addv 0x0001, r0
    mpysu y0, [r0--], a1
    maa y0, [r0++], a1
    add p*, a1
    mov [r2++], y0
    mpysu y0, [r1--], a1
    maa y0, [r1], a1
    add p*, a1
    mov a1, b0
    clr a1, always
    mpysu y0, [r0--], a1
    maa y0, [r0], a1
    add p*, a1
    sub a1, a0
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov b0h, [r0++]
    mov b0l, [r0]
    addv 0xfffd, r0
.Lmirror_end:
    pop mod3
    pop r7
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop mod0
    ret always

// PSHR32 over the FFT buffer
.section .text.opus_fft_downshift
.global opus_fft_downshift
opus_fft_downshift:
    push mod0
    push r0
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0002, r7
    mov [r7], sv
    addv 0x0001, r7
    mov [r7], a0h
    addv 0x0001, r7
    or [r7], a0
    mov a0, b1
    addv 0xfffd, r7
    mov [r7], a0
    sub 0x0001u8, a0
    bkrep a0l, .Ldown_end
    mov [r0++], a0h
    or [r0--], a0
    shfi a0, a0, +8
    add b1, a0
    shfc a0, a0, always
    mov a0h, [r0++]
    mov a0l, [r0++]
.Ldown_end:
    pop r7
    pop r0
    pop mod0
    ret always

// twiddles k*j*stride, grouped per j
.section .text.opus_fft_gather
.global opus_fft_gather
opus_fft_gather:
    push mod0
    push cfgj
    push y0
    push r0
    push r4
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    addv 0x0005, r7
    mov [r7], cfgj
    clr b0, always
    addv 0xfffb, r7
.Lgather_k:
    addv 0x0003, r7
    mov [r7], a0
    add b0, a0
    mov a0, b0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r4
    add 0x0002u8, a0
    mov a0l, [r7]
    addv 0xfffc, r7
    mov [r7], a1
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    bkrep a0l, .Lgather_end
    mov a1l, r0
    mov [r0++], y0
    mov y0, [r4++]
    mov [r0], y0
    mov y0, [r4++s]
    add b0, a1
.Lgather_end:
    addv 0x0001, r7
    mov [r7], a0
    sub 0x0001u8, a0
    mov a0l, [r7]
    addv 0xfffe, r7
    cmp 0x0000u8, a0
    br .Lgather_k, gt
    pop r7
    pop r4
    pop r0
    pop y0
    pop cfgj
    pop mod0
    ret always
