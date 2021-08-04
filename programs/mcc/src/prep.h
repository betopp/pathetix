//prep.h
//Preprocessor pass for C compiler
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef PREP_H
#define PREP_H

#include "tok.h"

//Runs preprocessing steps on the given tokenized file, except for final token replacement
void prep_pass(tok_t *tok);

//Replaces preprocessing tokens with language tokens in the given token range.
void prep_repl(tok_t *tok);

#endif //PREP_H
