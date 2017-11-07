#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "..\include\asm64.h"
#include "..\include\asm.h"
#include "..\include\register.h"
#include "..\include\arch.h"
#include "..\include\debug.h"
#include "..\include\util.h"

//секция данных для глобальных и статических переменных
void AsmDataSection (AsmCTX* ctx)
{
    AsmOutLn(ctx, ".section .data");
}

//секция данных для неизменяемых переменных
void AsmRODataSection (AsmCTX* ctx)
{
    AsmOutLn(ctx, ".section .rodata");
}

//статические данные
void AsmStaticData (AsmCTX* ctx, const char* label, int global, int size, intptr_t initial)
{
    if (global)
        AsmOutLn(ctx, ".globl %s", label);

    AsmOutLn(ctx, "%s:", label);

    if (size == 1) AsmOutLn(ctx, ".byte %d", initial);
    else if (size == 2) AsmOutLn(ctx, ".word %d", initial);
    else if (size == 4) AsmOutLn(ctx, ".quad %d", initial);
    else if (size == 8) AsmOutLn(ctx, ".octa %d", initial);
    else
        DebugErrorUnhandledInt("AsmStaticData", "data size", size);
}

//строка
void AsmStringConstant (AsmCTX* ctx, const char* label, const char* str)
{
    AsmOutLn(ctx, "%s:", label);
    AsmOutLn(ctx, ".asciz \"%s\"", str);
}

//метка
void AsmLabel (AsmCTX* ctx, const char* label)
{
    AsmOutLn(ctx, "\t%s:", label);
}

void AsmJump (AsmCTX* ctx, const char* label)
{
    AsmOutLn(ctx, "jmp %s", label);
}

void AsmBranch (AsmCTX* ctx, Operand Condition, const char* label)
{
    char* CStr = OperandToStr(Condition);
    
    AsmOutLn(ctx, "j%s %s", CStr, label);
    free(CStr);
}

void AsmCall (AsmCTX* ctx, const char* label)
{
    AsmOutLn(ctx, "call %s", label);
}

void AsmCallIndirect (IrBLOCK* block, Operand L)
{
    char* LStr = OperandToStr(L);
    
    IrBlockOut(block, "call %s", LStr);
    free(LStr);
}

void AsmReturn (AsmCTX* ctx)
{
    AsmOutLn(ctx, "ret");
}

//толкнуть элемент в стек
void AsmPush (IrCTX* ir, IrBLOCK* block, Operand L)
{
    AsmCTX* ctx = ir->assem;

    /*флаги*/
    if (L.tag == OPERAND_FLAGS)
    {
        AsmPush(ir, block, OperandCreateLiteral(0));
        
        Operand top = OperandCreateMem(ctx->stackPtr.base, 0, ctx->arch->wordsize);
        
        AsmConditionalMove(ir, block, L, top, OperandCreateLiteral(1));

    /* > чем слово*/
    }
    else if (OperandGetSize(ctx->arch, L) > ctx->arch->wordsize)
    {
        if (DebugAssert("AsmPush", "memory operand", L.tag == OPERAND_MEM)) return;

        int size = OperandGetSize(ctx->arch, L);

        L.offset += size;
        L.size = ctx->arch->wordsize;

        for (int i = 0; i < size; i += ctx->arch->wordsize)
        {
            L.offset -= ctx->arch->wordsize;
            AsmPush(ir, block, L);
        }

    /* < чем слово*/
    }
    else if (L.tag == OPERAND_MEM && OperandGetSize(ctx->arch, L) < ctx->arch->wordsize)
    {
        Operand intermediate = OperandCreateReg(RegAlloc(ctx->arch->wordsize));
        
        AsmMove(ir, block, intermediate, L);
        AsmPush(ir, block, intermediate);
        OperandFree(intermediate);
    }
    else
    {
        char* LStr = OperandToStr(L);
        
        IrBlockOut(block, "push %s", LStr);
        free(LStr);
    }
}

//вытолкнуть элемент из стека
void AsmPop (IrCTX* ir, IrBLOCK* block, Operand L)
{
    (void) ir;

    char* LStr = OperandToStr(L);
    
    IrBlockOut(block, "pop %s", LStr);
    free(LStr);
}

void AsmPushN (IrCTX* ir, IrBLOCK* block, int n)
{
    AsmCTX* ctx = ir->assem;

    if (n)
        AsmBOP(ir, block, BINOP_SUB, ctx->stackPtr, OperandCreateLiteral(n * ctx->arch->wordsize));
}

void asmPopN (IrCTX* ir, IrBLOCK* block, int n)
{
    AsmCTX* ctx = ir->assem;

    if (n)
        AsmBOP(ir, block, BINOP_ADD, ctx->stackPtr, OperandCreateLiteral(n * ctx->arch->wordsize));
}

void AsmMove (IrCTX* ir, IrBLOCK* block, Operand Dest, Operand Src)
{
    AsmCTX* ctx = ir->assem;

    if (Dest.tag == OPERAND_INVALID || Src.tag == OPERAND_INVALID) return;

    /*слишком большой для регистра*/
    else if (OperandGetSize(ctx->arch, Dest) > ctx->arch->wordsize)
    {
        if (DebugAssert("AsmMove", "Dest mem", Dest.tag == OPERAND_MEM) || DebugAssert("AsmMove", "Src mem", Src.tag == OPERAND_MEM)
           || DebugAssert("AsmMove", "operand size equality", OperandGetSize(ctx->arch, Dest) == OperandGetSize(ctx->arch, Src))) return;

        int size = OperandGetSize(ctx->arch, Dest);
        int chunk = ctx->arch->wordsize;
        
        Dest.size = Src.size = chunk;

        /*Move up the operands, so far as a chunk would not go past their ends*/
        for (int i = 0; i + chunk <= size; i += chunk, Dest.offset += chunk, Src.offset += chunk)
            AsmMove(ir, block, Dest, Src);

        /*Were the operands not an even multiple of the chunk size?*/
        if (size % chunk != 0)
        {
            /*Final chunk size is the remainder*/
            Dest.size = Src.size = size % chunk;
            AsmMove(ir, block, Dest, Src);
        }

    /*оба операнда в памяти*/
    }
    else if (OperandIsMem(Dest) && OperandIsMem(Src))
    {
        Operand intermediate = OperandCreateReg(RegAlloc(max(Dest.size, Src.size)));
        
        AsmMove(ir, block, intermediate, Src);
        AsmMove(ir, block, Dest, intermediate);
        OperandFree(intermediate);
    /*флаги*/
    }
    else if (Src.tag == OPERAND_FLAGS)
    {
        AsmMove(ir, block, Dest, OperandCreateLiteral(0));
        AsmConditionalMove(ir, block, Src, Dest, OperandCreateLiteral(1));
    }
    else
    {
        char* DestStr = OperandToStr(Dest);
        char* SrcStr = OperandToStr(Src);

        if (OperandGetSize(ctx->arch, Dest) > OperandGetSize(ctx->arch, Src) && Src.tag != OPERAND_LITERAL)
            IrBlockOut(block, "movzx %s, %s", DestStr, SrcStr);
        else
            IrBlockOut(block, "mov %s, %s", DestStr, SrcStr);

        free(DestStr);
        free(SrcStr);
    }
}

void AsmConditionalMove (IrCTX* ir, IrBLOCK* block, Operand Cond, Operand Dest, Operand Src)
{
    char falseLabel[10];
    
    sprintf(falseLabel, ".%X", ir->labelNo++);

    Cond.condition = ConditionNegate(Cond.condition);
    char* cond = OperandToStr(Cond);

    IrBlockOut(block, "j%s %s", cond, falseLabel);
    AsmMove(ir, block, Dest, Src);
    IrBlockOut(block, "%s:", falseLabel);

    free(cond);
}

//сохрание значения в стеке
void AsmSaveReg (IrCTX* ir, IrBLOCK* block, REG_INDEX r)
{
    AsmCTX* ctx = ir->assem;
    
    IrBlockOut(block, "push %s", RegIndexGetName(r, ctx->arch->wordsize));
}

//извлечение значения из стеке
void AsmRestoreReg (IrCTX* ir, IrBLOCK* block, REG_INDEX r)
{
    AsmCTX* ctx = ir->assem;
    
    IrBlockOut(block, "pop %s", RegIndexGetName(r, ctx->arch->wordsize));
}

//проход по все строке
void AsmRepStos (IrCTX* ir, IrBLOCK* block, Operand RAX, Operand RCX, Operand RDI, Operand Dest, int length, Operand Src)
{
    int chunksize = ir->arch->wordsize;
    int iterations = length / chunksize;

    AsmMove(ir, block, RAX, Src);
    AsmMove(ir, block, RCX, OperandCreateLiteral(iterations));
    AsmEvalAddress(ir, block, RDI, Dest);

    IrBlockOut(block, "rep stos%s", chunksize == 8 ? "q" : "d");
}

//загрузка эффективного адреса
void AsmEvalAddress (IrCTX* ir, IrBLOCK* block, Operand L, Operand R)
{
    AsmCTX* ctx = ir->assem;

    if (OperandIsMem(L) && OperandIsMem(R))
    {
        Operand intermediate = OperandCreateReg(RegAlloc(ctx->arch->wordsize));
        
        AsmEvalAddress(ir, block, intermediate, R);
        AsmMove(ir, block, L, intermediate);
        OperandFree(intermediate);
    }
    else
    {
        char* LStr = OperandToStr(L);
        
        R.size = ctx->arch->wordsize;
        
        char* RStr = OperandToStr(R);
        
        IrBlockOut(block, "lea %s, %s", LStr, RStr);
        free(LStr);
        free(RStr);
    }
}

//сравнение двух значений
void AsmCompare (IrCTX* ir, IrBLOCK* block, Operand L, Operand R)
{
    AsmCTX* ctx = ir->assem;

    if ((OperandIsMem(L) && OperandIsMem(R)) || (L.tag == OPERAND_LITERAL && R.tag == OPERAND_LITERAL))
    {
        Operand intermediate = OperandCreateReg(RegAlloc(L.tag == OPERAND_MEM ? max(L.size, R.size) : ctx->arch->wordsize));
        
        AsmMove(ir, block, intermediate, L);
        AsmCompare(ir, block, intermediate, R);
        OperandFree(intermediate);
    }
    else if (L.tag == OPERAND_LITERAL)
    {
        AsmCompare(ir, block, R, L);
    }
    else
    {
        char* LStr = OperandToStr(L);
        char* RStr = OperandToStr(R);
        
        IrBlockOut(block, "cmp %s, %s", LStr, RStr);
        free(LStr);
        free(RStr);
    }
}

//бинарные операции
void AsmBOP (IrCTX* ir, IrBLOCK* block, BIN_OPERATION Op, Operand L, Operand R)
{
    if (OperandIsMem(L) && OperandIsMem(R))
    {
        Operand intermediate = OperandCreateReg(RegAlloc(max(L.size, R.size)));
        
        AsmMove(ir, block, intermediate, R);
        AsmBOP(ir, block, Op, L, intermediate);
        OperandFree(intermediate);
    }
    else if (Op == BINOP_MUL && L.tag == OPERAND_MEM)
    {
        if (R.tag == OPERAND_REG)
        {
            AsmBOP(ir, block, BINOP_MUL, R, L);
            AsmMove(ir, block, L, R);
            OperandFree(R);
        }
        else
        {
            Operand tmp = OperandCreateReg(RegAlloc(max(L.size, R.size)));

            char* LStr = OperandToStr(L);
            char* RStr = OperandToStr(R);
            char* tmpStr = OperandToStr(tmp);
            
            //приемник = источник * число 
            IrBlockOut(block, "imul %s, %s, %s", tmpStr, LStr, RStr);
            free(tmpStr);
            free(LStr);
            free(RStr);

            AsmMove(ir, block, L, tmp);
            OperandFree(tmp);
        }
    }
    else
    {
        char* LStr = OperandToStr(L);
        char* RStr = OperandToStr(R);
        const char* OpStr = Op == BINOP_ADD ? "add" :
                            Op == BINOP_SUB ? "sub" :
                            Op == BINOP_MUL ? "imul" :
                            Op == BINOP_BITAND ? "and" :
                            Op == BINOP_BITOR ? "or" :
                            Op == BINOP_BITXOR ? "xor" :
                            Op == BINOP_SHR ? "sar" :
                            Op == BINOP_SHL ? "sal" : 0;

        if (OpStr) IrBlockOut(block, "%s %s, %s", OpStr, LStr, RStr);
        else printf("AsmBOP(): необработанный оператор, '%d'\n", Op);

        free(LStr);
        free(RStr);
    }
}

//деление
void AsmDivision (IrCTX* ir, IrBLOCK* block, Operand R)
{
    (void) ir;
    char* RStr = OperandToStr(R);
    
    IrBlockOut(block, "idiv %s", RStr);
    free(RStr);
}

//унарные операции
void AsmUOP (IrCTX* ir, IrBLOCK* block, UNARY_OPERATION Op, Operand R)
{
    (void) ir;
    char* RStr = OperandToStr(R);

    if (Op == UNARY_INC) IrBlockOut(block, "add %s, 1", RStr);
    else if (Op == UNARY_DEC) IrBlockOut(block, "sub %s, 1", RStr);
    else if (Op == UNARY_NEG || Op == UNARY_BITWISENOT) IrBlockOut(block, "%s %s", Op == UNARY_NEG ? "neg" : "not", RStr);
    else printf("AsmUOP(): необработанный оператор, %d", Op);

    free(RStr);
}

//комментарий
void AsmComment (AsmCTX* ctx, const char* str)
{
    AsmOutLn(ctx, ";%s", str);
}

void AsmFilePrologue (AsmCTX* ctx)
{
    AsmOutLn(ctx, ".file 1 \"%s\"", ctx->filename);
    AsmOutLn(ctx, ".intel_syntax noprefix");
}

void AsmFileEpilogue (AsmCTX* ctx)
{
    (void) ctx;
}

void AsmFnLinkageBegin (FILE* file, const char* name)
{
    fprintf(file, ".balign 16\n");
    fprintf(file, ".globl %s\n", name);
    fprintf(file, "%s:\n", name);
}

void AsmFnLinkageEnd (FILE* file, const char* name)
{
    (void) file, (void) name;
}

//подготовка стэка и сохранение регистров для вызова функции
void AsmFnPrologue (IrCTX* ir, IrBLOCK* block, int localSize)
{
    AsmCTX* ctx = ir->assem;

    AsmPush(ir, block, ctx->basePtr);
    AsmMove(ir, block, ctx->basePtr, ctx->stackPtr);

    if (localSize != 0)
        AsmBOP(ir, block, BINOP_SUB, ctx->stackPtr, OperandCreateLiteral(localSize));

    for (int i = 0; i < ctx->arch->calleeSaveRegs.length; i++)
    {
        REG_INDEX r = (REG_INDEX) VectorGet(&ctx->arch->calleeSaveRegs, i);
        AsmSaveReg(ir, block, r);
    }
}

//извлечение регистров из стека после завершения функции
void AsmFnEpilogue (IrCTX* ir, IrBLOCK* block)
{
    AsmCTX* ctx = ir->assem;

    for (int i = ctx->arch->calleeSaveRegs.length-1; i >= 0 ; i--)
    {
        REG_INDEX r = (REG_INDEX) VectorGet(&ctx->arch->calleeSaveRegs, i);
        AsmRestoreReg(ir, block, r);
    }

    AsmMove(ir, block, ctx->stackPtr, ctx->basePtr);
    AsmPop(ir, block, ctx->basePtr);
}

//внутрении функции
//проверка операнда на размещения в памяти
static int OperandIsMem (Operand L)
{
    return L.tag == OPERAND_MEM || L.tag == OPERAND_LABELMEM;
}

