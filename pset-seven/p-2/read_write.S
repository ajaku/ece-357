.section .bss
	my_buffer: .space 4096

.section .text
	.globl _start

_start:
	mov	$0, %rax		     
	mov	$0, %rdi	         
	lea [my_buffer], %rsi  
	mov	$4096, %rdx
	syscall

	mov	$1, %rax			
	mov	$1, %rdi			
	lea [my_buffer], %rsi 
	mov	$4096, %rdx		
	syscall

	mov	$60, %rax		
	mov	$0, %rdi
	syscall
