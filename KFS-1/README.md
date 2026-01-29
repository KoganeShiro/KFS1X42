# KFS-1
Kernel From Scratch

## Goals
At the end of this subject, you will have:
	• A kernel you can boot via GRUB
	• An ASM bootable base
	• A basic kernel library, with basics functions and types
	• Some basic code to print some stuff on the screen
	• A basic "Hello world" kernel

## Mandatory
### Base
You must make a kernel, bootable with GRUB, who can write characters on screen.
In order to do that, you have to:
	• Install GRUB on an virtual image
	• Write an ASM boot code that **handles multiboot header**, and use [[Bootloader (GRUB ex)|GRUB]] to init and call main function of the [[Kernel (operating system)|kernel]] itself.
	• Write basic kernel code of the choosen language.
	• Compile it with correct flags, and link it to make it [[Booting|bootable]].
	• Once all of those steps above are done, you can write some helpers like kernel types or basic functions (strlen, strcmp, ...)
	• Your work **must not exceed 10 MB**.
	• Code the interface between your kernel and the screen.
	• Display "42" on the screen.
For the link part, you must create a linker file with the GNU [[Linker (ld)]]. Some docs [here](https://www.math.utah.edu/docs/info/ld_3.html#SEC4).

### Makefile
Your makefile must compile all your source files with the right flags and the right compiler. Keep in mind that your kernel will use at least two different languages (ASM and whatever-you-choose), so make your Makefile’s rules correctly.
After compilation, all the objects must be linked together in order to create the final [[Kernel (operating system)|Kernel binary]] (Cf. [[Linker (ld)|Linker part]]).


## Bonus
The following extra is intended to be useful:
	• Add scroll and cursor support to your I/O interface.
	• Add colors support to your I/O interface.
	• Add helpers like printf / printk in order to print information / debug easily.
	• Handle keyboard entries and print them.
	• Handle different screens, and keyboard shortcuts to switch easily between them.


## Ressources
- [os dev wiki](https://wiki.osdev.org/Expanded_Main_Page)
- https://wiki.osdev.org/Tutorials#Kernel_Basics
- https://theogill.medium.com/creating-a-kernel-from-scratch-1a1aa569780f
- https://0xax.gitbooks.io/linux-insides/content/Booting/linux-bootstrap-1.html
- https://popovicu.com/posts/writing-an-operating-system-kernel-from-scratch/
- https://www.linuxjournal.com/content/what-does-it-take-make-kernel-0


