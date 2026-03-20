    .org    0

    .globl  _pc_boot_drive
    .lcomm  _pc_boot_drive, 4

    .globl  _pc_boot_part_base
    .lcomm  _pc_boot_part_base, 4

    .section .text.startup
    .code16
    .globl  _s1start
_s1start:
    xor     %ax, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %ss

    mov     $0x7C00, %sp
    mov     %sp, %bp

.clear_bss:
    pushl   %edx
    pushl   %ecx

    mov     $0, %al
    mov     $__bss_start, %di
    mov     $__bss_end, %ecx
    sub     $__bss_start, %ecx
    rep     stosb

    popl    _pc_boot_part_base
    popl    _pc_boot_drive

.activate_a20:
    mov     $0x2401, %ax
    int     $0x15
    jnc     _s1start.1

    in      $0x92, %al
    or      $2, %al
    out     %al, $0x92

    jmp     _s1start.1

    mov     $str_a20error, %si
    call    print_string
    jmp     .halt16

_s1start.1:
    call    switch_r16_to_p32
    .code32

.jump:
    xor     %ebp, %ebp
    call    s1main

.halt:
    hlt
    jmp     .halt

    .code16
.halt16:
    hlt
    jmp     .halt16

print_string:
    mov     $0x0E, %ah
    xor     %bx, %bx

print_string.1:
    lodsb
    or      %al, %al
    jz      print_string.2

    int     $0x10
    jmp     print_string.1

print_string.2:
    ret

str_a20error:
    .asciz "Error activating A20 line.\r\n"
