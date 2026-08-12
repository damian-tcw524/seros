CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -Wall -Wextra
ASFLAGS = -m32
LDFLAGS = -m elf_i386 -T linker.ld

BUILD = build

C_SOURCES = \
	src/kernel.c \
	src/kernel/idt.c \
	src/kernel/kstdlib.c

C_OBJECTS = \
	$(BUILD)/kernel.o \
	$(BUILD)/kernel/idt.o \
	$(BUILD)/kernel/kstdlib.o

BOOT_OBJECT = $(BUILD)/boot.o
ISR_OBJECT = $(BUILD)/kernel/isr.o

KERNEL = $(BUILD)/kernel.elf
ISO = $(BUILD)/seros.iso

.PHONY: all clean iso run

all: $(ISO)


# --------------------------------------------------
# Kernel ELF
# --------------------------------------------------

$(KERNEL): $(BOOT_OBJECT) $(ISR_OBJECT) $(C_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

	@grub-file --is-x86-multiboot $@
	@echo "Multiboot check: OK"


# --------------------------------------------------
# Boot assembly
# --------------------------------------------------

$(BOOT_OBJECT): src/boot.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# --------------------------------------------------
# ISR assembly
# --------------------------------------------------

$(ISR_OBJECT): src/kernel/isr.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# --------------------------------------------------
# C files
# --------------------------------------------------

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# --------------------------------------------------
# ISO
# --------------------------------------------------

$(ISO): $(KERNEL)
	cp $(KERNEL) iso/boot/kernel.elf
	grub-mkrescue -o $@ iso


# --------------------------------------------------
# Clean
# --------------------------------------------------

clean:
	rm -rf $(BUILD)/*


# --------------------------------------------------
# Run
# --------------------------------------------------

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -boot d