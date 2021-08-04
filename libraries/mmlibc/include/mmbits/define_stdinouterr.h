//mmlibc/include/mmbits/define_stdinouterr.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_STDINOUTERR_H
#define _DEFINE_STDINOUTERR_H

#include <mmbits/typedef_file.h>

extern FILE _stdin_storage;
extern FILE _stdout_storage;
extern FILE _stderr_storage;

#define stdin (&_stdin_storage)
#define stdout (&_stdout_storage)
#define stderr (&_stderr_storage)

#endif //_DEFINE_STDINOUTERR_H

