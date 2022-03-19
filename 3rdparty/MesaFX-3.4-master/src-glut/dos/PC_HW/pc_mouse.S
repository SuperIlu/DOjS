	.file	"pc_mouse.c"
/APP
	
		.text				
		.p2align 5,,31			
		.global	_mouse_wrap		
_mouse_wrap:					
		cld				
		lodsl				
		movl	%eax, %es:42(%edi)	
		addw	$4, %es:46(%edi)	
		pushl	%es			
		movl	%ss, %ebx		
		movl	%esp, %esi		
		lss	%cs:_mouse_wrap_end, %esp
		pushl	%ss			
		pushl	%ss			
		popl	%es			
		popl	%ds			
		mov	___djgpp_dos_sel, %fs	
		pushl	%fs			
		popl	%gs			
		pushl	%edi			
		call	_mouse			
		popl	%edi			
		movl	%ebx, %ss		
		movl	%esi, %esp		
		popl	%es			
		iret				
		.global	_mouse_wrap_end		
_mouse_wrap_end:.long	0, 0
/NO_APP
	.section .text
	.p2align 4,,15
_mouse:
LFB3:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$12, %esp
	.cfi_def_cfa_offset 32
	movl	_oy, %ebx
	movl	32(%esp), %ecx
	movswl	4(%ecx), %eax
	movl	%eax, %edx
	sarl	$31, %edx
	idivl	_sx
	movl	%eax, %ebp
	movswl	(%ecx), %eax
	movl	%eax, %edx
	movl	%ebp, %edi
	sarl	$31, %edx
	idivl	_sy
	movl	%eax, %esi
	movl	%eax, _oy
	xorl	%eax, %eax
	movl	_ox, %edx
	movb	16(%ecx), %al
	subl	%edx, %edi
	movl	%eax, _pc_mouse+12
	movl	_maxx, %edx
	movl	_pc_mouse, %eax
	subl	%ebx, %esi
	addl	%edi, %eax
	movl	%ebp, _ox
	movsbl	17(%ecx), %ebx
	cmpl	%edx, %eax
	movl	%edx, %ecx
	jge	L2
	movl	_pc_mouse, %ecx
	addl	%edi, %ecx
L2:
	movl	_minx, %eax
	cmpl	%ecx, %eax
	jg	L3
	movl	_pc_mouse, %ecx
	movl	%edx, %eax
	addl	%edi, %ecx
	cmpl	%ecx, %edx
	jg	L22
L3:
	movl	%eax, _pc_mouse
	movl	_maxy, %edx
	movl	_pc_mouse+4, %eax
	movl	%edx, %ecx
	addl	%esi, %eax
	cmpl	%edx, %eax
	jge	L4
	movl	_pc_mouse+4, %ecx
	addl	%esi, %ecx
L4:
	movl	_miny, %eax
	cmpl	%ecx, %eax
	jg	L5
	movl	_pc_mouse+4, %ecx
	movl	%edx, %eax
	addl	%esi, %ecx
	cmpl	%ecx, %edx
	jg	L23
L5:
	movl	%eax, _pc_mouse+4
	movl	_maxz, %edx
	movl	_pc_mouse+8, %eax
	movl	%edx, %ecx
	addl	%ebx, %eax
	cmpl	%edx, %eax
	jge	L6
	movl	_pc_mouse+8, %ecx
	addl	%ebx, %ecx
L6:
	movl	_minz, %eax
	cmpl	%ecx, %eax
	jg	L7
	movl	_pc_mouse+8, %ecx
	movl	%edx, %eax
	addl	%ebx, %ecx
	cmpl	%ecx, %edx
	jg	L24
L7:
	movl	%eax, _pc_mouse+8
	movl	_emulat3, %eax
	testl	%eax, %eax
	je	L8
	movl	_pc_mouse+12, %eax
	andl	$3, %eax
	cmpl	$3, %eax
	je	L25
L8:
	movl	_mouse_func, %eax
	testl	%eax, %eax
	je	L1
	movl	_pc_mouse+12, %esi
	movl	_pc_mouse+8, %ebx
	movl	_pc_mouse+4, %ecx
	movl	_pc_mouse, %edx
	pushl	%esi
	.cfi_def_cfa_offset 36
	pushl	%ebx
	.cfi_def_cfa_offset 40
	pushl	%ecx
	.cfi_def_cfa_offset 44
	pushl	%edx
	.cfi_def_cfa_offset 48
	call	*%eax
	addl	$16, %esp
	.cfi_def_cfa_offset 32
L1:
	addl	$12, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
L24:
	.cfi_restore_state
	movl	_pc_mouse+8, %eax
	addl	%ebx, %eax
	jmp	L7
	.p2align 4,,7
L23:
	movl	_pc_mouse+4, %eax
	addl	%esi, %eax
	jmp	L5
	.p2align 4,,7
L22:
	movl	_pc_mouse, %eax
	addl	%edi, %eax
	jmp	L3
	.p2align 4,,7
L25:
	movl	$4, _pc_mouse+12
	jmp	L8
	.cfi_endproc
LFE3:
	.p2align 4,,15
_mouse_end:
LFB4:
	.cfi_startproc
	ret
	.cfi_endproc
LFE4:
	.p2align 4,,15
	.globl	_pc_remove_mouse
_pc_remove_mouse:
LFB5:
	.cfi_startproc
	movl	_mouse_callback, %eax
	testl	%eax, %eax
	jne	L33
	ret
	.p2align 4,,7
L33:
	subl	$24, %esp
	.cfi_def_cfa_offset 28
	pushl	$_pc_remove_mouse
	.cfi_def_cfa_offset 32
	call	_pc_clexit
	movl	_mouse_callback, %edx
/APP
# 90 "PC_HW/pc_mouse.c" 1
	
		movl	%edx, %ecx	
		shrl	$16, %ecx	
		movw	$0x0304, %ax	
		int	$0x31		
		movw	$0x000c, %ax	
		xorl	%ecx, %ecx	
		int	$0x33		
      
# 0 "" 2
/NO_APP
	movl	_mouse_wrap_end, %eax
	movl	$0, _mouse_callback
	subl	$16384, %eax
	movl	%eax, (%esp)
	call	_free
	addl	$28, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
LFE5:
	.p2align 4,,15
_pc_install_mouse.part.1:
LFB13:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$12, %esp
	.cfi_def_cfa_offset 32
/APP
# 118 "PC_HW/pc_mouse.c" 1
	
		xorl	%eax, %eax	
		int	$0x33		
		andl	%ebx, %eax	
		movl	%eax, %edx	
   
# 0 "" 2
/NO_APP
	testl	%edx, %edx
	jne	L35
L43:
	addl	$12, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	xorl	%ebx, %ebx
	movl	%ebx, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
L35:
	.cfi_restore_state
	movl	%edx, %ebx
	pushl	%edx
	.cfi_def_cfa_offset 36
	pushl	%edx
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_mouse_func
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%ecx
	.cfi_def_cfa_offset 44
	popl	%esi
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_mouse_callback
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%edi
	.cfi_def_cfa_offset 44
	popl	%ebp
	.cfi_def_cfa_offset 40
	pushl	$52
	.cfi_def_cfa_offset 44
	pushl	$_mouse_regs
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%eax
	.cfi_def_cfa_offset 44
	popl	%edx
	.cfi_def_cfa_offset 40
	pushl	$16
	.cfi_def_cfa_offset 44
	pushl	$_pc_mouse
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%ecx
	.cfi_def_cfa_offset 44
	popl	%esi
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_minx
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%edi
	.cfi_def_cfa_offset 44
	popl	%ebp
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_maxx
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%eax
	.cfi_def_cfa_offset 44
	popl	%edx
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_miny
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%ecx
	.cfi_def_cfa_offset 44
	popl	%esi
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_maxy
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%edi
	.cfi_def_cfa_offset 44
	popl	%ebp
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_minz
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%eax
	.cfi_def_cfa_offset 44
	popl	%edx
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_maxz
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%ecx
	.cfi_def_cfa_offset 44
	popl	%esi
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_sx
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%edi
	.cfi_def_cfa_offset 44
	popl	%ebp
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_sy
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%eax
	.cfi_def_cfa_offset 44
	popl	%edx
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_emulat3
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%ecx
	.cfi_def_cfa_offset 44
	popl	%esi
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_ox
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%edi
	.cfi_def_cfa_offset 44
	popl	%ebp
	.cfi_def_cfa_offset 40
	pushl	$4
	.cfi_def_cfa_offset 44
	pushl	$_oy
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_data
	popl	%eax
	.cfi_def_cfa_offset 44
	movl	$_mouse_end, %eax
	subl	$_mouse, %eax
	popl	%edx
	.cfi_def_cfa_offset 40
	pushl	%eax
	.cfi_def_cfa_offset 44
	pushl	$_mouse
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_code
	movl	$_mouse_wrap_end, %eax
	subl	$_mouse_wrap, %eax
	popl	%ecx
	.cfi_def_cfa_offset 44
	popl	%esi
	.cfi_def_cfa_offset 40
	pushl	%eax
	.cfi_def_cfa_offset 44
	pushl	$_mouse_wrap
	.cfi_def_cfa_offset 48
	call	__go32_dpmi_lock_code
	xorl	%eax, %eax
	movl	$16384, (%esp)
	movw	___djgpp_ds_alias, %ax
	movl	%eax, _mouse_wrap_end+4
	call	_pc_malloc
	addl	$16, %esp
	.cfi_def_cfa_offset 32
	movl	%eax, %ebp
	movl	%eax, _mouse_wrap_end
	testl	%eax, %eax
	je	L43
	movl	$_mouse_wrap, %esi
	movl	$_mouse_regs, %edi
/APP
# 154 "PC_HW/pc_mouse.c" 1
	
		pushl	%ds		
		pushl	%es		
		movw	$0x0303, %ax	
		pushl	%ds		
		pushl	%cs		
		popl	%ds		
		popl	%es		
		int	$0x31		
		popl	%es		
		popl	%ds		
		jc	0f		
		shll	$16, %ecx	
		movw	%dx, %cx	
		movl	%ecx, _mouse_callback	
	0:				
   
# 0 "" 2
/NO_APP
	movl	_mouse_callback, %eax
	testl	%eax, %eax
	je	L44
	addl	$16384, %ebp
	movw	%ax, _mouse_regs+20
	sarl	$16, %eax
	movl	%ebp, _mouse_wrap_end
	movw	%ax, _mouse_regs+34
	movw	$12, _mouse_regs+28
	movw	$255, _mouse_regs+24
	pushl	%eax
	.cfi_def_cfa_offset 36
	pushl	%eax
	.cfi_def_cfa_offset 40
	pushl	$_mouse_regs
	.cfi_def_cfa_offset 44
	pushl	$51
	.cfi_def_cfa_offset 48
	call	___dpmi_int
/APP
# 192 "PC_HW/pc_mouse.c" 1
	movw $0xb, %ax; int $0x33
# 0 "" 2
/NO_APP
	xorl	%eax, %eax
	cmpl	$2, %ebx
	setle	%al
	movl	$_pc_remove_mouse, (%esp)
	movl	$0, _oy
	movl	$0, _ox
	movl	%eax, _emulat3
	call	_pc_atexit
	addl	$16, %esp
	.cfi_def_cfa_offset 32
	addl	$12, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	%ebx, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
L44:
	.cfi_restore_state
	subl	$12, %esp
	.cfi_def_cfa_offset 44
	pushl	%ebp
	.cfi_def_cfa_offset 48
	call	_free
	addl	$16, %esp
	.cfi_def_cfa_offset 32
	jmp	L43
	.cfi_endproc
LFE13:
	.p2align 4,,15
	.globl	_pc_install_mouse
_pc_install_mouse:
LFB6:
	.cfi_startproc
	movl	_mouse_callback, %eax
	testl	%eax, %eax
	jne	L46
	jmp	_pc_install_mouse.part.1
	.p2align 4,,7
L46:
	xorl	%eax, %eax
	ret
	.cfi_endproc
LFE6:
	.p2align 4,,15
	.globl	_pc_install_mouse_handler
_pc_install_mouse_handler:
LFB7:
	.cfi_startproc
	subl	$12, %esp
	.cfi_def_cfa_offset 16
	movl	_mouse_callback, %eax
	testl	%eax, %eax
	je	L48
L50:
	movl	16(%esp), %edx
	movl	_mouse_func, %eax
	movl	%edx, _mouse_func
L47:
	addl	$12, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
L48:
	.cfi_restore_state
	call	_pc_install_mouse.part.1
	testl	%eax, %eax
	jne	L50
	jmp	L47
	.cfi_endproc
LFE7:
	.p2align 4,,15
	.globl	_pc_mouse_area
_pc_mouse_area:
LFB8:
	.cfi_startproc
	movl	4(%esp), %eax
	movl	%eax, _minx
	movl	12(%esp), %eax
	movl	%eax, _maxx
	movl	8(%esp), %eax
	movl	%eax, _miny
	movl	16(%esp), %eax
	movl	%eax, _maxy
	ret
	.cfi_endproc
LFE8:
	.p2align 4,,15
	.globl	_pc_mouse_speed
_pc_mouse_speed:
LFB9:
	.cfi_startproc
/APP
# 228 "PC_HW/pc_mouse.c" 1
	cli
# 0 "" 2
/NO_APP
	movl	4(%esp), %eax
	testl	%eax, %eax
	jg	L54
	movl	$1, %eax
L54:
	movl	%eax, _sx
	movl	8(%esp), %eax
	testl	%eax, %eax
	jg	L55
	movl	$1, %eax
L55:
	movl	%eax, _sy
/APP
# 233 "PC_HW/pc_mouse.c" 1
	sti
# 0 "" 2
/NO_APP
	ret
	.cfi_endproc
LFE9:
	.p2align 4,,15
	.globl	_pc_query_mouse
_pc_query_mouse:
LFB10:
	.cfi_startproc
	movl	4(%esp), %eax
	movl	_pc_mouse, %edx
	movl	%edx, (%eax)
	movl	8(%esp), %eax
	movl	_pc_mouse+4, %edx
	movl	%edx, (%eax)
	movl	12(%esp), %eax
	movl	_pc_mouse+8, %edx
	movl	%edx, (%eax)
	movl	_pc_mouse+12, %eax
	ret
	.cfi_endproc
LFE10:
	.p2align 4,,15
	.globl	_pc_warp_mouse
_pc_warp_mouse:
LFB11:
	.cfi_startproc
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	pushl	%edx
	.cfi_def_cfa_offset 16
/APP
# 250 "PC_HW/pc_mouse.c" 1
	movw $0xb, %ax; int $0x33
# 0 "" 2
/NO_APP
	movl	_maxx, %eax
	movl	$0, _oy
	movl	$0, _ox
	cmpl	16(%esp), %eax
	jle	L58
	movl	16(%esp), %eax
L58:
	movl	_minx, %edx
	cmpl	%edx, %eax
	jge	L59
	movl	%edx, %eax
L59:
	movl	%eax, _pc_mouse
	movl	_maxy, %eax
	cmpl	20(%esp), %eax
	jle	L60
	movl	20(%esp), %eax
L60:
	movl	_miny, %edx
	cmpl	%edx, %eax
	jge	L61
	movl	%edx, %eax
L61:
	movl	%eax, _pc_mouse+4
	movl	_mouse_func, %eax
	testl	%eax, %eax
	je	L57
	movl	_pc_mouse+12, %esi
	movl	_pc_mouse+8, %ebx
	movl	_pc_mouse+4, %ecx
	movl	_pc_mouse, %edx
	pushl	%esi
	.cfi_def_cfa_offset 20
	pushl	%ebx
	.cfi_def_cfa_offset 24
	pushl	%ecx
	.cfi_def_cfa_offset 28
	pushl	%edx
	.cfi_def_cfa_offset 32
	call	*%eax
	addl	$16, %esp
	.cfi_def_cfa_offset 16
L57:
	popl	%eax
	.cfi_def_cfa_offset 12
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
LFE11:
.lcomm _oy,4,4
.lcomm _ox,4,4
.lcomm _emulat3,4,4
	.section .data
	.p2align 2
_sy:
	.long	2
	.p2align 2
_sx:
	.long	2
	.p2align 2
_maxz:
	.long	255
.lcomm _minz,4,4
	.p2align 2
_maxy:
	.long	199
.lcomm _miny,4,4
	.p2align 2
_maxx:
	.long	319
.lcomm _minx,4,4
.lcomm _pc_mouse,16,16
.lcomm _mouse_regs,52,16
.lcomm _mouse_callback,4,4
.lcomm _mouse_func,4,4
	.ident	"GCC: (GNU) 7.2.0"
