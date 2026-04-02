# == Docker =====================================================
DOCKER_IMAGE = kfs1-builder
DOCKERFILE   = src/dockerfile

# == Toolchain (used inside the container) ====================
CC      = i686-elf-gcc
AS      = i686-elf-as
LD      = i686-elf-gcc

# == Flags =====================================================
CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(SRCDIR) \
		  -fno-builtin -fno-exceptions -fno-stack-protector
LDFLAGS = -ffreestanding -O2 -nostdlib -nodefaultlibs -lgcc

# == Paths =====================================================
ISODIR  = isodir
KERNEL  = $(ISODIR)/boot/kernel.bin
ISO     = kfs2.iso
OBJDIR  = obj
SRCDIR  = src

# == Sources ==================================================
ASM_SRC = boot/boot.s
C_SRC   = kernel.c \
		  drivers/vga.c \
		  drivers/vga_write.c \
		  drivers/handle_cursor.c \
		  drivers/handle_keyboard.c \
		  drivers/keymap.c \
		  drivers/port_io.c \
		  tty/handle_screen.c \
		  tty/handle_screen_shortcut.c \
		  tty/handle_scroll.c \
		  printk/printk.c \
		  printk/convert.c \
		  lib/string.c \
		  lib/print_header.c \
		  commands/clear.c \
		  commands/reboot.c \
		  commands/shutdown.c \
		  commands/command_parser.c \
		  commands/handle_commands.c \
		  commands/print_gdt.c \
		  commands/halt.c \
		  gdt/gdt.c 

# == Objects ==================================================
OBJ     = $(OBJDIR)/boot.o $(addprefix $(OBJDIR)/, $(C_SRC:.c=.o))

# ══════════════════════════════════════════════════════════════
# HOST targets — run these from your machine
# ══════════════════════════════════════════════════════
docker-build:
	docker build -t $(DOCKER_IMAGE) -f $(DOCKERFILE) .

# Build the ISO inside the container
docker-iso: docker-build
	docker run --rm -v $(PWD):/workspace $(DOCKER_IMAGE) make iso

# Build + run QEMU on the host
# -enable-kvm : use hardware virtualisation (KVM) so the host CPU natively
#               executes guest code.  The "pause" spin-wait in the keyboard
#               loop triggers Pause-Loop Exiting (PLE), which lets the host
#               deschedule the vCPU — dropping host CPU usage from ~100 % to ~0 %.
# -cpu host   : expose the real host CPU model to the guest (requires KVM).
run: docker-iso
	qemu-system-i386 -enable-kvm -cpu host -cdrom $(ISO) 
#-display gtk,zoom-to-fit=on

# Remove Docker image and containers
docker-clean:
	docker rmi -f $(DOCKER_IMAGE) 2>/dev/null || true

# ══════════════════════════════════════════════════════════════
# CONTAINER targets — these run inside Docker (cross-compiler)
# ══════════════════════════════════════════════════════════════

help:
	@echo "Available targets:"
	@echo "  run            - Build + run QEMU on the host"
	@echo "  re		        - Clean and rebuild everything"
	@echo "  clean		    - Clean everything except Docker image"
	@echo "  fclean		    - Clean everything even remove Docker image"
	@echo "  docker-build   - Build the Docker image (only needed once)"
	@echo "  docker-clean   - Remove Docker image"
	@echo "  help           - Show this help message"

re: clean run

iso: $(ISO)

$(OBJDIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJDIR)/%.o: boot/%.s
	mkdir -p $(OBJDIR)
	$(AS) $< -o $@

$(KERNEL): $(OBJ)
	mkdir -p $(ISODIR)/boot
	$(LD) -T boot/linker.ld -o $(KERNEL) $(OBJ) $(LDFLAGS)

$(ISO): $(KERNEL)
	mkdir -p $(ISODIR)/boot/grub
	cp boot/grub.cfg $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISODIR)

clean:
	rm -rf $(OBJDIR)
	rm -rf $(ISODIR)
	rm -f $(ISO)

fclean: clean docker-clean

.PHONY: all iso clean run docker-build docker-iso help re fclean
