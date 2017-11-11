/* Each of the following is a low-level interrupt handler: each one is
 * tasked with handling a different interrupt type, and acts as a sort
 * of wrapper around a high-level, C-based handler.
 */

.global lolevel_handler_rst
.global lolevel_handler_irq
.global lolevel_handler_svc
.global lolevel_handler_pab
.global lolevel_handler_dab

lolevel_handler_rst: bl    int_init                @ initialise interrupt vector table

                     msr   cpsr, #0xD2             @ enter IRQ mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_irq            @ initialise IRQ mode stack
                     msr   cpsr, #0xD7             @ enter ABT mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_abt            @ initialise ABT mode stack
                     msr   cpsr, #0xD3             @ enter SVC mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_svc            @ initialise SVC mode stack

                     movw r0, #0x0000
                     movt r0, #0xFFFF              @load 0xFFFF0000
                     mcr  p15, 0, r0, c12, c0, 0   @set vectore base address to 0xFFFF0000

                     sub   sp, sp, #68             @ initialise dummy context

                     mov   r0, sp                  @ set    high-level C function arg. = SP
                     bl    rst_handler     @ invoke high-level C function

                     ldmia sp!, { r0, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r0                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60             @ update SVC mode SP
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_irq: sub   lr, lr, #4              @ correct return address
                     msr   cpsr, #0xD2             @ enter IRQ mode with IRQ and FIQ interrupts disabled
                     sub   sp, sp, #60             @ update SVC mode stack
                     stmia sp, { r0-r12, sp, lr }^ @ store  USR registers
                     mrs   r0, spsr                @ get    USR        CPSR
                     stmdb sp!, { r0, lr }         @ store  USR PC and CPSR
                     mov   r0, sp

                     bl    irq_handler             @ invoke high-level C function

                     ldmia sp!, { r0, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r0                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60             @ update SVC mode SP
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_svc: sub   lr, lr, #0              @ correct return address
                     sub   sp, sp, #60             @ update SVC mode stack
                     stmia sp, { r0-r12, sp, lr }^ @ store  USR registers
                     mrs   r5, spsr                @ get    USR        CPSR
                     stmdb sp!, { r5, lr }         @ store  USR PC and CPSR
                     msr   cpsr, #0xD3             @ enter SVC mode with IRQ and FIQ interrupts disabled

                     add   r5, sp, #68
                     ldm   r5, {r5}
                     and   r5, r5, #0x1F

                     mov   r3, sp

                     sub   r4, lr, #4
                     ldm   r4, {r4}
                     and   r4, r4, #0xFFFFFF

                     stmdb sp!, {r5}
                     stmdb sp!, {r4}

                     bl    svc_handler             @ invoke high-level C function

                     ldmia sp!, {r4}
                     ldmia sp!, {r5}

                     ldmia sp!, { r1, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r1                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_pab: sub   lr, lr, #4              @ correct return address
                     msr   cpsr, #0xD7             @ enter ABT mode with IRQ and FIQ interrupts disabled
                     sub   sp, sp, #60             @ update SVC mode stack
                     stmia sp, { r0-r12, sp, lr }^ @ store  USR registers
                     mrs   r5, spsr                @ get    USR        CPSR
                     stmdb sp!, { r5, lr }         @ store  USR PC and CPSR

                     mov   r0, lr
                     mov   r1, sp
                     bl    pab_handler             @ invoke high-level C function

                     ldmia sp!, { r1, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r1                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_dab: sub   lr, lr, #8              @ correct return address
                     msr   cpsr, #0xD7             @ enter ABT mode with IRQ and FIQ interrupts disabled
                     sub   sp, sp, #60             @ update SVC mode stack
                     stmia sp, { r0-r12, sp, lr }^ @ store  USR registers
                     mrs   r5, spsr                @ get    USR        CPSR
                     stmdb sp!, { r5, lr }         @ store  USR PC and CPSR

                     mov   r0, lr
                     ldm   r0, {r0}
                     mov   r1, sp
                     bl    dab_handler             @ invoke high-level C function

                     ldmia sp!, { r1, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r1                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60
                     movs  pc, lr                  @ return from interrupt

test:                ldrb r3, [r3, #-1024]
                     ldr r3, [r3, #-1024]
                     ldrb r3, [r3, #1024]
                     ldrb r3, [r3, r4]
                     ldrb r3, [r3, r4, lsl #5]
                     ldrb r3, [r3, r4, lsr #5]