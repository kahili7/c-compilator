#ifndef X_INCLUDE_IR
#define X_INCLUDE_IR

#include "..\include\ast.h"
#include "..\include\operand.h"
#include "..\include\vector.h"

//промежуточный код

typedef enum INSTR_TAG {
    INSTR_UNDEFINED,
    INSTR_MOVE,
    INSTR_BOP,
    INSTR_UOP
} INSTR_TAG;

typedef enum TERM_TAG {
    TERM_UNDEFINED,
    TERM_JUMP,
    TERM_BRANCH,
    TERM_CALL,
    TERM_CALLINDIRECT,
    TERM_RETURN
} TERM_TAG;

typedef struct IrINSTR {
    INSTR_TAG tag;      //тип инструкции
    OP_TAG op;          //тип операнда

    Operand dest;
    Operand l;
    Operand r;
} IrINSTR;

typedef struct IrTERM {
    TERM_TAG tag;

    union {
        IrBLOCK* to;    //для jmp
        
        //для ветвления
        struct {
            IrBLOCK* ifTrue;
            IrBLOCK* ifFalse;
            Operand cond;
        };
        
        //для вызова функции
        struct {
            IrBLOCK* ret;
            
            union {
                /*termCall*/
                Symbol* toAsSym;
                /*termCallIndirect*/
                Operand toAsOperand;
            };
        };
    };
} IrTERM;

typedef enum STATICDATA_TAG {
    STATICDATA_UNDEFINED,
    STATICDATA_REGULAR,
    STATICDATA_STRINGCONSTANT
} STATICDATA_TAG;

typedef struct IrSTATICDATA {
    STATICDATA_TAG tag;

    union {
        /*dataRegular*/
        struct {
            const char* label;
            int global;
            int size;
            intptr_t initial;
        };
        /*dataStringConstant*/
        struct {
            char* strlabel;
            char* str;
        };
    };
} IrSTATICDATA;

typedef struct IrBLOCK {
    Vector instrs;
    IrTERM* term;

    char* label;

    char* str;      //строка с командами ассемблера заканчивающиеся \n
    int length;     //длина на которую увеличивается строка
    int capacity;   //емкость строки

    int nthChild;   //индекс родительского FN вектора

    ///Blocks that this block may (at runtime) have (directly)
    ///come from / go to, respectively
    Vector preds;
    Vector succs;
} IrBLOCK;

//блок описывающий фукцию
typedef struct IrFN {
    char* name;
    ///prologue and epilogue manage the stack frame and register saving
    ///created and managed internally. entryPoint is what the emitter should
    ///fill, using epilogue as a continuation / return point
    IrBLOCK* prologue;
    IrBLOCK* entryPoint;
    IrBLOCK* epilogue;
    ///Includes and owns the above blocks, as well as all others
    Vector blocks;  //вектор блоков
} IrFN;

//промежуточное представление
//контекст
typedef struct IrCTX {
    Vector fns;
    Vector data;
    Vector rodata;
    
    int labelNo;

    AsmCTX* assem;      //контекст ассемблера
    const Arch* arch;   //архитектурные данные
} IrCTX;

void IrInit (IrCTX* ctx, const char* output, const Arch* arch);
void IrFree (IrCTX* ctx);

void IrEmit (IrCTX* ctx);
IrFN* IrFnCreate (IrCTX* ctx, const char* name, int stacksize);

void IrBlockOut (IrBLOCK* block, const char* format, ...);
IrBLOCK* IrBlockCreate (IrCTX* ctx, IrFN* fn);
void IrBlockDelete (IrFN* fn, IrBLOCK* block);

void IrStaticValue (IrCTX* ctx, const char* label, int global, int size, intptr_t initial);
Operand IrStringConstant (IrCTX* ctx, const char* str);

void IrJump (IrBLOCK* block, IrBLOCK* to);
void IrBranch (IrBLOCK* block, Operand cond, IrBLOCK* ifTrue, IrBLOCK* ifFalse);
void IrCall (IrBLOCK* block, Symbol* to, IrBLOCK* ret);
void IrCallIndirect (IrBLOCK* block, Operand to, IrBLOCK* ret);

int IrBlockGetPredNo (IrFN* fn, IrBLOCK* block);
int IrBlockGetSuccNo (IrBLOCK* block);

#endif /*X_INCLUDE_IR*/