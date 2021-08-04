#amd64/Makefile.program.inc
#Makefile included by programs when building on AMD64
#Bryan E. Topp <betopp@betopp.com> 2021

#Location of this makefile
SELFDIR = $(dir $(lastword $(MAKEFILE_LIST)))
PX_ROOT = $(SELFDIR)/../../

CRTOBJ = $(PX_ROOT)/libraries/pxcrt/amd64/obj

#Toolchain location
CC=clang




CFLAGS += -target x86_64-none-elf64 -march=athlon64 -m64 -mno-sse -mno-avx
CFLAGS += -ffreestanding -nostdlib -nostdinc
CFLAGS += -mabi=sysv
CFLAGS += -c -g
CFLAGS += -std=c99 -pedantic -Wall -Werror -Wextra
CFLAGS += -I$(PX_ROOT)/libraries/mmlibc/include
CFLAGS += -I$(PX_ROOT)/libraries/pcmd/include

CFLAGS += $(PROGRAM_CFLAGS)

LINKFLAGS = $(PROGRAM_LINKFLAGS)

LINKFLAGS += -pedantic -Wall -Werror -Wextra
LINKFLAGS += -g -static -ffreestanding -nostdlib
LINKFLAGS += -L$(PX_ROOT)/libraries/mmlibc/amd64/lib
LINKFLAGS += -L$(PX_ROOT)/libraries/pcmd/amd64/lib
LINKFLAGS += -L$(PX_ROOT)/libraries/libpx/amd64/lib
LINKFLAGS += -T$(PX_ROOT)/libraries/pxcrt/amd64/pxcrt.ld
LINKFLAGS += -Wl,-error-limit=0

LIBS = $(PROGRAM_LIBS)
LIBS += -lpcmd -lc -lpx




#Build information provided to compiler
BUILDVERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
BUILDDATE := $(shell date -u +%FT%TZ)
BUILDUSER := $(shell whoami)@$(shell hostname)
CFLAGS += -DBUILDVERSION=\"$(BUILDVERSION)\"
CFLAGS += -DBUILDDATE=\"$(BUILDDATE)\"
CFLAGS += -DBUILDUSER=\"$(BUILDUSER)\"

#All objects to build...
CFILES=$(PROGRAM_SRCFILES)
CFILES_OBJECTS=$(patsubst %.c, $(OBJDIR)/$(PROGRAM_BINNAME)/c/%.o, $(CFILES))

ELF_FILE=$(BINDIR)/$(PROGRAM_BINNAME)

#Output location
BINDIR=$(SELFDIR)/bin
OBJDIR=$(SELFDIR)/obj

#Overall targets
all: $(ELF_FILE)

$(ELF_FILE): $(CRTOBJ)/crt0.o $(CRTOBJ)/crti.o $(CFILES_OBJECTS) $(CRTOBJ)/crtn.o
	mkdir -p $(@D)
	$(CC) $(LINKFLAGS) $^ $(LIBS) -o $@ -MMD

#C objects are compiled with GCC
$(OBJDIR)/$(PROGRAM_BINNAME)/c/%.o : %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@ -MMD
	
clean:
	rm -f $(CFILES_OBJECTS)
	rm -f $(BINDIR)/$(PROGRAM_BINNAME)

-include $(patsubst %.o, %.o.d, $(CFILES_OBJECTS))
