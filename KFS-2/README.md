# KFS-2
Kernel From Scratch

## Goals
You will have to **create**, **fill** and **link** a Global Descriptor Table into your Kernel.
Actually, you will have to understand how "memory" really works in a system, how the **Stack** and **RAM** works, how to use it, how to fill it and how to link it with the **BIOS**.
Yeah, the BIOS. Thanks to **GRUB** (Bootloader), it will help you a lot !


## Mandatory
### Base
Let’s sum this up:
	• You must create a [[GDT (Global Descriptor Table)]].
	• Your GDT must contain:
		◦ Kernel Code
		◦ Kernel Data
		◦ Kernel stack
		◦ User code
		◦ User data
		◦ User stack
		◦ Your work should not exceed 10 MB.
	• You must declare your GDT to the BIOS.
	• The GDT must be set at address 0x00000800.
When this is done, you have to code a tool to print the kernel stack, in a human-friendly
way. (Tip: If you haven’t made a printk yet, now is a good time !)


### Makefile
Your makefile must compile all your source files with the right flags and the right compiler. Keep in mind that your kernel will use at least two different languages (ASM and whatever-you-choose), so make your Makefile’s rules correctly.
After compilation, all the objects must be linked together in order to create the final [[Kernel (operating system)|Kernel binary]] (Cf. [[Linker (ld)|Linker part]]).


## Bonus
Assuming your keyboard work correctly in your Kernel, and you able to catch an entry, let’s code a Shell !
Not a POSIX Shell, just a minimalistic shell with a few commands, for debugging purposes.
For example, you could implement the print-kernel-stack-thing in this shell, and some other things like reboot, halt and such.


## Walkthrough
From what we did in KFS-1, we will need to implement the GDT table.
But first, what is the GDT and what does it do? ...


For the bonuses, since we already start by implemented some basics commands, we can add some more by implementing the commands asked in the bonus part:
- print-kernel-stack-thing

- halt

## Ressources
- [os dev wiki](https://wiki.osdev.org/Expanded_Main_Page)
- https://wiki.osdev.org/Tutorials#Kernel_Basics
- https://theogill.medium.com/creating-a-kernel-from-scratch-1a1aa569780f
- https://0xax.gitbooks.io/linux-insides/content/Booting/linux-bootstrap-1.html
- https://popovicu.com/posts/writing-an-operating-system-kernel-from-scratch/
- https://www.linuxjournal.com/content/what-does-it-take-make-kernel-0


