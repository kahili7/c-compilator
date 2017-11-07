#include <stdlib.h>
#include <stdarg.h>

#include "..\include\ir.h"
#include "..\include\vector.h"
#include "..\include\hashmap.h"
#include "..\include\debug.h"
#include "..\include\operand.h"
#include "..\include\asm.h"
#include "..\include\asm64.h"

//внутренние функции
static void IrEmitBlock (IrCTX* ctx, FILE* file, const IrBLOCK* prevblock, const IrBLOCK* block, const IrBLOCK* nextblock)
{
    DebugEnter(block->label);

    if (!(block->preds.length <= 1 && (block->preds.length == 1 ? VectorGet(&block->preds, 0) == prevblock : 1)))
        AsmLabel(ctx->assem, block->label);

    fputs(block->str, file);
    DebugMsg(block->str);

    if (block->term) IrEmitTerm(ctx, file, block->term, nextblock);
    else
        DebugError("IrEmitBlock", "незакрытый блок %s", block->label);

    fputs("\n", file);
    DebugLeave();
}

static void IrEmitBlockChain (IrCTX* ctx, FILE* file, IntSet* done, Vector* priority, const IrBLOCK* block)
{
    /*Already put in the priority list, leave*/
    if (IntSetAdd(done, (intptr_t) block)) return;

    /*Add all the predecessors and their predecessors to the list*/
    for (int j = 0; j < block->preds.length; j++)
    {
        IrBLOCK* pred = VectorGet(&block->preds, j);
        
        IrEmitBlockChain(ctx, file, done, priority, pred);
    }

    /*Followed by this block*/
    VectorPush(priority, (void*) block);
}

static void IrEmitTerm (IrCTX* ctx, FILE* file, const IrTERM* term, const IrBLOCK* nextblock)
{
    (void) file;

    IrBLOCK* jumpTo = 0;

    if (term->tag == TERM_JUMP) jumpTo = term->to;
    else if (term->tag == TERM_BRANCH)
    {
        if (term->ifTrue == nextblock)
        {
            Operand cond = OperandCreateFlags(ConditionNegate(term->cond.condition));
            
            AsmBranch(ctx->assem, cond, term->ifFalse->label);
            jumpTo = term->ifTrue;
        }
        else
        {
            AsmBranch(ctx->assem, term->cond, term->ifTrue->label);
            jumpTo = term->ifFalse;
        }
    }
    else if (term->tag == TERM_CALL)
    {
        AsmCall(ctx->assem, term->toAsSym->label);
        jumpTo = term->ret;
    }
    else if (term->tag == TERM_CALLINDIRECT)
    {
        //asmCallIndirect(ctx->asm, term->toAsOperand);
        jumpTo = term->ret;
    }
    else if (term->tag == TERM_RETURN) AsmReturn(ctx->assem);
    else DebugErrorUnhandledInt("IrEmitTerm", "terminal tag", term->tag);

    /*выполнить прыжок, если он не дублирующий*/
    if (jumpTo && jumpTo != nextblock)
        AsmJump(ctx->assem, jumpTo->label);
}

static void IrEmitFn (IrCTX* ctx, FILE* file, const IrFN* fn)
{
    DebugEnter(fn->name);

    IntSet done;
    IntSetInit(&done, fn->blocks.length * 2);

    Vector priority;
    VectorInit(&priority, fn->blocks.length);

    /*Decide an order to emit the blocks in to minimize unnecessary jumps*/
    IrEmitBlockChain(ctx, file, &done, &priority, fn->epilogue);

    /*Emit*/
    AsmFnLinkageBegin(file, fn->name);

    for (int j = 0; j < priority.length; j++)
    {
        IrBLOCK *prevblock = VectorGet(&priority, j - 1);
        IrBLOCK *block = VectorGet(&priority, j);
        IrBLOCK *nextblock = VectorGet(&priority, j + 1);
        
        IrEmitBlock(ctx, file, prevblock, block, nextblock);
    }

    AsmFnLinkageEnd(file, fn->name);

    VectorFree(&priority);
    IntSetFree(&done);
    DebugLeave();
}

static void IrEmitStaticData (IrCTX* ctx, FILE* file, const IrSTATICDATA* data)
{
    (void) file;

    if (data->tag == STATICDATA_REGULAR)
        AsmStaticData(ctx->asm, data->label, data->global, data->size, data->initial);

    else if (data->tag == STATICDATA_STRINGCONSTANT)
        AsmStringConstant(ctx->asm, data->strlabel, data->str);

    else
        DebugErrorUnhandledInt("IrEmitStaticData", "static data tag", data->tag);
}

//создание функций и данных
void IrEmit (IrCTX* ctx)
{
    FILE* file = ctx->assem->file;

    AsmFilePrologue(ctx->assem);

    for (int i = 0; i < ctx->fns.length; i++)
    {
        IrFN* fn = VectorGet(&ctx->fns, i);
        
        IrEmitFn(ctx, file, fn);
    }

    AsmDataSection(ctx->assem);

    for (int i = 0; i < ctx->data.length; i++)
    {
        IrSTATICDATA* data = VectorGet(&ctx->data, i);
        
        IrEmitStaticData(ctx, file, data);
    }

    AsmRODataSection(ctx->assem);

    for (int i = 0; i < ctx->rodata.length; i++)
    {
        IrSTATICDATA* data = VectorGet(&ctx->rodata, i);
        
        IrEmitStaticData(ctx, file, data);
    }

    AsmFileEpilogue(ctx->assem);
}