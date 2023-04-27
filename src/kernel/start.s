.section ".text.start"
.extern _stack_top
.extern _DTB_ADDRESS
.extern _init
.global _start
_start:
	adr x1, _DTB_ADDRESS
	str x0, [x1]
	adrp x0, _stack_top
	//ldr x0, =_stack_top
	mov sp, x0
	bl _init
_hang:
	wfe
	b _hang
