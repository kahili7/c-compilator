#ifndef X_INCLUDE_ASM64
#define X_INCLUDE_ASM64

#include "..\include\operand.h"

//бинарные команды
typedef enum BIN_OPERATION {
    BINOP_UNDEFINED,
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_BITAND,
    BINOP_BITOR,
    BINOP_BITXOR,
    BINOP_SHR,
    BINOP_SHL
} BIN_OPERATION;

//унарные команды
typedef enum UNARY_OPERATION {
    UNARY_UNDEFINED,
    UNARY_INC,
    UNARY_DEC,
    UNARY_NEG,
    UNARY_BITWISENOT
} UNARY_OPERATION;

void AsmComment (AsmCTX* ctx, const char* str);

void AsmUOP (IrCTX* ir, IrBLOCK* block, UNARY_OPERATION Op, Operand R);
void AsmBOP (IrCTX* ir, IrBLOCK* block, BIN_OPERATION Op, Operand L, Operand R);
void AsmDivision (IrCTX* ir, IrBLOCK* block, Operand R);
void AsmCompare (IrCTX* ir, IrBLOCK* block, Operand L, Operand R);
void AsmRepStos (IrCTX* ir, IrBLOCK* block, Operand RAX, Operand RCX, Operand RDI, Operand Dest, int length, Operand Src);
void AsmConditionalMove (IrCTX* ir, IrBLOCK* block, Operand Cond, Operand Dest, Operand Src);
void AsmMove (IrCTX* ir, IrBLOCK* block, Operand Dest, Operand Src);

void AsmReturn (AsmCTX* ctx);
void AsmCall (AsmCTX* ctx, const char* label);
void AsmCallIndirect (IrBLOCK* block, Operand L);
void AsmBranch (AsmCTX* ctx, Operand Condition, const char* label);
void AsmJump (AsmCTX* ctx, const char* label);
void AsmLabel (AsmCTX* ctx, const char* label);

void AsmStringConstant (AsmCTX* ctx, const char* label, const char* str);
void AsmStaticData (AsmCTX* ctx, const char* label, int global, int size, intptr_t initial);
void AsmRODataSection (AsmCTX* ctx);
void AsmDataSection (AsmCTX* ctx);

void AsmPush (IrCTX* ir, IrBLOCK* block, Operand L);
void AsmPop (IrCTX* ir, IrBLOCK* block, Operand L);

void AsmPushN (IrCTX* ir, IrBLOCK* block, int n);
void asmPopN (IrCTX* ir, IrBLOCK* block, int n);

void AsmSaveReg (IrCTX* ir, IrBLOCK* block, REG_INDEX r);
void AsmRestoreReg (IrCTX* ir, IrBLOCK* block, REG_INDEX r);

void AsmEvalAddress (IrCTX* ir, IrBLOCK* block, Operand L, Operand R);

void AsmFnPrologue (IrCTX* ir, IrBLOCK* block, int localSize);
void AsmFnEpilogue (IrCTX* ir, IrBLOCK* block);

void AsmFnLinkageBegin (FILE* file, const char* name);
void AsmFnLinkageEnd (FILE* file, const char* name);

void AsmFilePrologue (AsmCTX* ctx);
void AsmFileEpilogue (AsmCTX* ctx);
#endif /*X_INCLUDE_ASM64*/