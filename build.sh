rm -rf build/*
gcc -m32 -ffreestanding -c src/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -c src/boot.S -o build/boot.o
ld -m elf_i386 -T linker.ld -o build/kernel.elf build/boot.o build/kernel.o
grub-file --is-x86-multiboot build/kernel.elf
echo $?
cp build/kernel.elf iso/boot/kernel.elf
grub-mkrescue -o build/seros.iso iso

# qemu-system-i386 -cdrom build/seros.iso -boot d