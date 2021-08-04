# pathetix
Pathetic operating system

This is a toy operating system that I wrote in my spare time. It works somewhat like Unix, but is single-user right now.

It boots and runs the Open Korn Shell on an AMD64 PC. The kernel is fully reentrant and 64-bit only. A PS/2 keyboard and EGA text screen are used as a console.

The root filesystem is in RAM only. A multiboot bootloader should load a TAR file as a module for the kernel to find. The kernel unpacks the TAR into the RAM FS before trying to exec init.

All code is original and available under the MIT license, except the Korn Shell port.


