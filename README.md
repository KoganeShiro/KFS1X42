# KFS42
Kernel From Scratch

## Instruction
### Emulation
The following part is not mandatory, you are free to use any virtual manager you want to, however, i suggest you to use [[Kernel Virtual Manager (KVM)|KVM]]. It’s a Kernel Virtual Manager, and have advanced execution and debugs functions.

### Language
A lot of language can be used instead of C, like C++, Rust, Go, etc. You could even code your entire kernel in ASM if you want !
Just be careful so that language has a good documentation on the kernel feature. Choose wisely !

###  Compilers
You can choose any compilers you want. I recommend to use gcc and nasm.
A Makefile must be turned in.

### Flags
In order to boot your kernel without any dependencies, you must compile your code with the following flags (Adapt the flags for your language, those ones are a C++ example):
	• -fno-builtin
	• -fno-exceptions
	• -fno-stack-protector
	• -fno-rtti
	• -nostdlib
	• -nodefaultlibs
Pay attention to -nodefaultlibs and -nostdlib. Your Kernel will be compiled on a host system, yes, but cannot be linked to any existing library on that host, otherwise it will not be executed.

### Linking
You cannot use an existing linker in order to link your kernel. As written above, your kernel will not boot. So, you must create a linker for your kernel. Be carefull, you **CAN use the ’ld’ binary** available on your host, but you **CANNOT use the .ld file** of your host.

### Architecture
The i386 (X86) architecture is mandatory

