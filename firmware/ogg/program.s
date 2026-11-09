// SPDX-License-Identifier: BSD-2-Clause
.section .text.ogg_program_write
.global ogg_program_write
.type ogg_program_write,@function
ogg_program_write:
    push mod3
    dint
    push mod2
    rst 0x1111, mod2
    push stt2
    push r0
    push r3
    push r4
    mov a0l, r4
    mov a1l, r0
    mov b0l, r3
    mov r3, a0
    br .Lprogram_done, eq
    push stt2
    pop a0l
    and 0xff3f, a0
    mov b1l, a1
    and 0x3, a1
    shfi a1, a1, +6
    or a1, a0
    push a0l
    pop stt2
    // MOVD [r0++], [r4++]
    .short 0x5fa8
    addv 0xffff, r3
    mov r3, a0
    br .Lprogram_done, eq
    and 1, a0
    br .Lprogram_pairs, eq
    .short 0x5fa8
    addv 0xffff, r3
    mov r3, a0
    br .Lprogram_done, eq
.Lprogram_pairs:
    mov r3, a0
    and 0xffff, a0
    shfi a0, a0, -1
    mov a0l, r3
    addv 0xffff, r3
    bkrep r3, .Lprogram_end
    .short 0x5fa8
    .short 0x5fa8
.Lprogram_end:
.Lprogram_done:
    pop r4
    pop r3
    pop r0
    pop stt2
    pop mod2
    pop mod3
    ret always
.size ogg_program_write, .-ogg_program_write
