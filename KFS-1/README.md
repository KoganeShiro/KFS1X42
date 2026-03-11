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
	• Add helpers like printf / printk in order to print information / debug easily.
	• Add colors support to your I/O interface.
	• Handle keyboard entries and print them.
	• Add scroll and cursor support to your I/O interface.
	• Handle different screens, and keyboard shortcuts to switch easily between them.


## Walkthrough
So, how do we actually do that ? First, we need to gain some knowledge about how a kernel works, and how to boot it. The best resource for that is the [os dev wiki](https://wiki.osdev.org/Expanded_Main_Page). It contains a lot of information about how to create an operating system from scratch, and how to boot it. You can also read [this](https://www.linuxjournal.com/content/what-does-it-take-make-kernel-0) or [this](https://theogill.medium.com/creating-a-kernel-from-scratch-1a1aa569780f) article to get a more high level overview of the process.

There is a tutorial [Bare Bones](https://wiki.osdev.org/Bare_Bones) that will actually give you the mandatory part of this subject done. It will show you how to create a simple kernel that can be booted with GRUB, and that can print some characters on the screen. You can then use this base to add the bonus parts.
To explain a little bit more about what the tutorial tell you to do, here are some insights:
- The bootloader (GRUB) will load your kernel in memory, and then call the entry point of your kernel (the main function using the ASM boot code). But before that, it will also pass some information to your kernel, like the memory map of the system, the command line arguments, etc. This information is passed in a structure called the multiboot info structure, and you can access it in your kernel code to get some useful information about the system. 

	You can learn more [here](https://medium.com/@hasancansert/understanding-linux-kernel-initialization-from-bootloader-to-userspace-d875431d27ec) or [here](https://0xax.gitbooks.io/linux-insides/content/Booting/linux-bootstrap-1.html) or [here](https://wiki.osdev.org/Bootloader).
	

- The ASM boot code is responsible for setting up the environment for your kernel to run. It will set up the stack, and then call the main function of your kernel. It is also responsible for handling the multiboot header, which is a special structure that GRUB looks for in order to know how to load your kernel. The multiboot header contains some information about the kernel, like its entry point, its size, etc. 

	You can find more information about the multiboot header [here](https://wiki.osdev.org/Multiboot) or [here](https://intermezzos.github.io/book/first-edition/multiboot-headers.html).


- The linker file is responsible for telling the linker how to link your kernel. It will specify the entry point of your kernel, and the memory layout of your kernel. It is also responsible for specifying the sections of your kernel, like the .text section (where your code will be), the .data section (where your initialized data will be), and the .bss section (where your uninitialized data will be). 

	You can find more information about linker files [here](https://wiki.osdev.org/Linker_Scripts) or [here](https://mcyoung.xyz/2021/06/01/linker-script/).


- VGA is a standard for displaying text and graphics on a computer screen. It is a very simple standard, and it is still used today in some cases (like in the booting process). The VGA standard defines a memory area where you can write characters and their colors, and then the VGA hardware will display them on the screen. It can also be called a frame buffer. 

	You can find more information about VGA [here](https://wiki.osdev.org/VGA_Hardware) or [here](https://www.quora.com/How-are-images-made-on-the-screen). Learn more about screen [here](https://youtu.be/8x1imoz5nrs?si=Bf_Dh7bdFw0-mXiQ) or [here](https://youtu.be/WNRQLessSms?si=XvdOJe8MYmm0hd0a).



#### In short,
You can view it like the booting process as a chain of events:
1. The computer is powered on, and the [BIOS/UEFI firmware](https://www.fortinet.com/resources/cyberglossary/what-is-firmware) is executed.
2. The BIOS/UEFI looks for a bootable device (like a hard drive, a USB stick, etc.) and loads the bootloader (GRUB) from that device into memory.
3. GRUB is executed, and it looks for the kernel binary on the disk. It loads the kernel into memory, and then calls the entry point of the kernel (the main function using the ASM boot code).
4. The ASM boot code is executed, and it sets up the environment for the kernel to run. It also handles the multiboot header, and then calls the main function of the kernel.
5. The kernel is executed, and it can now do whatever it wants, like print some characters on the screen, handle keyboard input, etc.

> The kernel just being an executable program that is loaded in memory and executed by the CPU, it can do whatever it wants, but it is usually responsible for managing the hardware resources of the computer, and providing some basic services to the user programs.

In order to write something on the screen, all we need to do is to write some characters and their colors in the VGA memory area to display our text. And that is the mandatory part of this subject.


#### So now, we are ready to start doing the bonuses.
We first start with creating a struct that will allow me to do the bonuses more easily. (The struct did change during the development but well here is the final version)
```c
/* Global kernel state */
typedef struct kfs {
	int current_screen;
	int total_screens;
	screen screens[MAX_SCREENS];
} kfs;

/* Scroll-back ring buffer (stores full VGA entries: char + color) */
typedef struct ring_buffer {
    uint16_t  buffer[VGA_WIDTH * HISTORY_MAX_LINES];
    uint16_t  live_snapshot[VGA_BUFFER_SIZE]; /* VGA snapshot taken when entering scroll mode */
    size_t    current_line;  /* next line to write into (ring head)     */
    size_t    total_lines;   /* how many lines stored (capped at max)   */
    size_t    scroll_offset; /* 0 = live view, >0 = scrolled back N lines */
} ring_buffer;

/* terminal screen related variables */
typedef struct screen {
    size_t          row;
    size_t          column;
    uint8_t         color;
    uint16_t*       buffer;        /* active write target: VGA_MEMORY or saved_buffer */
    uint16_t*       saved_buffer;  /* always points to this screen's private RAM copy */
    uint16_t        cursor_pos;
    int             screen_id;
    ring_buffer*    history;       /* scroll-back history for this screen */
} screen;
```

Since we were able to display characters on the screen, We started to write a very simple printf function (actually it was more like a putstr function) since it was already quite done by following the tutorial.

##### Handle keyboard input
We did the handle keyboards bonus right after so that we could write on the screen. We create some define variable to handle the different keys.
To explain it simply, a keyboard is a device that send us some information when we press a key. This information is called a scancode, and it is a number that represent the key that we pressed. We can then use this scancode to map which key was pressed, and do something with it (like print it on the screen obviously or shortcuts). These scancodes mapping can be found [here](https://www.millisecond.com/support/docs/current/html/language/scancodes.htm). 
So, we create an infinite loop in order to listen to the keyboard input, and when we receive a scancode, we try to map it to an ascii character, and then print it on the screen. Some keys are specials and need to be handled differently (like the shift key or the backspace, etc.) in order to print the correct character. For example, if we press the shift key and then the 'a' key, we want to print 'A' instead of 'a'. So we need to keep track of the state of these special keys in order to know how to map the scancode to the correct character.
The backspace handling was a little bit tricky, we need to move the cursor back, and then print a null character to erase the previous character which is quite easy to do. The tricky part is if we move the cursor to the middle of the line. When it is the case, we need to shift all the characters after the cursor one position to the left in order to fill the gap left by the erased character.

> To learn more about how keyboard works, you can check the [keyboard page](https://wiki.osdev.org/Keyboard) on the os dev wiki. You can also check the [PS/2 controller page](https://wiki.osdev.org/PS/2_Controller) since most of the keyboards are connected to the computer via a PS/2 port and watch this [video](https://youtu.be/h-NM1xSSzHQ?si=xYdWO6b3UC-RBg7B).


##### Implement printf
After we had the keyboard working, we finish implementing for real the printk function that we started to write before, adding va args support. We added some colors support to it, and some log levels (debug, info, warning, error). The man page wasn't really explicit about how to implement it, so we kind of made a mix of printf adding log levels with the minimal format specifier support (like %d, %i, %s, %c, %x, %p). 
> More information about the function [here](https://docs.kernel.org/core-api/printk-basics.html)


##### Implement cursor support
After it worked, we implemented the cursor support, we learned that the VGA hardware has a cursor that can be enabled or disabled, and that we can move it around the screen by writing to some specific ports. They are pre-defined cursor visual settings that we can use. 
Since we want to create a terminal like screen, so we can use left and right arrows to move the cursor on the current line but no up and down arrows support since we are not in a text editor. (even if it would be possible to do it).

> More information about the cursor [here](https://www.reddit.com/r/AskProgramming/comments/x7tt5u/how_does_a_cursor_work/) or [here](https://docs.kernel.org/admin-guide/vga-softcursor.html) or [here](https://wiki.osdev.org/Text_Mode_Cursor).


##### Handle different screens
Then the switch between screens shortcut (alt + 1, alt + 2, alt + 3). So the logic will be to have a sort of screen arrays containing a buffer of the content of the VGA memory (frame buffer). By changing the screen, we will render the content of the buffer of the screen we want to switch to on the VGA memory, and update the pointer of the active screen. We also added a little message to indicate on which screen we are when we switch.
For that, we used a circular buffer to store the content of the VGA memory for each screen, and we also save the cursor position for each screen in order to restore it when we switch back to that screen.

##### Implement scroll support
And finally, we implemented the scroll support.
The scroll support is a bit tricky, there are two type of scroll possible, the one when we are typing at the end of the screen and the other would be the scroll history where we need to save the current screen content in a buffer before scrolling, and then restore it when we want to scroll back and forth with the page up and page down keys (+ disabling the cursor so we won't be able to modify the content).


##### For fun, we also implemented some commands
- clear: to clear the screen
- help: to display some help message
- header: to display the header of the kernel (the "42" message)
- reboot: to reboot the computer (by calling the reboot function of the keyboard controller)
- shutdown: to shutdown the computer (by calling the shutdown function of the keyboard controller)


## Ressources
- [os dev wiki](https://wiki.osdev.org/Expanded_Main_Page)
- https://wiki.osdev.org/Tutorials#Kernel_Basics
- https://theogill.medium.com/creating-a-kernel-from-scratch-1a1aa569780f
- https://0xax.gitbooks.io/linux-insides/content/Booting/linux-bootstrap-1.html
- https://popovicu.com/posts/writing-an-operating-system-kernel-from-scratch/
- https://www.linuxjournal.com/content/what-does-it-take-make-kernel-0


