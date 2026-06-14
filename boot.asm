bits 32

; Multiboot Header Definitions
MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM

section .text
global _start
global inb              
global outb             
extern kernel_main

_start:
    cli                 ; Disable interrupts safely

    ; 1. Reset segment registers to flat 32-bit data selector (GRUB default)
    mov ax, 0x10        ; 0x10 is standard GRUB data descriptor index
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 2. Initialize aligned memory stack workspace
    mov esp, stack_top  ; Safely link stack pointer
    
    ; 3. Execute main kernel function
    call kernel_main    
    
.halt_loop:
    hlt                 
    jmp .halt_loop      

; Read a single byte from a hardware port
inb:
    mov edx, [esp + 4]  
    xor eax, eax        
    in al, dx           
    ret                 

; Write a single byte to a hardware port
outb:
    mov edx, [esp + 4]  
    mov al, [esp + 8]   
    out dx, al          
    ret

; Allocate 8 Kilobytes of memory space for stack safety
section .bss
align 16
stack_bottom:
    resb 8192           ; Doubled size to prevent overflows
stack_top:

