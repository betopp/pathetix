//mmlibc/include/termios.h
//Terminal control declarations for MMK's libc.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TERMIOS_H
#define _TERMIOS_H

#include <mmbits/typedef_cc.h>
#include <mmbits/typedef_speed.h>
#include <mmbits/typedef_tcflag.h>
#include <mmbits/struct_termios.h>
#include <mmbits/define_termccs.h>
#include <mmbits/define_termimodes.h>
#include <mmbits/define_termomodes.h>
#include <mmbits/define_termbauds.h>
#include <mmbits/define_termctrl.h>
#include <mmbits/define_termlmodes.h>
#include <mmbits/define_termattrs.h>
#include <mmbits/define_termline.h>
#include <mmbits/typedef_pid.h>
#include <mmbits/struct_winsize.h>

speed_t cfgetispeed(const struct termios *termios_p);
speed_t cfgetospeed(const struct termios *termios_p);
int cfsetispeed(struct termios *termios_p, speed_t speed);
int cfsetospeed(struct termios *termios_p, speed_t speed);
int tcdrain(int fd);
int tcflow(int fd, int action);
int tcflush(int fd, int queue_selector);
int tcgetattr(int fd, struct termios *termios_p);
pid_t tcgetsid(int fd);
int tcsendbreak(int fd, int duration);
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);

#endif // _TERMIOS_H
