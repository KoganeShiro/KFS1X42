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
But first, what is the GDT and what does it do? 
GDT or Global Descriptor Table is the implementation of the segmentation in protected mode. It is a table that contains entries that describe the memory segments of the system. Each entry contains information about the base address, limit, and access rights of a segment. The GDT is used by the CPU to translate logical addresses to physical addresses and to enforce memory protection.
Not to be confused with paging, which is another memory management technique that allows the system to use more memory than the physical RAM by using a portion of the hard drive as virtual memory.
The main difference between segmentation and paging is that segmentation divides the memory into variable-sized segments, while paging divides the memory into fixed-size pages. Segmentation is more flexible but can lead to fragmentation, while paging is more efficient but can lead to overhead due to page tables.
From my understanding, it seems like like this way of segmentation is not really used in modern operating systems (outdated), but it is still a good exercise to understand how memory management works in protected mode. In recent operating systems we use flat memory model, where all segments have the same base address and limit, and the access rights are set to allow access to all segments. This way we can use paging to manage the virtual memory and avoid fragmentation issues.

What are asked in the subject is to create a GDT with 6 entries: Kernel code, Kernel data, Kernel stack, User code, User data and User stack. Each entry will have its own base address, limit and access rights. The GDT must be set at address 0x00000800 and declared to the BIOS.

The "modern" way of implementing a GDT would be to follow this:
| Segment     | Base | Limit | Privilege | Type       | Why                 |
| ----------- | ---- | ----- | --------- | ---------- | ------------------- |
| Null        | —    | —     | —         | —          | Required by CPU     |
| Kernel Code | 0    | 4GB   | 0         | Executable | Kernel instructions |
| Kernel Data | 0    | 4GB   | 0         | Read/Write | Kernel data + stack |
| User Code   | 0    | 4GB   | 3         | Executable | User programs       |
| User Data   | 0    | 4GB   | 3         | Read/Write | User data + stack   |

But what the subject is asking is more something like this:
Index | Segment        | Base | Limit | Privilege | Type        | Reason
------+----------------+------+-------+-----------+-------------+------------------------------
0     | Null           | 0    | 0     | —         | —           | Required by CPU (must exist)
1     | Kernel Code    | 0    | 4GB   | 0         | Executable  | Kernel instructions
2     | Kernel Data    | 0    | 4GB   | 0         | Read/Write  | Kernel data
3     | Kernel Stack   | 0    | 4GB   | 0         | Read/Write  | (Same as data, for SS)
4     | User Code      | 0    | 4GB   | 3         | Executable  | User programs
5     | User Data      | 0    | 4GB   | 3         | Read/Write  | User data
6     | User Stack     | 0    | 4GB   | 3         | Read/Write  | (Same as data, for SS)


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


