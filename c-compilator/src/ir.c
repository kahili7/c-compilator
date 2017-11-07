#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "..\include\ir.h"
#include "..\include\vector.h"
#include "..\include\debug.h"
#include "..\include\operand.h"
#include "..\include\asm.h"
#include "..\include\asm64.h"

//размеры векторов 
enum {
    IRCTX_FnNo = 8,
    IRCTX_DataNo = 8,
    IRCTX_RODataNo = 64,
    IRFN_BlockNo = 8,
    IRBLOCK_InstrNo = 8,
    IRBLOCK_StrSize = 1024,
    IRBLOCK_PredNo = 2,
    IRBLOCK_SuccNo = 2
};

//IR контекст
void IrInit (IrCTX* ctx, const char* output, const Arch* arch)
{
    VectorInit(&ctx->fns, IRCTX_FnNo);
    VectorInit(&ctx->data, IRCTX_DataNo);
    VectorInit(&ctx->rodata, IRCTX_RODataNo);

    ctx->labelNo = 0;

    ctx->assem = AsmInit(output, arch);
    ctx->arch = arch;
}

void IrFree (IrCTX* ctx)
{
    VectorFreeObjs(&ctx->fns, (VectorDtor) IrFnDestroy);
    VectorFreeObjs(&ctx->data, (VectorDtor) IrStaticDataDestroy);
    VectorFreeObjs(&ctx->rodata, (VectorDtor) IrStaticDataDestroy);
    AsmEnd(ctx->assem);
}

//внутренние функции
static void IrAddFn (IrCTX* ctx, IrFN* fn)
{
    VectorPush(&ctx->fns, fn);
}

static void IrAddData (IrCTX* ctx, IrSTATICDATA* data)
{
    VectorPush(&ctx->data, data);
}

static void IrAddROData (IrCTX* ctx, IrSTATICDATA* data)
{
    VectorPush(&ctx->rodata, data);
}

static char* IrCreateLabel (IrCTX* ctx)
{
    char* label = malloc(10);
    
    sprintf(label, ".%04X", ctx->labelNo++);
    return label;
}

//создание функции
IrFN* IrFnCreate (IrCTX* ctx, const char* name, int stacksize)
{
    IrFN* fn = malloc(sizeof(IrFN));
    
    fn->name = name ? strdup(name) : IrCreateLabel(ctx);
    VectorInit(&fn->blocks, IRFN_BlockNo);

    fn->prologue = IrBlockCreate(ctx, fn);
    fn->entryPoint = IrBlockCreate(ctx, fn);
    fn->epilogue = IrBlockCreate(ctx, fn);

    AsmFnPrologue(ctx, fn->prologue, stacksize);
    AsmFnEpilogue(ctx, fn->epilogue);

    IrJump(fn->prologue, fn->entryPoint);
    IrReturn(fn->epilogue);

    IrAddFn(ctx, fn);
    return fn;
}

static void IrFnDestroy (IrFN* fn)
{
    VectorFreeObjs(&fn->blocks, (VectorDtor) IrBlockDestroy);
    free(fn->name);
    free(fn);
}

//функции для вывода блока в поток
void IrBlockOut (IrBLOCK* block, const char* format, ...)
{
    va_list args[2];
    va_start(args[0], format);
    va_copy(args[1], args[0]);
    DebugVarMsg(format, args[1]);
    va_end(args[1]);

    int length = vsnprintf(block->str + block->length, block->capacity - block->length, format, args[0]);
    va_end(args[0]);

    if (length < 0 || block->length + length >= block->capacity)
    {
        block->capacity *= 2;
        block->capacity += length + 2;
        block->str = realloc(block->str, block->capacity);

        va_start(args[0], format);
        vsnprintf(block->str + block->length, block->capacity, format, args[0]);
        va_end(args[0]);
    }

    block->length += length;
    block->str[block->length++] = '\n';
}

//создание блока
IrBLOCK* IrBlockCreate (IrCTX* ctx, IrFN* fn)
{
    IrBLOCK* block = malloc(sizeof(IrBLOCK));
    
    VectorInit(&block->instrs, IRBLOCK_InstrNo);
    block->term = 0;
    block->label = IrCreateLabel(ctx);

    block->str = calloc(IRBLOCK_StrSize, sizeof(char*));
    block->length = 0;
    block->capacity = IRBLOCK_StrSize;

    VectorInit(&block->preds, IRBLOCK_PredNo);
    VectorInit(&block->succs, IRBLOCK_SuccNo);

    IrAddBlock(fn, block);
    return block;
}

//удаление блока
void IrBlockDelete (IrFN* fn, IrBLOCK* block)
{
    /*удалить элемент из FN вектора*/
    IrBLOCK* replacement = VectorRemoveReorder(&fn->blocks, block->nthChild);
    
    replacement->nthChild = block->nthChild;

    /*удалить элементы из preds и succs*/
    for (int i = 0; i < block->preds.length; i++)
    {
        IrBLOCK* pred = VectorGet(&block->preds, i);
        int index = VectorFind(&pred->succs, block);
        
        VectorRemoveReorder(&pred->succs, index);
    }

    for (int i = 0; i < block->succs.length; i++)
    {
        IrBLOCK* succ = VectorGet(&block->succs, i);
        int index = VectorFind(&succ->preds, block);
        
        VectorRemoveReorder(&succ->preds, index);
    }

    IrBlockDestroy(block);
}

//объединение их, переместить из успешных в заранее и удалить успешные
void IrBlocksCombine (IrFN* fn, IrBLOCK* pred, IrBLOCK* succ)
{
    //succ -> pred
    VectorPushFromVector(&pred->instrs, &succ->instrs);

    //освободить pred терминал, и взять succ
    IrTermDestroy(pred->term);
    pred->term = succ->term;
    succ->term = 0;
    
    //объединить строки
    int totalLength = pred->length + succ->length - 1;

    if (pred->capacity >= totalLength)
    {
        strncat(pred->str, succ->str, pred->capacity);
        pred->length = totalLength;

    }
    else
    {
        char* str = malloc(totalLength);
        
        snprintf(str, totalLength, "%s%s", pred->str, succ->str);
        free(pred->str);
        pred->str = str;
    }

    //ссылка на succ из succ
    for (int i = 0; i < succ->succs.length; i++)
        IrBlockLink(pred, VectorGet(&succ->succs, i));

    if (fn->epilogue == succ)
        fn->epilogue = pred;

    IrBlockDelete(fn, succ);
}

int IrBlockGetPredNo (IrFN* fn, IrBLOCK* block)
{
    return block->preds.length + (block == fn->prologue ? 1 : 0);
}

int IrBlockGetSuccNo (IrBLOCK* block)
{
    return block->succs.length + (block->term->tag == TERM_CALL || block->term->tag == TERM_CALLINDIRECT ? 1 : 0);
}

//внутренние функции
static void IrBlockDestroy (IrBLOCK* block)
{
    VectorFree(&block->preds);
    VectorFree(&block->succs);

    VectorFreeObjs(&block->instrs, (VectorDtor) IrInstrDestroy);
    IrTermDestroy(block->term);

    free(block->label);
    free(block->str);
    free(block);
}

static void IrAddBlock (IrFN* fn, IrBLOCK* block)
{
    block->nthChild = VectorPush(&fn->blocks, block);
}

static void IrBlockLink (IrBLOCK* from, IrBLOCK* to)
{
    VectorPush(&from->succs, to);
    VectorPush(&to->preds, from);
}

//принудительное прерывание блока если его нет
static void IrBlockTerminate (IrBLOCK* block, IrTERM* term)
{
    if (block->term)
    {
        DebugError("IrBlockTerminate", "attempted to terminate already terminated block");
        IrTermDestroy(block->term);
    }

    block->term = term;
}

//функции для терминальных инструкций
void IrJump (IrBLOCK* block, IrBLOCK* to)
{
    IrTERM* term = IrTermCreate(TERM_JUMP, block);
    
    term->to = to;
    IrBlockLink(block, to);
}

void IrBranch (IrBLOCK* block, Operand cond, IrBLOCK* ifTrue, IrBLOCK* ifFalse)
{
    IrTERM* term = IrTermCreate(TERM_BRANCH, block);
    
    term->cond = cond;
    term->ifTrue = ifTrue;
    term->ifFalse = ifFalse;

    IrBlockLink(block, ifTrue);
    IrBlockLink(block, ifFalse);
}

void IrCall (IrBLOCK* block, Symbol* to, IrBLOCK* ret)
{
    IrTERM* term = IrTermCreate(TERM_CALL, block);
    
    term->toAsSym = to;
    term->ret = ret;

    IrBlockLink(block, ret);
}

void IrCallIndirect (IrBLOCK* block, Operand to, IrBLOCK* ret)
{
    IrTERM* term = IrTermCreate(TERM_CALLINDIRECT, block);
    
    term->toAsOperand = to;
    term->ret = ret;

    IrBlockLink(block, ret);
    AsmCallIndirect(block, to);
}

static void IrReturn (IrBLOCK* block)
{
    IrTermCreate(TERM_RETURN, block);
}

static IrTERM* IrTermCreate (TERM_TAG tag, IrBLOCK* block)
{
    IrTERM* term = malloc(sizeof(IrTERM));
    
    term->tag = tag;
    term->to = 0;
    term->cond = OperandCreate(OPERAND_UNDEFINED);
    term->ifTrue = 0;
    term->ifFalse = 0;
    term->ret = 0;
    term->toAsSym = 0;
    term->toAsOperand = OperandCreate(OPERAND_UNDEFINED);
    
    IrBlockTerminate(block, term);
    return term;
}

static void IrTermDestroy (IrTERM* term)
{
    free(term);
}

static IrINSTR* IrInstrCreate (INSTR_TAG tag, IrBLOCK* block)
{
    IrINSTR* instr = malloc(sizeof(IrINSTR));
    
    instr->tag = tag;
    instr->op = OP_UNDEFINED;
    instr->dest = OperandCreate(OPERAND_UNDEFINED);
    instr->l = OperandCreate(OPERAND_UNDEFINED);
    instr->r = OperandCreate(OPERAND_UNDEFINED);

    IrAddInstr(block, instr);
    return instr;
}

static void IrInstrDestroy (IrINSTR* instr)
{
    (void) IrInstrCreate;
    free(instr);
}

static void IrAddInstr (IrBLOCK* block, IrINSTR* instr)
{
    VectorPush(&block->instrs, instr);
}

//статические данные
void IrStaticValue (IrCTX* ctx, const char* label, int global, int size, intptr_t initial)
{
    IrSTATICDATA* data = IrStaticDataCreate(ctx, 0, STATICDATA_REGULAR);
    
    data->label = label;
    data->global = global;
    data->size = size;
    data->initial = initial;
}

Operand IrStringConstant (IrCTX* ctx, const char* str)
{
    IrSTATICDATA* data = IrStaticDataCreate(ctx, 1, STATICDATA_STRINGCONSTANT);
    
    data->strlabel = IrCreateLabel(ctx);
    data->str = (void*) strdup(str);
    return OperandCreateLabelOffset(data->label);
}

//статические данные - внутренние функции
static IrSTATICDATA* IrStaticDataCreate (IrCTX* ctx, int ro, STATICDATA_TAG tag)
{
    IrSTATICDATA* data = malloc(sizeof(IrSTATICDATA));
    
    data->tag = tag;
    (ro ? IrAddROData : IrAddData)(ctx, data);
    return data;
}

static void IrStaticDataDestroy (IrSTATICDATA* data)
{
    if (data->tag == STATICDATA_STRINGCONSTANT)
    {
        free(data->strlabel);
        free(data->str);
    }

    free(data);
}