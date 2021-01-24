qemu-system-aarch64 -machine virt,virtualization=on,gic-version=3 -cpu max -smp 4 -m 1G -nographic -nodefaults -serial stdio -d unimp -kernel Kopernik.bin
