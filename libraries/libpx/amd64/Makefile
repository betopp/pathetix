#Makefile
#Builds kernel-call library on AMD64
#Bryan E. Topp <betopp@betopp.com> 2021

#Toolchain location
NASM=nasm
CC=clang
OBJCOPY=objcopy

#Source files location
SRCDIRS=src

COMPILEFLAGS = -target x86_64-none-elf64 -march=athlon64 -m64 -mno-sse -mno-avx
COMPILEFLAGS += -ffreestanding -nostdlib -nostdinc
COMPILEFLAGS += -mabi=sysv
COMPILEFLAGS += -c -g -O2
COMPILEFLAGS += -std=gnu99 -pedantic -Wall -Werror -Wextra -Wno-language-extension-token -I../include -I../../../libraries/mmlibc/include

NASMFLAGS = -f elf64 -g -F dwarf

#All objects to build...
SFILES=$(shell find $(SRCDIRS) -name *.asm)
SFILES_OBJECTS=$(patsubst %.asm, $(OBJDIR)/s/%.o, $(SFILES))

CFILES=$(shell find $(SRCDIRS) -name *.c)
CFILES_OBJECTS=$(patsubst %.c, $(OBJDIR)/c/%.o, $(CFILES))

LIB_A_FILE=$(LIBDIR)/libpx.a

#Output location
LIBDIR=lib
OBJDIR=obj

#Overall targets
all: $(LIB_A_FILE)

#Static library
$(LIB_A_FILE): $(SFILES_OBJECTS) $(CFILES_OBJECTS)
	mkdir -p $(@D)
	$(AR) rcs $@ $^

#S objects are assembled with NASM
$(OBJDIR)/s/%.o : %.asm
	mkdir -p $(@D)
	$(NASM) $(NASMFLAGS) $< -o $@ -MD $@.d

#C objects are compiled with GCC
$(OBJDIR)/c/%.o : %.c
	mkdir -p $(@D)
	$(CC) $(COMPILEFLAGS) $< -o $@ -MMD

clean:
	rm -f $(LIB_A_FILE)
	rm -Rf $(OBJDIR) $(LIBDIR)

-include $(patsubst %.o, %.o.d, $(SFILES_OBJECTS))
-include $(patsubst %.o, %.d, $(CFILES_OBJECTS))

