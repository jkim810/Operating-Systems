    
    .text

.global _start

_start:
    movq    $1,   %rax    #system call #1 (write)
    movq    $1,   %rdi    #output to stdout(1)
    movq    $msg, %rsi    #message to output
    movq    $14,  %rdx    #length of msg rdx
    syscall
    
    .data
    
msg: .ascii "hello, world!\n"
