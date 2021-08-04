//consts.h
//Type and value determination for constant syntax nodes
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef CONSTS_H
#define CONSTS_H

#include "syntax.h"

//Determines type and value of an integer constant, as represented in a syntax tree.
//Stores the result in the syntax tree.
void consts_intc(syntax_node_t *node);

//Determines type and value of a floating constant, as represented in a syntax tree.
//Stores the result in the syntax tree.
void consts_fltc(syntax_node_t *node);

#endif //CONSTS_H
