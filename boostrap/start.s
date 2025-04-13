.section .text
.global _start

_start:
    andq $~15, %rsp
    
    movq %rsp, %rbp
    movq (%rbp), %rdi       
    leaq 8(%rbp), %rsi      
    leaq 8(%rsi,%rdi,8), %rdx 
    
    call _init
    
    call main
    
    movq %rax, %rdi
    call exit

    hlt

.section .init
.global _init
_init:
    ret

.section .fini
.global _fini
_fini:
    ret