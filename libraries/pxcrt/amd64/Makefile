#Makefile
#Makefile for C runtime on AMD64
#Bryan E. Topp <betopp@betopp.com> 2021

#Toolchain
NASM = nasm

#Output directories
OBJDIR = obj

#Default target: all C runtime objects
allobjs: $(OBJDIR)/crt0.o $(OBJDIR)/crti.o $(OBJDIR)/crtn.o

#Assemble each object
$(OBJDIR)/%.o : src/%.asm
	mkdir -p $(@D)
	$(NASM) -f elf64 -g -F dwarf $< -o $@

clean:
	rm -rf $(OBJDIR)

