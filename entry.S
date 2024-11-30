.syntax unified

.pool
.set NEW_LR,        0x200043C0

.global	_start
.arm
.type	_start, %function

_start:
    bl  fix_usb_lr
    cmp r0, #0x0
    bne hang
    bl  usb_desc_fix
    bl  hook_init
ret:
    ldr pc, =NEW_LR
