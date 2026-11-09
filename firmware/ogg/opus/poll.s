// SPDX-License-Identifier: BSD-2-Clause
// Decoder poll, fast path keeps all but flags.
// 0x45ff: counter word below OGG_TASK_GUARD, task page.
.section .text.ogg_opus_poll
.global ogg_opus_poll
ogg_opus_poll:
    push r0
    mov 0x790e, r0
    cmpv 0x0001, [r0]
    brr .Lpoll_out, neq
    mov 0x45ff, r0
    addv 0x0001, [r0]
    tst0 0x0007, [r0]
    brr .Lpoll_slow, eq
    mov 0x7ac2, r0
    tst0 0x0008, [r0]
    brr .Lpoll_slow, eq
.Lpoll_out:
    pop r0
    ret always
.Lpoll_slow:
    pop r0
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
    push sv
    call ogg_opus_poll_slow, always
    pop sv
    popa b1
    pop b1e
    popa b0
    pop b0e
    popa a1
    pop a1e
    popa a0
    pop a0e
    pop mod0
    ret always
