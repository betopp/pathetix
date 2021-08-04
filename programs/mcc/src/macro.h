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

//Processes the given token for macro replacement. Parameter is the first token, potentially, in the use of a macro.
//May delete and replace the token (and some following tokens, if parameters are used)
//Variables *repl_start_out and *repl_end_out are set to the first/last token in the replacement sequence.
//Variable *follow_out is set to the input token following the replaced macro.
void macro_process(tok_t *tok, tok_t **repl_start_out, tok_t **repl_end_out, tok_t **follow_out);

#endif //MACRO_H
