file build/kernel.elf
grub-file --is-x86-multiboot build/kernel.elf; echo $?
xorriso -indev build/seros.iso -ls /boot

cat iso/boot/grub/grub.cfg
ls -l iso/boot/grub/grub.cfg
qemu-system-i386 \
    -drive file=build/seros.iso,media=cdrom,readonly=on \
    -boot order=d

xorriso -indev build/seros.iso -report_el_torito plain

grub-file --is-x86-multiboot build/kernel.elf
echo "kernel check: $?"

ls -lh build/seros.iso

cat iso/boot/grub/grub.cfg
file build/seros.iso
xorriso -indev build/seros.iso -report_el_torito plain
readelf -S build/kernel.elf