//tok.h
//Tokenizer
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef TOK_H
#define TOK_H

#include <stdio.h>
#include <stdbool.h>

//Types of token that we manipulate
typedef enum tok_type_e
{
	TOK_NONE = 0, // None/invalid
	
	TOK_FILE,     // Beginning of file
	TOK_EOF,      // End of file
	TOK_NEWLINE,  // Newline
	TOK_IDENT,    // Identifier
	TOK_STRLIT,   // String literal
	TOK_SYSHDR,   // String but with < > instead of doublequotes
	TOK_CHARACTER,// Character constant
	TOK_PNUMBER,  // Preprocessor number
	TOK_JUNK,     // Untokenizable character
	TOK_INTC,     // Integer constant
	TOK_FLTC,     // Floating constant
	TOK_PUNCT_S,
	TOK_ELLIPS,   // ...
	TOK_DLEQ,     // <<=
	TOK_DGEQ,     // >>=
	TOK_DLT,      // <<
	TOK_DGT,      // >>
	TOK_DPLUS,    // ++
	TOK_DMINUS,   // --
	TOK_PLUSEQ,   // +=
	TOK_MINEQ,    // -=
	TOK_SLSHEQ,   // /=
	TOK_ASTEQ,    // *=
	TOK_EXCEQ,    // !=
	TOK_BAREQ,    // |=
	TOK_AMPEQ,    // &=
	TOK_CAREQ,    // ^=
	TOK_PCTEQ,    // %=
	TOK_DEQ,      // ==
	TOK_LEQ,      // <=
	TOK_GEQ,      // >=
	TOK_DBAR,     // ||
	TOK_DAMP,     // &&
	TOK_ARROW,    // ->
	TOK_DHASH,    // ##
	TOK_LT,       // <
	TOK_GT,       // >
	TOK_HASH,     // #
	TOK_COMMA,    // ,
	TOK_SCOLON,   // ;
	TOK_PLUS,     // +
	TOK_MINUS,    // -
	TOK_SLASH,    // /
	TOK_ASTER,    // *
	TOK_PCT,      // %
	TOK_EQU,      // =
	TOK_BRACKL,   // [
	TOK_BRACKR,   // ]
	TOK_PARENL,   // (
	TOK_PARENR,   // )
	TOK_BRACEL,   // {
	TOK_BRACER,   // }
	TOK_DOT,      // .
	TOK_EXCL,     // !
	TOK_TILDE,    // ~
	TOK_BAR,      // |
	TOK_CARAT,    // ^
	TOK_AMP,      // &
	TOK_QSTN,     // ?
	TOK_COLON,    // :
	TOK_PUNCT_E,
	
	TOK_KEYW_S,
	TOK_AUTO,     // auto
	TOK_BREAK,    // break
	TOK_CASE,     // case
	TOK_CHAR,     // char
	TOK_CONST,    // const
	TOK_CONTINUE, // continue
	TOK_DEFAULT,  // default
	TOK_DO,       // do
	TOK_DOUBLE,   // double
	TOK_ELSE,     // else
	TOK_ENUM,     // enum
	TOK_EXTERN,   // extern
	TOK_FLOAT,    // float
	TOK_FOR,      // for
	TOK_GOTO,     // goto
	TOK_IF,       // if
	TOK_INLINE,   // inline
	TOK_INT,      // int
	TOK_LONG,     // long
	TOK_REGISTER, // register
	TOK_RESTRICT, // restrict
	TOK_RETURN,   // return
	TOK_SHORT,    // short
	TOK_SIGNED,   // signed
	TOK_SIZEOF,   // sizeof
	TOK_STATIC,   // static
	TOK_STRUCT,   // struct
	TOK_SWITCH,   // switch
	TOK_TYPEDEF,  // typedef
	TOK_UNION,    // union
	TOK_UNSIGNED, // unsigned
	TOK_VOID,     // void
	TOK_VOLATILE, // volatile
	TOK_WHILE,    // while
	TOK_BOOL,     // _Bool
	TOK_COMPLEX,  // _Complex
	TOK_IMAGINARY,// _Imaginary
	TOK_KEYW_E,
	
	TOK_MAX
} tok_type_t;


//A file is stored in memory as a doubly-linked list of tokens.
//One link in such a list.
typedef struct tok_s
{
	tok_type_t type; //Type of this token
	char *text; //Original text of the token
	bool immediate; //Whether there was no space between this and the prior token
	int line; //Line where the token started
	struct tok_s *prev; //Previous token in token list
	struct tok_s *next; //Next token in token list
} tok_t;

//Reads the given file and converts it into a list of preprocessor tokens.
//This returns the results of translation phases 1, 2, and 3 as defined by ISO9899:1999.
tok_t *tok_read(FILE *fd);

//Returns a string representing the type of the given token.
const char *tok_typename(const tok_t *tok);

//Removes the given token from a token-list and frees it.
void tok_delete_single(tok_t *tok);

//Removes the given range of tokens, inclusive, from a token-list and frees all of them.
void tok_delete_range(tok_t *first, tok_t *last);

//Frees an entire token list
void tok_delete_all(tok_t *first);

//Makes a copy of the given range of tokens
tok_t *tok_copy(tok_t *first, tok_t *last);

//Makes a copy of the entire list of tokens
tok_t *tok_copy_all(tok_t *first);

//Prints an error about a given token and aborts.
void tok_err(tok_t *violator, const char *str, ...);


//Turns keyword identifiers into keyword tokens after preprocessing
void tok_pass_keyw(tok_t *list);

//Removes all newlines/file delimiters
void tok_pass_nowh(tok_t *list);

//Converts preprocessor numbers into integer or floating point numbers
void tok_pass_nums(tok_t *list);


#endif //TOK_H
