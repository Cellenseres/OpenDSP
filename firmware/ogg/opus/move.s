// SPDX-License-Identifier: BSD-2-Clause
// Word move with one poll per chunk.
.section .text.ogg_move_words
.global ogg_move_words
.type ogg_move_words,@function
.global ogg_copy_words
ogg_copy_words:
ogg_move_words:
    push r0
    push r3
    push r4
    mov a0l, r4
    mov a1l, r0
    mov b0l, r3
    and 0xffff, a0
    and 0xffff, a1
    cmp a1, a0
    br .Lmove_done, eq
    br .Lmove_back, gt
.Lforward_chunk:
    cmpv 0x0000, r3
    br .Lmove_done, eq
    mov r3, a0
    and 0xffff, a0
    cmp 0x0200, a0
    brr .Lforward_some, le
    mov 0x0200, a0
.Lforward_some:
    mov r3, a1
    sub a0, a1
    mov a1l, r3
    addv 0xffff, a0l
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    push mod2
    rst 0x1111, mod2
    bkrep a0l, .Lforward_end
    mov [r0++], a0
    mov a0l, [r4++]
.Lforward_end:
    pop mod2
    pop mod0
    call ogg_opus_poll, always
    br .Lforward_chunk, always
.Lmove_back:
    cmpv 0x0000, r3
    br .Lmove_done, eq
    mov r3, a0
    and 0xffff, a0
    sub 0x0001, a0
    mov r0, a1
    add a0, a1
    mov a1l, r0
    mov r4, a1
    add a0, a1
    mov a1l, r4
.Lback_chunk:
    cmpv 0x0000, r3
    br .Lmove_done, eq
    mov r3, a0
    and 0xffff, a0
    cmp 0x0200, a0
    brr .Lback_some, le
    mov 0x0200, a0
.Lback_some:
    mov r3, a1
    sub a0, a1
    mov a1l, r3
    addv 0xffff, a0l
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    push mod2
    rst 0x1111, mod2
    bkrep a0l, .Lback_end
    mov [r0--], a0
    mov a0l, [r4--]
.Lback_end:
    pop mod2
    pop mod0
    call ogg_opus_poll, always
    br .Lback_chunk, always
.Lmove_done:
    pop r4
    pop r3
    pop r0
    ret always
.size ogg_move_words, .-ogg_move_words
