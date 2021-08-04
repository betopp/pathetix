#Makefile
#Builds kernel for AMD64
#Bryan E. Topp <betopp@betopp.com> 2021

#Toolchain location
NASM=nasm
CC=clang
OBJCOPY=objcopy

#Source files location
SRCDIRS=src
LDSCRIPT=src/kernel.ld

TARGETFLAGS = -target x86_64-none-elf64 -march=athlon64 -m64 -mno-sse -mno-avx
TARGETFLAGS += -ffreestanding -nostdlib -nostdinc 
TARGETFLAGS += -mabi=sysv -mno-red-zone -mcmodel=kernel -O0

LINKFLAGS = $(TARGETFLAGS)
LINKFLAGS += -static -T$(LDSCRIPT)

COMPILEFLAGS = $(TARGETFLAGS)
COMPILEFLAGS += -c -g -Isrc -DKERNEL -MMD
COMPILEFLAGS += -std=gnu99 -pedantic -Wall -Werror -Wextra -Wno-language-extension-token

COMPILEFLAGS += -I../shared/hal/
COMPILEFLAGS += -I../../libraries/mmlibc/include
COMPILEFLAGS += -I../../libraries/libpx/include

NASMFLAGS = -f elf64 -g -F dwarf

#Build information provided to compiler
BUILDVERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
BUILDDATE := $(shell date -u +%FT%TZ)
BUILDUSER := $(shell whoami)@$(shell hostname)
COMPILEFLAGS += -DBUILDVERSION=\"$(BUILDVERSION)\"
COMPILEFLAGS += -DBUILDDATE=\"$(BUILDDATE)\"
COMPILEFLAGS += -DBUILDUSER=\"$(BUILDUSER)\"

#All objects to build...
CFILES=$(shell find $(SRCDIRS) -follow -name *.c)
SFILES=$(shell find $(SRCDIRS) -follow -name *.asm)

CFILES_OBJECTS=$(patsubst src/%.c, $(OBJDIR)/c/%.o, $(CFILES))
SFILES_OBJECTS=$(patsubst src/%.asm, $(OBJDIR)/s/%.o, $(SFILES))

#Output location
OBJDIR=obj
BINDIR=bin

BINFILE=$(BINDIR)/pathetix.bin
ELFFILE=$(BINDIR)/pathetix.elf

#Final step is stripping the ELF to produce a kernel BIN
$(BINFILE): $(ELFFILE)
	mkdir -p $(@D)
	$(OBJCOPY) $< -O binary $@

#ELF kernel is linked of all objects. Note that assembled objects are first, for boot headers/etc.
$(ELFFILE) : $(SFILES_OBJECTS) $(CFILES_OBJECTS)
	mkdir -p $(@D)
	$(CC) -o $@ $^ $(LIBFILES) $(LINKFLAGS) 

#C objects are compiled with GCC
$(OBJDIR)/c/%.o : src/%.c
	mkdir -p $(@D)
	$(CC) -c $(COMPILEFLAGS) $< -o $@

#S objects are assembled with NASM
$(OBJDIR)/s/%.o : src/%.asm
	mkdir -p $(@D)
	$(NASM) $(NASMFLAGS) $< -o $@

clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)

#Include previously generated dependencies from C files, to catch header changes ( -MMD option )
-include $(patsubst %.o, %.d, $(CFILES_OBJECTS))
