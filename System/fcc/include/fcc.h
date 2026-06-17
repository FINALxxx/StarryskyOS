#ifndef __FCC_H__
#define __FCC_H__

#include <stdint.h>
#include <stddef.h>

/* ===== Code Buffer ===== */
/* Internal SRAM — must be executable; PSRAM (0x40000000+) is data-only */
#define FCC_CODE_BASE   0x30000000
#define FCC_CODE_MAX    (32 * 1024)

/* ===== Source Buffer ===== */
#define FCC_SRC_MAX     (32 * 1024)

/* ===== Token Types ===== */
typedef enum {
    TOK_EOF = 0,
    /* Keywords */
    TOK_INT, TOK_RETURN, TOK_IF, TOK_ELSE, TOK_WHILE,
    /* Literals & identifiers */
    TOK_IDENT, TOK_NUMBER,
    /* Single-char */
    TOK_LBRACE,    /* { */
    TOK_RBRACE,    /* } */
    TOK_LPAREN,    /* ( */
    TOK_RPAREN,    /* ) */
    TOK_SEMI,      /* ; */
    TOK_ASSIGN,    /* = */
    TOK_COMMA,     /* , */
    /* Operators */
    TOK_PLUS,      /* + */
    TOK_MINUS,     /* - */
    TOK_STAR,      /* * */
    TOK_SLASH,     /* / */
    TOK_PERCENT,   /* % */
    TOK_EQ,        /* == */
    TOK_NE,        /* != */
    TOK_LT,        /* < */
    TOK_GT,        /* > */
    TOK_LE,        /* <= */
    TOK_GE,        /* >= */
    TOK_AND,       /* && */
    TOK_OR,        /* || */
    TOK_NOT,       /* ! */
} TokenType;

/* ===== Token ===== */
typedef struct {
    TokenType type;
    int       value;       /* TOK_NUMBER */
    char      name[64];    /* TOK_IDENT */
    int       line;        /* source line */
} Token;

/* ===== Lexer ===== */
typedef struct {
    const char *src;
    int         pos;
    int         line;
    Token       current;
} Lexer;

/* ===== Compiler ===== */
typedef struct {
    Lexer     lexer;
    uint32_t *code;
    uint32_t  code_size;   /* bytes */
    int       error;
} Compiler;

/* ===== Lexer API (used across fcc modules) ===== */
void lexer_init(Lexer *lex, const char *src);
void lexer_next(Lexer *lex);

/* ===== Public API ===== */
int fccCmd(int argc, char *argv[]);

#endif /* __FCC_H__ */
