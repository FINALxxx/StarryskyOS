#include "fcc.h"
#include "stdio.h"

/* Codegen functions used by parser */
void codegen_init(Compiler *comp);
void cg_prologue(void);
void cg_epilogue(void);
void cg_retval(int imm);

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

/* consume current token unconditionally */
static void advance(Compiler *comp) {
    if (!comp->error) {
        lexer_next(&comp->lexer);
    }
}

/* ===== Expression parser ===== */

/**
 * Issue 01: only integer literal after "return".
 * Later issues extend this for arithmetic, variables, function calls.
 */
static void parse_expr(Compiler *comp) {
    if (comp->lexer.current.type == TOK_NUMBER) {
        int val = comp->lexer.current.value;
        advance(comp);
        cg_retval(val);
        return;
    }
    if (comp->lexer.current.type == TOK_IDENT) {
        parse_error(comp, "variables not yet supported");
        return;
    }
    parse_error(comp, "expected expression");
}

/* ===== Statement parser ===== */

static void parse_statement(Compiler *comp) {
    if (comp->lexer.current.type == TOK_RETURN) {
        advance(comp);  /* consume "return" */
        parse_expr(comp);
        expect(comp, TOK_SEMI, "';'");
    } else {
        parse_error(comp, "expected statement");
    }
}

/* ===== Function parser ===== */

static void parse_function(Compiler *comp) {
    /* type: "int" */
    expect(comp, TOK_INT, "'int'");

    /* function name */
    if (!comp->error && comp->lexer.current.type != TOK_IDENT) {
        parse_error(comp, "expected function name");
    }
    advance(comp);

    /* parameter list: "(" ")" */
    expect(comp, TOK_LPAREN, "'('");
    expect(comp, TOK_RPAREN, "')'");

    /* body */
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
    /* Issue 01: one function only */
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
