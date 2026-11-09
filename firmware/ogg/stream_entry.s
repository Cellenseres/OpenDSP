// SPDX-License-Identifier: BSD-2-Clause
.text
.global ogg_stream_command
ogg_stream_command:
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    call ogg_stream_command_impl, always
    pop mod0
    ret always

.global ogg_stream_poll
ogg_stream_poll:
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    call ogg_stream_poll_impl, always
    pop mod0
    ret always

.global ogg_stream_quiesce
ogg_stream_quiesce:
    push mod3
    dint
    push r0
    mov 0x790e, r0
    mov 1, a1
    mov a1l, [r0]
    mov 0x7910, r0
    mov a1l, [r0]
    call ogg_dma_quiesce, always
    call ogg_dma_error, always
    mov 0x790e, r0
    mov 0, a1
    mov a1l, [r0]
    mov 0x7910, r0
    mov a1l, [r0]
    pop r0
    pop mod3
    ret always

.global ogg_stream_guard_error
.type ogg_stream_guard_error,@function
ogg_stream_guard_error:
    push mod3
    dint
    push mod2
    rst 0x1111, mod2
    push r0
    push r1
    push r2
    push r3
    push r4
    mov 0x790e, r0
    mov 1, a1
    mov a1l, [r0]
    mov 0x7016, r1
    mov [r1], r2
    cmpv 0, r2
    br .Lguard_failed, neq
    mov 0x7020, r1
    mov [r1], r2
    cmpv 0, r2
    br .Lguard_failed, neq
    mov 0x4600, r1
    mov 0x2d60, r4
    mov 0, a0
    mov 0xa55a, r2
    mov 31, r3
    // differences ORed together, no branches
    bkrep r3, .Lguard_end
    mov [r1++], a1
    sub r2, a1
    or a1, a0, a0
    mov [r4++], a1
    sub r2, a1
    or a1, a0, a0
.Lguard_end:
    br .Lguard_failed, neq
    br .Lguard_restore, always
.Lguard_failed:
    mov 1, a0
.Lguard_restore:
    mov 0, a1
    mov a1l, [r0]
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop mod2
    pop mod3
    ret always
.size ogg_stream_guard_error, .-ogg_stream_guard_error
