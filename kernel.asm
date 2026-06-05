bits 32

; Multiboot Header
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
global inb              ; Export low-level input port function to C
extern kernel_main

_start:
    cli
    call kernel_main
    hlt

; Read a single byte from a hardware port
inb:
    mov edx, [esp + 4]  ; Get the port number passed from C
    in al, dx           ; Read the byte from the hardware port into AL
    ret                 ; Return to C with the byte value in AL
