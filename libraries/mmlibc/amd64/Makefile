#Makefile
#Builds libc for AMD64
#Bryan E. Topp <betopp@betopp.com> 2021

#Toolchain location
NASM=nasm
CC=clang
LD=clang
OBJCOPY=objcopy

#Source files location
SRCDIRS=src

TARGETFLAGS = -target x86_64-none-elf64 -march=athlon64 -m64 -mno-sse -mno-avx
TARGETFLAGS += -ffreestanding -nostdlib -nostdinc -D__is_libc 
TARGETFLAGS += -mabi=sysv

LDFLAGS = $(TARGETFLAGS)
LDFLAGS += -shared -static-libgcc -lgcc

CFLAGS = $(TARGETFLAGS)
CFLAGS += -c -g -Isrc
CFLAGS += -std=c99 -pedantic -Wall -Werror -Wextra
CFLAGS += -I../include -I../../libpx/include

NASMFLAGS = -f elf64 -g -F dwarf


#Build information provided to compiler
BUILDVERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
BUILDDATE := $(shell date -u +%FT%TZ)
BUILDUSER := $(shell whoami)@$(shell hostname)
CFLAGS += -DBUILDVERSION=\"$(BUILDVERSION)\"
CFLAGS += -DBUILDDATE=\"$(BUILDDATE)\"
CFLAGS += -DBUILDUSER=\"$(BUILDUSER)\"

#All objects to build...
CFILES=$(shell find $(SRCDIRS) -follow -name *.c)
SFILES=$(shell find $(SRCDIRS) -follow -name *.asm)

CFILES_OBJECTS=$(patsubst src/%.c, $(OBJDIR)/c/%.o, $(CFILES))
SFILES_OBJECTS=$(patsubst src/%.asm, $(OBJDIR)/s/%.o, $(SFILES))

LIBC_FILE=$(LIBDIR)/libc.so
LIBC_A_FILE=$(LIBDIR)/libc.a

#Output location
LIBDIR=lib
OBJDIR=obj

#Overall targets
all: $(LIBC_A_FILE)

#Static libc
$(LIBC_A_FILE): $(SFILES_OBJECTS) $(CFILES_OBJECTS)
	mkdir -p $(@D)
	$(AR) rcs $@ $^

#C objects are compiled with GCC
$(OBJDIR)/c/%.o : src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@ -MMD

#S objects are assembled with NASM
$(OBJDIR)/s/%.o : src/%.asm
	mkdir -p $(@D)
	$(NASM) $(NASMFLAGS) $< -o $@ -MD $@.d

clean:
	rm -Rf $(LIBDIR)
	rm -Rf $(OBJDIR)

-include $(patsubst %.o, %.o.d, $(SFILES_OBJECTS))
-include $(patsubst %.o, %.d, $(CFILES_OBJECTS))


