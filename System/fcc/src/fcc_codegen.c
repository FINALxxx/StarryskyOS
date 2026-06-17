#include "fcc.h"
#include "stdio.h"

/* ===== RISC-V Register Numbers ===== */
enum {
    REG_ZERO = 0,
    REG_RA   = 1,
    REG_SP   = 2,
    REG_T0   = 5,
    REG_T1   = 6,
    REG_T2   = 7,
    REG_A0   = 10,
};

/* ===== RISC-V Opcodes ===== */
enum {
    OP_IMM  = 0x13,
    OP      = 0x33,
    LOAD    = 0x03,
    STORE   = 0x23,
    LUI     = 0x37,
    JALR    = 0x67,
    JAL     = 0x6F,
    BRANCH  = 0x63,
    AUIPC   = 0x17,
};

/* ===== funct3 / funct7 ===== */
enum {
    F3_ADD_SUB  = 0,   /* funct7=0x00 → ADD, funct7=0x20 → SUB */
    F3_LW       = 2,
    F3_SW       = 2,
    F3_JALR     = 0,
    F3_BEQ      = 0,
    F3_BNE      = 1,
    F3_BLT      = 4,
    F3_BGE      = 5,
    F7_NORMAL   = 0x00,
    F7_SUB      = 0x20,
    F7_MUL_DIV  = 0x01,  /* M extension */
};

/* ===== Instruction encoding helpers ===== */

static uint32_t enc_r(uint32_t funct7, uint32_t rs2, uint32_t rs1,
                       uint32_t funct3, uint32_t rd, uint32_t opcode) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15)
         | (funct3 << 12) | (rd << 7) | opcode;
}

static uint32_t enc_i(uint32_t imm12, uint32_t rs1, uint32_t funct3,
                       uint32_t rd, uint32_t opcode) {
    return ((imm12 & 0xFFF) << 20) | (rs1 << 15)
         | (funct3 << 12) | (rd << 7) | opcode;
}

static uint32_t enc_s(uint32_t imm12, uint32_t rs2, uint32_t rs1,
                       uint32_t funct3, uint32_t opcode) {
    return ((imm12 & 0xFE0) << 20) | (rs2 << 20) | (rs1 << 15)
         | (funct3 << 12) | ((imm12 & 0x1F) << 7) | opcode;
}

static uint32_t enc_u(uint32_t imm20, uint32_t rd, uint32_t opcode) {
    return ((imm20 & 0xFFFFF) << 12) | (rd << 7) | opcode;
}

/* ===== Code buffer management ===== */

static Compiler *g_compiler;

void codegen_init(Compiler *comp) {
    g_compiler = comp;
    comp->code      = (uint32_t *)FCC_CODE_BASE;
    comp->code_size = 0;
}

static void emit(uint32_t instr) {
    if (g_compiler->code_size < FCC_CODE_MAX) {
        g_compiler->code[g_compiler->code_size / 4] = instr;
        g_compiler->code_size += 4;
    }
}

/* ===== Instruction emitters ===== */

/* ADDI rd, rs1, imm12 */
void cg_addi(int rd, int rs1, int imm12) {
    emit(enc_i((uint32_t)imm12, (uint32_t)rs1, F3_ADD_SUB,
               (uint32_t)rd, OP_IMM));
}

/* ADD rd, rs1, rs2 */
void cg_add(int rd, int rs1, int rs2) {
    emit(enc_r(F7_NORMAL, (uint32_t)rs2, (uint32_t)rs1,
               F3_ADD_SUB, (uint32_t)rd, OP));
}

/* SUB rd, rs1, rs2 */
void cg_sub(int rd, int rs1, int rs2) {
    emit(enc_r(F7_SUB, (uint32_t)rs2, (uint32_t)rs1,
               F3_ADD_SUB, (uint32_t)rd, OP));
}

/* MUL rd, rs1, rs2  (RV32M) */
void cg_mul(int rd, int rs1, int rs2) {
    emit(enc_r(F7_MUL_DIV, (uint32_t)rs2, (uint32_t)rs1,
               0, (uint32_t)rd, OP));
}

/* DIV rd, rs1, rs2  (RV32M) */
void cg_div(int rd, int rs1, int rs2) {
    emit(enc_r(F7_MUL_DIV, (uint32_t)rs2, (uint32_t)rs1,
               4, (uint32_t)rd, OP));
}

/* REM rd, rs1, rs2  (RV32M) */
void cg_rem(int rd, int rs1, int rs2) {
    emit(enc_r(F7_MUL_DIV, (uint32_t)rs2, (uint32_t)rs1,
               6, (uint32_t)rd, OP));
}

/* LW rd, offset(rs1) */
void cg_lw(int rd, int rs1, int offset) {
    emit(enc_i((uint32_t)offset, (uint32_t)rs1, F3_LW,
               (uint32_t)rd, LOAD));
}

/* SW rs2, offset(rs1) */
void cg_sw(int rs2, int rs1, int offset) {
    emit(enc_s((uint32_t)offset, (uint32_t)rs2, (uint32_t)rs1,
               F3_SW, STORE));
}

/* LUI rd, imm20 — load upper immediate */
void cg_lui(int rd, int imm20) {
    emit(enc_u((uint32_t)imm20, (uint32_t)rd, LUI));
}

/* JALR rd, rs1, offset  —  rd=0, rs1=ra, offset=0  → ret */
void cg_jalr(int rd, int rs1, int offset) {
    emit(enc_i((uint32_t)offset, (uint32_t)rs1, F3_JALR,
               (uint32_t)rd, JALR));
}

/* JAL rd, offset  — jump and link */
void cg_jal(int rd, int offset) {
    /* J-type: imm[20|10:1|11|19:12] | rd | opcode */
    uint32_t imm = (uint32_t)offset;
    uint32_t instr = ((imm & 0x80000) << 11)       /* imm[20] */
                   | ((imm & 0x7FE)  << 20)         /* imm[10:1] */
                   | ((imm & 0x800)   <<  9)        /* imm[11] */
                   | ((imm & 0xFF000) >> 12)        /* imm[19:12] */
                   | ((uint32_t)rd << 7)
                   | JAL;
    emit(instr);
}

/* Branch instructions */
void cg_beq(int rs1, int rs2, int offset) {
    uint32_t imm = (uint32_t)offset;
    uint32_t instr = ((imm & 0x800) << 20)          /* imm[12] → bit 31 */
                   | ((imm & 0x1E)  <<  7)          /* imm[4:1] → bits 11:8 */
                   | ((imm & 0x7E0) << 20)          /* imm[10:5] → bits 30:25 */
                                                   | ((uint32_t)rs2 << 20)
                                                   | ((uint32_t)rs1 << 15)
                                                   | (F3_BEQ << 12)
                                                   | BRANCH;
    emit(instr);
}

void cg_bne(int rs1, int rs2, int offset) {
    uint32_t imm = (uint32_t)offset;
    uint32_t instr = ((imm & 0x800) << 20)
                   | ((imm & 0x1E)  <<  7)
                   | ((imm & 0x7E0) << 20)
                   | ((uint32_t)rs2 << 20)
                   | ((uint32_t)rs1 << 15)
                   | (F3_BNE << 12)
                   | BRANCH;
    emit(instr);
}

void cg_blt(int rs1, int rs2, int offset) {
    uint32_t imm = (uint32_t)offset;
    uint32_t instr = ((imm & 0x800) << 20)
                   | ((imm & 0x1E)  <<  7)
                   | ((imm & 0x7E0) << 20)
                   | ((uint32_t)rs2 << 20)
                   | ((uint32_t)rs1 << 15)
                   | (F3_BLT << 12)
                   | BRANCH;
    emit(instr);
}

void cg_bge(int rs1, int rs2, int offset) {
    uint32_t imm = (uint32_t)offset;
    uint32_t instr = ((imm & 0x800) << 20)
                   | ((imm & 0x1E)  <<  7)
                   | ((imm & 0x7E0) << 20)
                   | ((uint32_t)rs2 << 20)
                   | ((uint32_t)rs1 << 15)
                   | (F3_BGE << 12)
                   | BRANCH;
    emit(instr);
}

/* ===== Compound operations ===== */

/* li rd, imm  — load immediate (pseudo-instruction) */
void cg_li(int rd, int imm) {
    if (imm >= -2048 && imm <= 2047) {
        /* Fits in 12-bit signed */
        cg_addi(rd, REG_ZERO, imm);
    } else {
        /* Need LUI + ADDI sequence */
        int upper = imm >> 12;
        int lower = imm & 0xFFF;
        /* Sign-extend correction */
        if (lower & 0x800) {
            upper = (upper + 1) & 0xFFFFF;
            lower = lower - 0x1000; /* lower is now negative */
        }
        if (upper != 0) {
            cg_lui(rd, upper);
            if (lower != 0) {
                cg_addi(rd, rd, lower);
            }
        } else {
            cg_addi(rd, REG_ZERO, lower);
        }
    }
}

/* Function prologue */
void cg_prologue(void) {
    cg_addi(REG_SP, REG_SP, -16);
    cg_sw(REG_RA, REG_SP, 12);
}

/* Function epilogue */
void cg_epilogue(void) {
    cg_lw(REG_RA, REG_SP, 12);
    cg_addi(REG_SP, REG_SP, 16);
    cg_jalr(REG_ZERO, REG_RA, 0);  /* ret */
}

/* Set return value */
void cg_retval(int imm) {
    cg_li(REG_A0, imm);
}
