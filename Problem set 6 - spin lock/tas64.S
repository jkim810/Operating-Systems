	.text
.globl tas
	.type	tas,@function
tas:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$1, %rax
#APP
	lock;xchgb	%al,(%rdi)
#NO_APP
	movsbq	%al,%rax
	pop	%rbp
	ret
.Lfe1:
	.size	tas,.Lfe1-tas
