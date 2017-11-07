#ifndef X_INCLUDE_OPERAND
#define X_INCLUDE_OPERAND

#include "..\include\register.h"

typedef enum OPERAND_TAG {
    OPERAND_UNDEFINED,
    OPERAND_INVALID,
    OPERAND_VOID,
    OPERAND_FLAGS,
    OPERAND_REG,
    OPERAND_MEM,
    OPERAND_LITERAL,
    OPERAND_LABEL,
    OPERAND_LABELMEM,
    OPERAND_LABELOFFSET,
    OPERAND_STACK
} OPERAND_TAG;

typedef enum CONDITION_TAG {
    CONDITION_UNDEFINED,
    CONDITION_EQ,
    CONDITION_NE,
    CONDITION_GT,
    CONDITION_GE,
    CONDITION_LT,
    CONDITION_LE
} CONDITION_TAG;

typedef struct Operand {
    OPERAND_TAG tag;

    /*OPERAND_MEM: size ptr [base + index*factor + offset]*/

    union {
        struct {
            /*OPERAND_REG OPERAND_MEM*/
            Register* base; 
            /*OPERAND_MEM*/
            Register* index;
            int factor;
            int offset;
        };
        
        /*operandLiteral*/
        int literal;
        /*operandFlags*/
        CONDITION_TAG condition;
        /*operandLabel operandLabelMem operandLabelOffset*/
        const char* label;
    };

    int size;   //размер в байтах для операндов в памяти
    int array;
} Operand;


Operand OperandCreate (OPERAND_TAG tag);
Operand OperandCreateInvalid ();
Operand OperandCreateVoid ();
Operand OperandCreateFlags (CONDITION_TAG cond);
Operand OperandCreateReg (Register* r);
Operand OperandCreateMem (Register* base, int offset, int size);
Operand OperandCreateLiteral (int literal);
Operand OperandCreateLabel (const char* label);
Operand OperandCreateLabelMem (const char* label, int size);
Operand OperandCreateLabelOffset (const char* label);
void OperandFree (Operand Value);

int OperandIsEqual (Operand L, Operand R);
int OperandGetSize (const Arch* arch, Operand Value);
char* OperandToStr (Operand Value);
const char* OperandTagGetStr (OPERAND_TAG tag);

CONDITION_TAG ConditionFromOp (OP_TAG cond);
CONDITION_TAG ConditionNegate (CONDITION_TAG cond);
#endif /*X_INCLUDE_OPERAND*/