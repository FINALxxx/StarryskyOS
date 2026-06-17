#include "fcc.h"
#include "stdio.h"
#include "string.h"

/* ===== Codegen API ===== */
void codegen_init(Compiler *comp);
void cg_prologue(int frame_bytes);
void cg_epilogue(int frame_bytes);
void cg_pop(int rd);
void cg_push_imm(int imm);
void cg_add_op(void);  void cg_sub_op(void);
void cg_mul_op(void);  void cg_div_op(void);  void cg_rem_op(void);
void cg_neg_op(void);
void cg_store_local(int offset);
void cg_load_local(int rd, int offset);
void cg_store_global(int addr);
void cg_load_global(int rd, int addr);
void cg_addi(int rd, int rs1, int imm);
void cg_sw(int rs2, int rs1, int offset);
void cg_li(int rd, int imm);

/* ===== Symbol table ===== */

static VarInfo *sym_add(Compiler *comp, const char *name, int is_global) {
    if (comp->var_count >= MAX_VARS) return NULL;
    VarInfo *v = &comp->vars[comp->var_count++];
    strncpy(v->name, name, 63);
    v->name[63] = '\0';
    v->is_global = is_global;
    v->offset = 0;
    return v;
}

static VarInfo *sym_lookup(Compiler *comp, const char *name) {
    for (int i = comp->var_count - 1; i >= 0; i--) {
        if (strcmp(comp->vars[i].name, name) == 0)
            return &comp->vars[i];
    }
    return NULL;
}

static char *tok_name(Compiler *comp) { return comp->lexer.current.name; }

/* ===== Helpers ===== */

static void parse_error(Compiler *comp, const char *msg) {
    printf("fcc:%d: error: %s\r\n", comp->lexer.current.line, msg);
    comp->error = 1;
}

static void expect(Compiler *comp, TokenType type, const char *what) {
    if (!comp->error && comp->lexer.current.type != type) {
        char buf[80];
        snprintf(buf, sizeof(buf), "expected %s", what);
        parse_error(comp, buf);
    }
    if (!comp->error) lexer_next(&comp->lexer);
}

static void advance(Compiler *comp) {
    if (!comp->error) lexer_next(&comp->lexer);
}

/* ===== Forward declarations ===== */
static void parse_add(Compiler *comp);

/* ===== Expression parser (rvalue) ===== */

static void parse_primary(Compiler *comp) {
    if (comp->lexer.current.type == TOK_NUMBER) {
        int val = comp->lexer.current.value;
        advance(comp);
        cg_push_imm(val);
        return;
    }
    if (comp->lexer.current.type == TOK_IDENT) {
        VarInfo *v = sym_lookup(comp, tok_name(comp));
        if (v == NULL) { parse_error(comp, "undefined variable"); return; }
        advance(comp);
        if (v->is_global) cg_load_global(REG_T0, v->offset);
        else              cg_load_local(REG_T0, v->offset);
        cg_addi(REG_SP, REG_SP, -4);
        cg_sw(REG_T0, REG_SP, 0);
        return;
    }
    if (comp->lexer.current.type == TOK_LPAREN) {
        advance(comp);
        parse_add(comp);
        expect(comp, TOK_RPAREN, "')'");
        return;
    }
    parse_error(comp, "expected expression");
}

static void parse_unary(Compiler *comp) {
    if (comp->lexer.current.type == TOK_MINUS) {
        advance(comp);
        parse_unary(comp);
        cg_neg_op();
        return;
    }
    parse_primary(comp);
}

static void parse_mul(Compiler *comp) {
    parse_unary(comp);
    while (!comp->error
           && (comp->lexer.current.type == TOK_STAR
               || comp->lexer.current.type == TOK_SLASH
               || comp->lexer.current.type == TOK_PERCENT)) {
        TokenType op = comp->lexer.current.type;
        advance(comp);
        parse_unary(comp);
        if      (op == TOK_STAR)    cg_mul_op();
        else if (op == TOK_SLASH)   cg_div_op();
        else                        cg_rem_op();
    }
}

static void parse_add(Compiler *comp) {
    parse_mul(comp);
    while (!comp->error
           && (comp->lexer.current.type == TOK_PLUS
               || comp->lexer.current.type == TOK_MINUS)) {
        TokenType op = comp->lexer.current.type;
        advance(comp);
        parse_mul(comp);
        if (op == TOK_PLUS) cg_add_op();
        else                cg_sub_op();
    }
}

/* ===== Statement helpers ===== */

static void parse_assign(Compiler *comp, VarInfo *v) {
    expect(comp, TOK_ASSIGN, "'='");
    parse_add(comp);
    cg_pop(REG_T0);                          /* RHS value → t0 */
    if (v->is_global)
        cg_store_global(v->offset);
    else
        cg_store_local(v->offset);           /* store, no push-back */
    expect(comp, TOK_SEMI, "';'");
}

/**
 * Parse local var declaration: "int" already consumed.
 * Only adds to symbol table — no code emitted (declarations precede
 * prologue).  Initialization must use a separate assignment statement.
 */
static void parse_local_decl(Compiler *comp) {
    if (comp->lexer.current.type != TOK_IDENT) {
        parse_error(comp, "expected variable name");
        return;
    }
    char *name = tok_name(comp);
    for (int i = comp->local_base; i < comp->var_count; i++) {
        if (strcmp(comp->vars[i].name, name) == 0) {
            parse_error(comp, "duplicate variable");
            return;
        }
    }
    advance(comp);  /* consume IDENT */

    VarInfo *v = sym_add(comp, name, 0);
    v->offset = comp->local_bytes;
    comp->local_bytes += 4;

    /* No initializer in declaration — use a separate assignment stmt */
    if (comp->lexer.current.type == TOK_ASSIGN) {
        parse_error(comp, "initializer not supported in declaration; use 'x = ...;'");
        return;
    }
    expect(comp, TOK_SEMI, "';'");
}

/* ===== Function body (after "int IDENT") ===== */

static void parse_function_body(Compiler *comp, const char *name) {
    expect(comp, TOK_LPAREN, "'('");
    expect(comp, TOK_RPAREN, "')'");
    expect(comp, TOK_LBRACE, "'{'");

    /* Pass 1: collect local declarations */
    comp->local_base = comp->var_count;
    comp->local_bytes = 0;

    while (!comp->error
           && comp->lexer.current.type == TOK_INT) {
        advance(comp);
        parse_local_decl(comp);
    }

    int frame = 16 + comp->local_bytes;
    cg_prologue(frame);

    /* Pass 2: emit statement code (loop for multiple statements) */
    while (!comp->error && comp->lexer.current.type != TOK_RBRACE) {
        if (comp->lexer.current.type == TOK_RETURN) {
            advance(comp);
            parse_add(comp);
            cg_pop(REG_A0);
            expect(comp, TOK_SEMI, "';'");
        } else if (comp->lexer.current.type == TOK_IDENT) {
            VarInfo *v = sym_lookup(comp, tok_name(comp));
            if (v == NULL) {
                parse_error(comp, "undefined variable");
            } else {
                advance(comp);
                parse_assign(comp, v);
            }
        } else {
            parse_error(comp, "expected statement");
            break;
        }
    }

    cg_epilogue(frame);

    /* Pop locals from symbol table */
    comp->var_count = comp->local_base;

    expect(comp, TOK_RBRACE, "'}'");
}

/* ===== Top-level parser ===== */

/**
 * program → { global_decl | function }
 *
 * Both start with "int" IDENT.  We consume "int" IDENT, then:
 *   '('  → function
 *   ';' or '=' → global variable
 */
static void parse_program(Compiler *comp) {
    while (!comp->error && comp->lexer.current.type != TOK_EOF) {
        if (comp->lexer.current.type != TOK_INT) {
            parse_error(comp, "expected 'int' at top level");
            break;
        }
        advance(comp);  /* consume "int" */

        if (comp->lexer.current.type != TOK_IDENT) {
            parse_error(comp, "expected identifier");
            break;
        }
        char name[64];
        strncpy(name, tok_name(comp), 63);
        name[63] = '\0';
        advance(comp);  /* consume IDENT */

        if (comp->lexer.current.type == TOK_LPAREN) {
            /* Function */
            parse_function_body(comp, name);
        } else {
            /* Global variable */
            if (sym_lookup(comp, name)) {
                parse_error(comp, "duplicate global variable");
                break;
            }
            VarInfo *v = sym_add(comp, name, 1);
            v->offset = FCC_GLOBAL_BASE + comp->next_global;
            comp->next_global += 4;

            if (comp->lexer.current.type == TOK_ASSIGN) {
                advance(comp);
                if (comp->lexer.current.type != TOK_NUMBER) {
                    parse_error(comp, "global init must be constant");
                    break;
                }
                cg_li(REG_T0, comp->lexer.current.value);
                cg_store_global(v->offset);
                advance(comp);
            }
            expect(comp, TOK_SEMI, "';'");
        }
    }
}

/* ===== Entry point ===== */

int fcc_compile(Compiler *comp) {
    codegen_init(comp);
    comp->error       = 0;
    comp->var_count   = 0;
    comp->next_global = 0;

    parse_program(comp);

    return comp->error ? -1 : 0;
}
