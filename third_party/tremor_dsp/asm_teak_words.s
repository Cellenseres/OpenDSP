// SPDX-License-Identifier: BSD-2-Clause
// Codeword insert for _make_words, header bank only.

// a0l = r (u32 nodes, high word first), a1l = marker[33],
// b0l = state {length, top, count}; top is updated.
// Returns a0l = leaf slot address, 0 on error.
// Node indices must stay below 0x8000.
.section .overlay.6,"ax",@progbits
.global tremor_make_word
.type tremor_make_word,@function
tremor_make_word:
    push mod0
    push x0
    push y0
    push sv
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r0
    mov a1l, r1
    mov b0l, r2
    // b0 = entry = marker[length]
    mov [r2], a0
    shfi a0, a0, +1
    add r1, a0
    mov a0l, r3
    mov [r3++], a0l
    shfi a0, a0, +16
    or [r3], a0
    mov a0, b0
    cmp 0x0u8, a0
    br .Lword_entry, neq
    mov r2, r7
    addv 0x0002, r7
    mov [r7], a0
    br .Lword_error, neq
.Lword_entry:
    mov r2, r7
    addv 0x0001, r7
    mov [r7], r4
    mov 0x0000, r5
    // probe: codeword MSB at bit 38
    mov [r2], a0
    mov a0l, r6
    neg a0, always
    add 0x0020, a0
    mov a0l, sv
    mov b0, a1
    shfi a1, a1, +7
    shfc a1, a1, always
    mov r6, a0
    dec a0, always
    mov a0l, r6
    br .Lword_last, eq
.Lword_chase:
    mov 0x0000, r3
    shfi a1, a1, +1
    br .Lword_bit0, ge
    mov 0x0002, r3
.Lword_bit0:
    mov r5, a0
    shfi a0, a0, +2
    add r0, a0
    mov a0l, r7
    mov r5, a0
    cmp r4, a0
    br .Lword_new, ge
    mov r7, a0
    add r3, a0
    mov a0l, r7
    // a leaf here means a broken book
    mov [r7], a0
    br .Lword_error, neq
    addv 0x0001, r7
    mov [r7], a0
    br .Lword_have, neq
    mov r4, [r7]
    mov r4, r5
    br .Lword_next, always
.Lword_have:
    mov a0l, r5
    br .Lword_next, always
.Lword_new:
    mov r4, a0
    inc a0, always
    mov a0l, r4
    clr a0, always
    mov a0l, [r7++]
    mov r4, [r7++]
    mov a0l, [r7++]
    mov a0l, [r7]
    mov r4, r5
    mov r3, a0
    br .Lword_next, eq
    mov 0x0000, r5
.Lword_next:
    mov r6, a0
    dec a0, always
    mov a0l, r6
    br .Lword_chase, neq
.Lword_last:
    mov 0x0000, r3
    shfi a1, a1, +1
    br .Lword_lbit0, ge
    mov 0x0002, r3
.Lword_lbit0:
    mov r5, a0
    shfi a0, a0, +2
    add r0, a0
    mov a0l, r7
    mov r5, a0
    cmp r4, a0
    br .Lword_slot, lt
    mov r4, a0
    inc a0, always
    mov a0l, r4
    clr a0, always
    addv 0x0002, r7
    mov a0l, [r7++]
    mov a0l, [r7]
    addv 0xfffd, r7
.Lword_slot:
    mov r7, a0
    add r3, a0
    mov a0l, r3
    mov r2, r7
    addv 0x0001, r7
    mov r4, [r7]
    // marker update, j = length .. 1
    mov [r2], a0
    mov a0l, r6
    shfi a0, a0, +1
    add r1, a0
    mov a0l, r7
.Lword_mark:
    mov [r7++], a0l
    shfi a0, a0, +16
    or [r7], a0
    mov [r7], a1l
    and 0x0001, a1
    br .Lword_odd, neq
    inc a0, always
    addv 0xffff, r7
    mov a0h, [r7++]
    mov a0l, [r7]
    addv 0xfffd, r7
    mov r6, a0
    dec a0, always
    mov a0l, r6
    br .Lword_mark, neq
    br .Lword_prune, always
.Lword_odd:
    addv 0xfffd, r7
    mov [r7++], a0l
    shfi a0, a0, +16
    or [r7], a0
    shfi a0, a0, +1
    addv 0x0001, r7
    mov a0h, [r7++]
    mov a0l, [r7]
.Lword_prune:
    // j = length + 1 .. 32 while marker[j] >> 1 == entry
    mov [r2], a0
    neg a0, always
    add 0x0020, a0
    mov a0l, r6
    br .Lword_done, eq
    mov [r2], a0
    inc a0, always
    shfi a0, a0, +1
    add r1, a0
    mov a0l, r7
.Lword_pl:
    mov [r7++], a0l
    shfi a0, a0, +16
    or [r7], a0
    mov a0, a1
    shfi a0, a0, -1
    cmp b0, a0
    br .Lword_done, neq
    mov a1, b0
    addv 0xfffd, r7
    mov [r7++], a0l
    shfi a0, a0, +16
    or [r7], a0
    shfi a0, a0, +1
    addv 0x0001, r7
    mov a0h, [r7++]
    mov a0l, [r7++]
    mov r6, a0
    dec a0, always
    mov a0l, r6
    br .Lword_pl, neq
.Lword_done:
    mov r3, a0
    br .Lword_out, always
.Lword_error:
    clr a0, always
.Lword_out:
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop sv
    pop y0
    pop x0
    pop mod0
    ret always

// One run of unordered codeword lengths.
// a0l = state: word, shift, words, left,
// used, max, sparse, stop.
// words 0xffff: word lies past window.
// Returns stopped entry length, else 0.
.macro LENGTHS name, sparse, stop
.L\name\()_loop:
    mov r3, a0
    br .Llen_empty, eq
    mov r2, a0
    cmp 0x0006, a0
    br .L\name\()_have, ge
    mov r0, a0
    br .Llen_empty, eq
    dec a0, always
    mov a0l, r0
    mov [r7++], a0l
    mov r2, sv
    shfc a0, a0, always
    or a0, a1
    addv 0x0010, r2
.L\name\()_have:
.if \sparse
    tst0 r1, a1
    br .L\name\()_used, neq
    shfi a1, a1, -1
    addv 0xffff, r2
    addv 0xffff, r3
    br .L\name\()_loop, always
.L\name\()_used:
    shfi a1, a1, -1
    addv 0xffff, r2
.endif
    mov a1, a0
    and y0, a0
    inc a0, always
    shfi a1, a1, -5
    addv 0xfffb, r2
    addv 0xffff, r3
.if \stop
    br .Llen_out, always
.else
    addv 0x0001, r4
    cmp r5, a0
    br .L\name\()_loop, le
    mov a0l, r5
    br .L\name\()_loop, always
.endif
.endm

.global tremor_lengths_run
.type tremor_lengths_run,@function
tremor_lengths_run:
    push mod0
    push x0
    push y0
    push sv
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r6
    mov [r6++], a0l
    mov a0l, r7
    // a1 = first word >> shift
    mov [r6++], a0l
    neg a0, always
    mov a0l, sv
    add 0x0010, a0
    mov a0l, r2
    mov [r7++], a1l
    shfc a1, a1, always
    mov [r6++], a0l
    mov a0l, r0
    mov [r6++], a0l
    mov a0l, r3
    mov [r6++], a0l
    mov a0l, r4
    mov [r6++], a0l
    mov a0l, r5
    mov 0x0001, r1
    mov 0x001f, y0
    mov [r6++], a0
    br .Llen_sparse, neq
    mov [r6], a0
    br .Llen_dense_stop, neq
    LENGTHS dense_scan, 0, 0
.Llen_dense_stop:
    LENGTHS dense_stop, 0, 1
.Llen_sparse:
    mov [r6], a0
    br .Llen_sparse_stop, neq
    LENGTHS sparse_scan, 1, 0
.Llen_sparse_stop:
    LENGTHS sparse_stop, 1, 1
.Llen_empty:
    clr a0, always
.Llen_out:
    // give back the words still buffered
    mov r2, a1
    br .Llen_whole, eq
.Llen_back:
    addv 0xffff, r7
    addv 0x0001, r0
    sub 0x0010, a1
    br .Llen_back, gt
    neg a1, always
    mov a1l, r2
.Llen_whole:
    addv 0xffff, r0
    addv 0xfff9, r6
    mov r7, [r6++]
    mov r2, [r6++]
    mov r0, [r6++]
    mov r3, [r6++]
    mov r4, [r6++]
    mov r5, [r6]
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop sv
    pop y0
    pop x0
    pop mod0
    ret always

// Leaf value of one used entry.
// a0l = state: entry, quantvals, dim,
// shift, table (0: column offset).
// Returns a0 = packed value.
.global tremor_decpack
.type tremor_decpack,@function
tremor_decpack:
    push mod0
    push sv
    push r0
    push r1
    push r2
    push r3
    push r6
    push r7
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r6
    mov [r6++], a0l
    mov a0l, r0
    // b0 = quantvals << 16
    mov [r6++], a0l
    shfi a0, a0, +16
    mov a0, b0
    mov [r6++], a0l
    mov a0l, r1
    mov [r6++], a0l
    mov a0l, r2
    mov [r6], a0l
    mov a0l, r3
    clr b1, always
    mov 0x0000, sv
    mov r1, a0
    br .Ldp_done, eq
.Ldp_loop:
    // a0 = remainder << 16 | quotient
    mov r0, a0l
.rept 16
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
.endr
    mov a0l, r0
    shfi a0, a0, -16
    mov r3, a1
    br .Ldp_value, eq
    add r3, a0
    mov a0l, r7
    mov [r7], a0l
.Ldp_value:
    shfc a0, a0, always
    mov b1, a1
    or a0, a1
    mov a1, b1
    mov sv, a1
    add r2, a1
    mov a1l, sv
    mov r1, a0
    dec a0, always
    mov a0l, r1
    br .Ldp_loop, neq
.Ldp_done:
    mov b1, a0
    pop r7
    pop r6
    pop r3
    pop r2
    pop r1
    pop r0
    pop sv
    pop mod0
    ret always
