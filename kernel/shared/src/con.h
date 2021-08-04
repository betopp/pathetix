//con.h
//Local console as terminal emulator
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef CON_H
#define CON_H

#include <sys/types.h>
#include <stdint.h>

#include "hal_kbd.h"

//Initializes terminal emulator device.
void con_init(void);

//Clears and outputs a panic message to the console.
void con_panic(const char *str);

//Writes to the console.
ssize_t con_write(int minor, const void *buf, size_t len);

//Reads from the console.
ssize_t con_read(int minor, void *buf, size_t len);

//Controls console operation.
int con_ioctl(int minor, uint64_t request, void *ptr, size_t len);

//Called in interrupt context when a key is pressed
void con_kbd(hal_kbd_scancode_t scancode, bool state);

#endif //CON_H
