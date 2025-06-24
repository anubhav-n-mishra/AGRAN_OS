# ARGON_OS

A modular, robust, and efficient operating system built from scratch for x86 architecture. ARGON_OS features a custom bootloader, kernel, process scheduler, in-memory file system, and an interactive shell. Built with C and Assembly, tested on QEMU, and designed for learning and innovation in OS development.

---

## 🚀 Features
- Custom x86 bootloader and kernel initialization
- Robust process scheduling and memory management
- Safe, modular file system and directory management
- Interactive shell with command history and error handling
- Defensive programming for maximum system stability

---

## 🛠️ Technical Stack
- **Languages:** Assembly (NASM), C, Make
- **Tools:** QEMU, GCC cross-compiler (i686-elf), GNU Make, GDB
- **Hardware:** x86 architecture, VGA text mode, keyboard controller

---

## 📦 Project Structure
- `boot/` – Bootloader (boot.asm, GDT, protected mode switch)
- `kernel/` – Kernel core, screen, keyboard, memory management
- `shell/` – Shell interface, commands, parser
- `fs/` – File system implementation
- `process/` – Process management and scheduling
- `team_docs/` – Documentation and guides

---

## 🏗️ Setup & Build Instructions

### 1. Prerequisites
- Ubuntu/Debian: `sudo apt-get install build-essential nasm qemu-system-x86 gdb make git`
- Arch Linux: `sudo pacman -S base-devel nasm qemu gdb make git`

### 2. Cross-Compiler Setup
```bash
# Create directory for cross-compiler
mkdir -p ~/opt/cross
cd ~/opt/cross
# Download and build binutils and GCC (see team_docs/setup_and_running_guide.md for full steps)
```
Add to your `~/.bashrc` or `~/.zshrc`:
```bash
export PATH="$HOME/opt/cross/bin:$PATH"
```

### 3. Build & Run
```bash
# Compile bootloader
cd boot
nasm -f bin boot.asm -o boot.bin

# Compile kernel
cd ../kernel
i686-elf-gcc -c kernel.c -o kernel.o -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -c screen.c -o screen.o -ffreestanding -O2 -Wall -Wextra
i686-elf-ld -T linker.ld -o kernel.bin kernel.o screen.o

# Compile shell
cd ../shell
i686-elf-gcc -c shell.c -o shell.o -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -c commands.c -o commands.o -ffreestanding -O2 -Wall -Wextra
i686-elf-ld -T shell.ld -o shell.bin shell.o commands.o

# Create disk image
cd ..
dd if=/dev/zero of=os.img bs=512 count=2880
dd if=boot/boot.bin of=os.img conv=notrunc bs=512 count=1
dd if=kernel/kernel.bin of=os.img conv=notrunc bs=512 seek=1

# Run in QEMU
qemu-system-i386 -drive format=raw,file=os.img,if=floppy -m 32M
```

---

## 🕹️ Usage
- Use the shell to run commands: `help`, `clear`, `ls`, `create`, `write`, `read`, etc.
- See `team_docs/setup_and_running_guide.md` for advanced usage, debugging, and troubleshooting.

---

## 📅 Project Chronology & Execution Flow
- **Bootloader:** BIOS → boot.bin (sets up environment, loads kernel)
- **Kernel:** kernel.bin (hardware init, memory, loads shell)
- **Shell:** shell.bin (command prompt, user input, command execution)

See `team_docs/project_chronology.md` for full details.

---

## 👥 Team & Roles
- **Anubhav Mishra (Team Lead):** Architecture, integration, project management,Bootloader , toolchain setup.
- **Anish Negi:** Shell interface, command parsing, user interaction,kernel core, memory management
- **Gaurab Rana:** In-memory file system, file ops, defensive programming
- **Rachit Rawat:** Process creation, scheduling, IPC

---

## 🧩 Challenges & Solutions
- Screen buffer/cursor bugs → Robust scroll and cursor logic
- Command parsing/input issues → Enhanced shell parsing, input checks
- File system safety → Defensive printing, null-termination
- Process management → Round-robin scheduling, safe creation/termination
- Build system issues → Cross-compilation fixes, reliable builds
- Documentation → Up-to-date guides and logs

---

## 🌱 Future Improvements
- Persistent (disk-backed) file system
- Virtual memory and paging
- Advanced process scheduling
- Networking and GUI support
- More device drivers and system utilities

---

## 📚 References
- OSDev Wiki, Intel x86 manuals, GNU docs, QEMU, GCC, Make, OS textbooks, C/assembly guides

---

## 📄 License
MIT License (or specify your own)

---

## 🙏 Acknowledgments
Thanks to all contributors, mentors, and the open-source community!
