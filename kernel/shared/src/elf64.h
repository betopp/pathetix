//elf64.h
//Executable loading
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef ELF64_H
#define ELF64_H

#include <sys/types.h>
#include "process.h"
#include "hal_uspc.h"

//Loads an ELF from the given file descriptor into the given userspace.
//Returns 0 on success or a negative error number.
int elf64_load(id_t fdid, mem_space_t *mem, uintptr_t *entry_out);

#endif //ELF64_H
