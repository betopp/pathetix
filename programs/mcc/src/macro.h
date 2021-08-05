//macro.h
//Preprocessor macros
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef MACRO_H
#define MACRO_H

#include <stdbool.h>

#include "tok.h"

//Adds a macro definition
void macro_define(const char *name, tok_t *aftername);

//Removes a macro definition by name
void macro_undef(const char *name);

//Returns whether a macro is currently defined
bool macro_isdef(const char *name);

//Processes the given line for macro replacement. Works until end-of-line or end-of-file or the end of the list.
//Returns the token that now starts the new line (which may change if the first token was replaced).
tok_t *macro_process(tok_t *line);


#endif //MACRO_H
