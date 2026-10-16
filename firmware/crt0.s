// SPDX-License-Identifier: BSD-2-Clause
// Vectors, core setup, interrupt wrappers.
.section .text.start
.global _start
_start:
    br      reset, always           // 0x0000
    br      trap_vec, always        // 0x0002
    br      nmi_vec, always         // 0x0004
    br      int0_vec, always        // 0x0006 output
    nop
    nop
    nop
    nop
    nop
    nop
    br      int1_vec, always        // 0x000E
    nop
    nop
    nop
    nop
    nop
    nop
    br      int2_vec, always        // 0x0016

.text
reset:
    dint
    // mode registers: fixed state for compiled code
    mov     0x0000, mod3
    rst     0x6ce1, mod0
    set     0x0002, mod0
    rst     0xd0ff, mod1
    set     0x2000, mod1
    mov     0x0000, mod2
    mov     0x0000, cfgi
    mov     0x0000, cfgj
    mov     0x0000, stepi0
    mov     0x0000, stepj0
    mov     0x0021, arp0
    mov     0x258c, arp1
    mov     0x4ab5, arp2
    mov     0x6c63, arp3
    mov     0, sp
    addv    0x07ff, sp
    // memory layout: MMIO at 0x7800
    mov     0x8114, r0
    mov     0x0100, a0
    mov     a0l, [r0]
    mov     0x811e, r0
    mov     0x7800, a0
    mov     a0l, [r0]
    mov     0x7914, r0
    mov     0x201e, a0
    mov     a0l, [r0]
    mov     0x791a, r0
    mov     [r0], a0
    or      0x0040, a0
    mov     a0l, [r0]
    mov     0x790e, r0
    mov     0x0, a0
    mov     a0l, [r0]
    mov     0x7910, r0
    mov     a0l, [r0]
    // boot handshake: REPLY0..2 = 1
    mov     0x78c0, r0
    mov     0x78c4, r1
    mov     0x78c8, r2
    mov     0x1, a0
    mov     a0l, [r0]
    call    apbp_delay, always
    mov     0x1, a0
    mov     a0l, [r1]
    call    apbp_delay, always
    mov     0x1, a0
    mov     a0l, [r2]
    call    apbp_delay, always
    // 32-bit access through r0, r4, r2, r5
    mov     0x110c, ar0
    mov     0x56a3, ar1
    call    main, always
hang:
    br      hang, always

trap_vec:
    reti    always
nmi_vec:
    retic   always
int2_vec:
    reti    always

// Save caller-saved state, call the C handler.
// Not saved: stt2 (ends block loops), p0/p1. The handlers must not use
// block repeat; kernels without the multiplier run with interrupts on.
int0_vec:
    push    stt0
    push    stt1
    push    mod0
    set     0x0001, mod0            // pusha without saturation
    rst     0x0c80, mod0            // arithmetic shifts, product shift 0
    push    a0e
    pusha   a0
    push    a1e
    pusha   a1
    push    b0e
    pusha   b0
    push    b1e
    pusha   b1
    push    x0
    push    x1
    push    sv
    push    y0
    push    y1
    push    r5
    push    r6
    push    mod1
    push    mod2
    push    cfgi
    push    cfgj
    push    ar0
    push    ar1
    call    isr_int0, always
    br      isr_restore, always
int1_vec:
    push    stt0
    push    stt1
    push    mod0
    set     0x0001, mod0
    rst     0x0c80, mod0
    push    a0e
    pusha   a0
    push    a1e
    pusha   a1
    push    b0e
    pusha   b0
    push    b1e
    pusha   b1
    push    x0
    push    x1
    push    sv
    push    y0
    push    y1
    push    r5
    push    r6
    push    mod1
    push    mod2
    push    cfgi
    push    cfgj
    push    ar0
    push    ar1
    call    isr_int1, always
isr_restore:
    pop     ar1
    pop     ar0
    pop     cfgj
    pop     cfgi
    pop     mod2
    pop     mod1
    pop     r6
    pop     r5
    pop     y1
    pop     y0
    pop     sv
    pop     x1
    pop     x0
    popa    b1
    pop     b1e
    popa    b0
    pop     b0e
    popa    a1
    pop     a1e
    popa    a0
    pop     a0e
    pop     mod0
    pop     stt1
    pop     stt0
    reti    always

.weak isr_int0
.weak isr_int1
isr_int0:
isr_int1:
    ret     always

// pause after an APBP access
.global apbp_delay
apbp_delay:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    ret     always

.global irq_enable
irq_enable:
    eint
    ret     always

.global irq_disable
irq_disable:
    dint
    ret     always

// a0l = mod3 bits to set
.global irq_unmask
irq_unmask:
    mov     mod3, a1l
    or      a0l, a1
    mov     a1l, mod3
    ret     always
