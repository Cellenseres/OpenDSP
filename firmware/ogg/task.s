// SPDX-License-Identifier: BSD-2-Clause
.text
.global ogg_task_step
.global ogg_task_yield
.macro save_task
    push stt0
    push mod3
    dint
    push stt1
    push stt2
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
.endm

ogg_task_step:
    save_task
    mov 0x0f81, r0
    mov sp, a0
    mov a0l, [r0]
    mov 0x7910, r0
    mov [r0], a0
    mov 0x0f84, r0
    mov a0l, [r0]
    mov 0x7910, r0
    mov 1, a0
    mov a0l, [r0]
    mov 0x0f80, r0
    mov [r0], a0
    br .Ltask_start, eq
    mov 0x790e, r0
    mov 1, a1
    mov a1l, [r0]
    mov a0l, sp
    br .Ltask_restore, always
.Ltask_start:
    mov 0x0f83, r0
    mov [r0], a0
    mov 0x790e, r0
    mov 1, a1
    mov a1l, [r0]
    mov a0l, sp
    rst 0x0c80, mod0
    set 0x0003, mod0
    call ogg_task_main, always
.Ltask_dead:
    br .Ltask_dead, always

ogg_task_yield:
    save_task
    mov sp, a0
    mov 0x790e, r0
    mov 0, a1
    mov a1l, [r0]
    mov 0x0f80, r0
    mov a0l, [r0]
    mov 0x0f84, r0
    mov [r0], a0
    mov 0x7910, r0
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
    pop stt2
    pop stt1
    pop mod3
    pop stt0
    ret always

// FIFO top-up through the IRQ entry
.set int0_entry, _start + 6
.global output_kick
output_kick:
    push mod3
    dint
    call int0_entry, always
    pop mod3
    ret always

.global ogg_task_read
ogg_task_read:
    push mod3
    dint
    push r0
    push r1
    mov a0l, r1
    mov 0x790e, r0
    mov 1, b0
    mov b0l, [r0]
    // Page switch lags on hardware.
    nop
    nop
    mov [r1], a0l
    mov 0, b0
    mov b0l, [r0]
    nop
    nop
    pop r1
    pop r0
    pop mod3
    ret always

.global ogg_task_write
ogg_task_write:
    push mod3
    dint
    push r0
    push r1
    mov a0l, r1
    mov 0x790e, r0
    mov 1, b0
    mov b0l, [r0]
    nop
    nop
    mov a1l, [r1]
    mov 0, b0
    mov b0l, [r0]
    nop
    nop
    pop r1
    pop r0
    pop mod3
    ret always
