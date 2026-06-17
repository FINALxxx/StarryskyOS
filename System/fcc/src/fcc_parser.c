#include "fcc.h"
#include "stdio.h"

/* Codegen functions */
void codegen_init(Compiler *comp);
void cg_prologue(void);
void cg_epilogue(void);
void cg_pop(int rd);
void cg_push_imm(int imm);
void cg_add_op(void);
void cg_sub_op(void);
void cg_mul_op(void);
void cg_div_op(void);
void cg_rem_op(void);
void cg_neg_op(void);

/* ===== Error reporting ===== */

static void parse_error(Compiler *comp, const char *msg) {
    printf("fcc:%d: error: %s\r\n", comp->lexer.current.line, msg);
    comp->error = 1;
}

/* ===== Token helpers ===== */

static void expect(Compiler *comp, TokenType type, const char *what) {
    if (!comp->error && comp->lexer.current.type != type) {
        char buf[80];
        snprintf(buf, sizeof(buf), "expected %s", what);
        parse_error(comp, buf);
    }
    if (!comp->error) {
        lexer_next(&comp->lexer);
    }
}

static void advance(Compiler *comp) {
    if (!comp->error) {
        lexer_next(&comp->lexer);
    }
}

/* ===== Expression parser (recursive descent) ===== */

static void parse_add(Compiler *comp);
static void parse_mul(Compiler *comp);
static void parse_unary(Compiler *comp);

/**
 * primary → NUMBER
 *         | "(" add ")"
 *         | IDENT               (Issue 03: variable reference)
 */
static void parse_primary(Compiler *comp) {
    if (comp->lexer.current.type == TOK_NUMBER) {
        int val = comp->lexer.current.value;
        advance(comp);
        cg_push_imm(val);
        return;
    }
    if (comp->lexer.current.type == TOK_LPAREN) {
        advance(comp);  /* consume '(' */
        parse_add(comp);
        expect(comp, TOK_RPAREN, "')'");
        return;
    }
    if (comp->lexer.current.type == TOK_IDENT) {
        parse_error(comp, "variables not yet supported");
        return;
    }
    parse_error(comp, "expected expression");
}

/**
 * unary → "-" unary
 *       | primary
 */
static void parse_unary(Compiler *comp) {
    if (comp->lexer.current.type == TOK_MINUS) {
        advance(comp);  /* consume '-' */
        parse_unary(comp);
        cg_neg_op();
        return;
    }
    parse_primary(comp);
}

/**
 * mul → unary { ("*" | "/" | "%") unary }*
 */
static void parse_mul(Compiler *comp) {
    parse_unary(comp);
    while (!comp->error
           && (comp->lexer.current.type == TOK_STAR
               || comp->lexer.current.type == TOK_SLASH
               || comp->lexer.current.type == TOK_PERCENT)) {
        TokenType op = comp->lexer.current.type;
        advance(comp);
        parse_unary(comp);
        switch (op) {
            case TOK_STAR:    cg_mul_op(); break;
            case TOK_SLASH:   cg_div_op(); break;
            case TOK_PERCENT: cg_rem_op(); break;
            default: break;
        }
    }
}

/**
 * add → mul { ("+" | "-") mul }*
 */
static void parse_add(Compiler *comp) {
    parse_mul(comp);
    while (!comp->error
           && (comp->lexer.current.type == TOK_PLUS
               || comp->lexer.current.type == TOK_MINUS)) {
        TokenType op = comp->lexer.current.type;
        advance(comp);
        parse_mul(comp);
        switch (op) {
            case TOK_PLUS:  cg_add_op(); break;
            case TOK_MINUS: cg_sub_op(); break;
            default: break;
        }
    }
}

/* ===== Statement parser ===== */

/**
 * statement → "return" add ";"
 *           | "if"  ...        (Issue 04)
 *           | "while" ...      (Issue 04)
 */
static void parse_statement(Compiler *comp) {
    if (comp->lexer.current.type == TOK_RETURN) {
        advance(comp);   /* consume "return" */
        parse_add(comp); /* evaluate expression → result on stack */
        cg_pop(REG_A0);  /* pop result into return register */
        expect(comp, TOK_SEMI, "';'");
    } else {
        parse_error(comp, "expected statement");
    }
}

/* ===== Function parser ===== */

static void parse_function(Compiler *comp) {
    expect(comp, TOK_INT, "'int'");

    if (!comp->error && comp->lexer.current.type != TOK_IDENT) {
        parse_error(comp, "expected function name");
    }
    advance(comp);

    expect(comp, TOK_LPAREN, "'('");
    expect(comp, TOK_RPAREN, "')'");
    expect(comp, TOK_LBRACE, "'{'");

    if (!comp->error) {
        cg_prologue();
        parse_statement(comp);
        cg_epilogue();
    }

    expect(comp, TOK_RBRACE, "'}'");
}

/* ===== Top-level ===== */

static void parse_program(Compiler *comp) {
    if (!comp->error && comp->lexer.current.type != TOK_EOF) {
        parse_function(comp);
    }
    if (!comp->error && comp->lexer.current.type != TOK_EOF) {
        parse_error(comp, "unexpected token after function");
    }
}

/* ===== Entry point ===== */

int fcc_compile(Compiler *comp) {
    codegen_init(comp);
    comp->error = 0;

    parse_program(comp);

    return comp->error ? -1 : 0;
}
