    .org    0

    .section .text.startup
    .code16
    .globl  _s1start
_s1start:
    xor     %ax, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %ss
    mov     $0x7C00, %sp

    mov     $message, %si

.print:
    lodsb
    test    %al, %al
    jz      .halt

    mov     $0x0E, %ah
    xor     %bx, %bx
    int     $0x10
    jmp     .print

.halt:
    hlt
    jmp     .halt

message:
    .asciz  "Hello from stage1\r\n"
