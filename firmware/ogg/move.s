// SPDX-License-Identifier: BSD-2-Clause
.section .text.ogg_copy_forward
.global ogg_copy_forward
.type ogg_copy_forward,@function
ogg_copy_forward:
    push mod3
    dint
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    push mod2
    rst 0x1111, mod2
    push r0
    push r3
    push r4
    mov a0l, r0
    mov a1l, r4
    mov b0l, r3
    cmpv 0, r3
    br .Lforward_done, eq
    addv 0xffff, r3
    bkrep r3, .Lforward_end
    mov [r0++], a0
    mov a0l, [r4++]
.Lforward_end:
.Lforward_done:
    pop r4
    pop r3
    pop r0
    pop mod2
    pop mod0
    pop mod3
    ret always
.size ogg_copy_forward, .-ogg_copy_forward

.section .text.ogg_copy_backward
.global ogg_copy_backward
.type ogg_copy_backward,@function
ogg_copy_backward:
    push mod3
    dint
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    push mod2
    rst 0x1111, mod2
    push r0
    push r3
    push r4
    mov a0l, r0
    mov a1l, r4
    mov b0l, r3
    cmpv 0, r3
    br .Lbackward_done, eq
    addv 0xffff, r3
    mov r3, a1
    mov r0, a0
    add a1, a0
    mov a0l, r0
    mov r4, a0
    add a1, a0
    mov a0l, r4
    bkrep r3, .Lbackward_end
    mov [r0--], a0
    mov a0l, [r4--]
.Lbackward_end:
.Lbackward_done:
    pop r4
    pop r3
    pop r0
    pop mod2
    pop mod0
    pop mod3
    ret always
.size ogg_copy_backward, .-ogg_copy_backward
