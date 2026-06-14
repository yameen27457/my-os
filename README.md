This is an lightweight and simple x86_64 debian based system made on an android phone.
This project is currently under construction prebuilds are available.
How to compile:


### 1. Linux (Debian/Ubuntu/Arch)
Install the standard tools:
```bash
# Debian/Ubuntu
sudo apt install -y nasm gcc-multilib qemu-system-x86 python3

# Arch Linux
sudo pacman -S base-devel nasm qemu-desktop python3
```
Compile and run:
```bash
make
qemu-system-i386 -kernel mykernel.bin -serial tcp:127.0.0.1:4444,server,nowait
python3 bridge.py
```

### 2. macOS (Intel or Apple Silicon M1/M2/M3)
macOS uses a different file format (Mach-O) than Linux (ELF), so on Mac **must** install an x86 cross-compiler via Homebrew:
```bash
brew install i686-elf-gcc nasm qemu python3
```
Compile and run using the cross-compiler tools:
```bash
make
qemu-system-i386 -kernel mykernel.bin -serial tcp:127.0.0.1:4444,server,nowait
python3 bridge.py
```

### 3. Windows 10 / 11
The easiest way to run this on Windows is using **WSL2 (Windows Subsystem for Linux)**:
1. Open PowerShell as Administrator and install Ubuntu: `wsl --install`
2. Restart the PC, open the new Ubuntu terminal app, and follow the **Linux** setup steps above.

*Alternative (Native Windows):*
Install **MSYS2**, **NASM for Windows**, and **QEMU for Windows**, then use the MinGW64 terminal to compile.
