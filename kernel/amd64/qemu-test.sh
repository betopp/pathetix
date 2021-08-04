#!/bin/sh
qemu-system-x86_64 -m 512 -kernel bin/pathetix.bin  -s -S   -serial stdio     --accel tcg,thread=single -smp 4 -cpu max  -d guest_errors,int  -initrd sys.tar


