//charin.h
//Character input for C compiler
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef CHARIN_H
#define CHARIN_H

#include <stdio.h>

//Reads the contents of the given file into memory.
//This handles the first two "translation phases" of ISO9899:1999.
//Maps multibyte characters, replaces trigraphs, and splices lines.
//Returns the file as one giant string.
char *charin_read(FILE *fp);

#endif //CHARIN_H
