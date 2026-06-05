This is an lightweight and simple x86_64 debian based system made on an android phone.
This project is currently under construction prebuilds are available.
How to compile:


### 1. Linux (Debian/Ubuntu/Arch)
Install the standard tools:
```bash
# Debian/Ubuntu
sudo apt install -y nasm gcc-multilib qemu-system-x86

# Arch Linux
sudo pacman -S base-devel nasm qemu-desktop
```
Compile and run:
```bash
nasm -f elf32 kernel.asm -o k_asm.o
gcc -m32 -ffreestanding -c kernel.c -o k_c.o -fno-pic
ld -m elf_i386 -T linker.ld k_asm.o k_c.o -o kernel.bin
qemu-system-i386 -kernel kernel.bin
```

### 2. macOS (Intel or Apple Silicon M1/M2/M3)
macOS uses a different file format (Mach-O) than Linux (ELF), so on Mac **must** install an x86 cross-compiler via Homebrew:
```bash
brew install i686-elf-gcc nasm qemu
```
Compile and run using the cross-compiler tools:
```bash
nasm -f elf32 kernel.asm -o k_asm.o
i686-elf-gcc -ffreestanding -c kernel.c -o k_c.o -fno-pic
i686-elf-ld -T linker.ld k_asm.o k_c.o -o kernel.bin
qemu-system-i386 -kernel kernel.bin
```

### 3. Windows 10 / 11
The easiest way to run this on Windows is using **WSL2 (Windows Subsystem for Linux)**:
1. Open PowerShell as Administrator and install Ubuntu: `wsl --install`
2. Restart the PC, open the new Ubuntu terminal app, and follow the **Linux** setup steps above.

*Alternative (Native Windows):*
Install **MSYS2**, **NASM for Windows**, and **QEMU for Windows**, then use the MinGW64 terminal to compile.
