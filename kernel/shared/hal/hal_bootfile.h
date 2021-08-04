//hal_bootfile.h
//HAL interface - bootloader-provided files
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_BOOTFILE_H
#define HAL_BOOTFILE_H

#include "hal_frame.h"
#include <sys/types.h>

//Returns the number of files provided by the bootloader.
int hal_bootfile_count(void);

//Returns the address of the given file provided by the bootloader. Returns HAL_FRAME_ID_INVALID for invalid indices.
hal_frame_id_t hal_bootfile_addr(int idx);

//Returns the size of the given file provided by the bootloader. Returns 0 for invalid indices.
size_t hal_bootfile_size(int idx);

#endif //HAL_BOOTFILE_H

