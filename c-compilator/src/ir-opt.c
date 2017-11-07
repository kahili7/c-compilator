#include <stdlib.h>

#include "..\include\ir.h"
#include "..\include\hashmap.h"

static int UbrBlock (IrFN* fn, IrBLOCK* block)
{
    if (IrBlockGetPredNo(fn, block) == 0)
    {
        IrBlockDelete(fn, block);
        return 1;
    }

    return 0;
}

static int LbcBlock (IrFN* fn, IrBLOCK* block)
{
    IrBLOCK *pred = VectorGet(&block->preds, 0);

    if (block->preds.length == 1 && IrBlockGetSuccNo(pred) == 1)
    {
        IrBlocksCombine(fn, pred, block);
        return 1;
    }

    return 0;
}

static int BlaBlock (IrFN* fn, IntSet* done, IrBLOCK* block)
{
    if (IntSetAdd(done, (intptr_t) block)) return 0;

    //рекурсивный анализ предыдущих блоков
    for (int i = 0; i < block->preds.length; i++)
    {
        IrBLOCK* pred = VectorGet(&block->preds, i);
        int deleted = BlaBlock(fn, done, pred);

        if (deleted) i--;
    }

    return UbrBlock(fn, block) || LbcBlock(fn, block);
}

static void BlaFn (IrFN* fn)
{
    IntSet done;
    
    IntSetInit(&done, fn->blocks.length);
    BlaBlock(fn, &done, fn->epilogue);
    IntSetFree(&done);
}

void IrBlockLevelAnalysis (IrCTX* ctx)
{
    for (int i = 0; i < ctx->fns.length; i++)
    {
        IrFN* fn = VectorGet(&ctx->fns, i);
        BlaFn(fn);
    }
}