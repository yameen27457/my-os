# Target file names
KERNEL = mykernel.bin

# Compiler setup
CC = gcc
# Change this specific line in your Makefile to match this exactly:
CFLAGS = -m32 -ffreestanding -O0 -Wall -Wextra -fno-pie -fno-stack-protector -fno-omit-frame-pointer
AS = nasm
ASFLAGS = -f elf32
LD = ld
LDFLAGS = -m elf_i386 -T linker.ld

# The main rule that builds your OS binary
all: $(KERNEL)

$(KERNEL): boot.o kernel.o serial.o
	$(LD) $(LDFLAGS) boot.o kernel.o serial.o -o $(KERNEL)
	
boot.o: boot.asm
	$(AS) $(ASFLAGS) boot.asm -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

serial.o: serial.c
	$(CC) $(CFLAGS) -c serial.c -o serial.o

clean:
	rm -rf *.o $(KERNEL)

