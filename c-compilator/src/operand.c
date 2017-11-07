#include "..\include\operand.h"
#include "..\include\arch.h"
#include "..\include\util.h"

#include <assert.h>

Operand OperandCreate (OPERAND_TAG tag)
{
    Operand ret;
    
    ret.tag = tag;
    ret.base = 0;
    ret.index = 0;
    ret.factor = 0;
    ret.offset = 0;
    ret.literal = 0;
    ret.condition = CONDITION_TAG;
    ret.label = 0;
    ret.array = 0;
    ret.size = 0;
    return ret;
}

Operand OperandCreateInvalid ()
{
    return OperandCreate(OPERAND_INVALID);
}

Operand OperandCreateVoid ()
{
    return OperandCreate(OPERAND_VOID);
}

Operand OperandCreateFlags (CONDITION_TAG cond)
{
    Operand ret = OperandCreate(OPERAND_FLAGS);
    
    ret.condition = cond;
    return ret;
}

Operand OperandCreateReg (Register* r)
{
    Operand ret = OperandCreate(OPERAND_REG);
    
    ret.base = r;
    return ret;
}

Operand OperandCreateMem (Register* base, int offset, int size)
{
    Operand ret = OperandCreate(OPERAND_MEM);
    
    ret.base = base;
    ret.index = 0;
    ret.factor = 0;
    ret.offset = offset;
    ret.size = size;
    return ret;
}

Operand OperandCreateLiteral (int literal)
{
    Operand ret = OperandCreate(OPERAND_LITERAL);
    
    ret.literal = literal;
    return ret;
}

Operand OperandCreateLabel (const char* label)
{
    Operand ret = OperandCreate(OPERAND_LABEL);
    ret.label = label;
    return ret;
}

Operand OperandCreateLabelMem (const char* label, int size)
{
    Operand ret = OperandCreate(OPERAND_LABELMEM);
    ret.label = label;
    ret.size = size;
    return ret;
}

Operand OperandCreateLabelOffset (const char* label)
{
    Operand ret = OperandCreate(OPERAND_LABELOFFSET);
    ret.label = label;
    return ret;
}

void OperandFree (Operand Value)
{
    if (Value.tag == OPERAND_REG)
    {
        RegFree(Value.base);
        Value.base = 0;
    }
    else if (Value.tag == OPERAND_MEM)
    {
        if (Value.base != 0 && Value.base != RegGet(REG_RBP))
        {
            RegFree(Value.base);
            Value.base = 0;
        }

        if (Value.index != 0)
        {
            RegFree(Value.index);
            Value.index = 0;
        }
    }
    else if (Value.tag == OPERAND_UNDEFINED || Value.tag == OPERAND_INVALID
               || Value.tag == OPERAND_VOID || Value.tag == OPERAND_FLAGS
               || Value.tag == OPERAND_LITERAL || Value.tag == OPERAND_LABEL
               || Value.tag == OPERAND_LABELMEM || Value.tag == OPERAND_LABELOFFSET
               || Value.tag == OPERAND_STACK)
        /*Nothing to do*/;
    else
        DebugErrorUnhandled("OperandFree", "operand tag", OperandTagGetStr(Value.tag));
}

int OperandIsEqual (Operand L, Operand R)
{
    if (L.tag != R.tag) return 0;
    else if (L.tag == OPERAND_FLAGS) return L.condition == R.condition;
    else if (L.tag == OPERAND_REG) return L.base == R.base;
    else if (L.tag == OPERAND_MEM)
        return L.size == R.size && L.base == R.base
               && L.index == R.index && L.factor == R.factor
               && L.offset == R.offset;

    else if (L.tag == OPERAND_LITERAL) return L.literal == R.literal;
    else if (L.tag == OPERAND_LABEL || L.tag == OPERAND_LABELMEM || L.tag == OPERAND_LABELOFFSET) return L.label == R.label;
    else
    {
        assert(L.tag == OPERAND_VOID || L.tag == OPERAND_STACK || L.tag == OPERAND_INVALID || L.tag == OPERAND_UNDEFINED);
        return 1;
    }
}

int OperandGetSize (const Arch* arch, Operand Value)
{
    if (Value.tag == OPERAND_UNDEFINED || Value.tag == OPERAND_INVALID || Value.tag == OPERAND_VOID) return 0;
    else if (Value.tag == OPERAND_REG) return Value.base->allocatedAs;
    else if (Value.tag == OPERAND_MEM || Value.tag == OPERAND_LABELMEM) return Value.size;
    else if (Value.tag == OPERAND_LITERAL) return 1;
    else if (Value.tag == OPERAND_LABEL || Value.tag == OPERAND_LABELOFFSET || Value.tag == OPERAND_FLAGS) return arch->wordsize;
    else DebugErrorUnhandled("OperandGetSize", "operand tag", OperandTagGetStr(Value.tag));

    return 0;
}

char* OperandToStr (Operand Value)
{
    if (Value.tag == OPERAND_UNDEFINED) return strdup("<undefined>");
    else if (Value.tag == OPERAND_INVALID) return strdup("<invalid>");
    else if (Value.tag == OPERAND_VOID) return strdup("<void>");
    else if (Value.tag == OPERAND_FLAGS)
    {
        const char* conditions[7] = {"condition", "e", "ne", "g", "ge", "l", "le"};
        
        return strdup(conditions[Value.condition]);
    }
    else if (Value.tag == OPERAND_REG) return strdup(RegGetStr(Value.base));

    else if (Value.tag == OPERAND_MEM || Value.tag == OPERAND_LABELMEM)
    {
        const char* sizeStr;

        if (Value.size == 1) sizeStr = "byte";
        else if (Value.size == 2) sizeStr = "word";
        else if (Value.size == 4) sizeStr = "dword";
        else if (Value.size == 8) sizeStr = "qword";
        else if (Value.size == 16) sizeStr = "oword";
        else sizeStr = "dword";

        if (Value.tag == OPERAND_LABELMEM)
        {
            char* ret = malloc(strlen(sizeStr) + strlen(Value.label) + 9);
            
            sprintf(ret, "%s ptr [%s]", sizeStr, Value.label);
            return ret;
        }
        else if (Value.index == REG_UNDEFINED || Value.factor == 0)
        {
            if (Value.offset == 0)
            {
                const char* regStr = RegGetStr(Value.base);
                char* ret = malloc(strlen(sizeStr) + strlen(regStr) + 9);
                
                sprintf(ret, "%s ptr [%s]", sizeStr, regStr);
                return ret;

            }
            else
            {
                const char* regStr = RegGetStr(Value.base);
                char* ret = malloc(strlen(sizeStr) + strlen(regStr) + LogI(Value.offset, 10) + 3 + 10);
                
                sprintf(ret, "%s ptr [%s%+d]", sizeStr, regStr, Value.offset);
                return ret;
            }
        }
        else
        {
            const char* regStr = RegGetStr(Value.base);
            const char* indexStr = RegGetStr(Value.index);
            char* ret = malloc(strlen(sizeStr)
                               + strlen(regStr)
                               + LogI(Value.factor, 10) + 3
                               + strlen(indexStr)
                               + LogI(Value.offset, 10) + 3 + 10);
            
            sprintf(ret, "%s ptr [%s%+d*%s%+d]", sizeStr, regStr, Value.factor, indexStr, Value.offset);
            return ret;
        }
    }
    else if (Value.tag == OPERAND_LITERAL)
    {
        char* ret = malloc(LogI(Value.literal, 10) + 3);
        
        sprintf(ret, "%d", Value.literal);
        return ret;
    }
    else if (Value.tag == OPERAND_LABEL || Value.tag == OPERAND_LABELOFFSET)
    {
        char* ret = malloc(strlen(Value.label) + 8);
        
        sprintf(ret, "offset %s", Value.label);
        return ret;
    }
    else
    {
        DebugErrorUnhandled("OperandToStr", "operand tag", OperandTagGetStr(Value.tag));
        return strdup("<unhandled>");
    }
}

const char* OperandTagGetStr (OPERAND_TAG tag)
{
    if (tag == OPERAND_UNDEFINED) return "OPERAND_UNDEFINED";
    else if (tag == OPERAND_INVALID) return "OPERAND_INVALID";
    else if (tag == OPERAND_VOID) return "OPERAND_VOID";
    else if (tag == OPERAND_FLAGS) return "OPERAND_FLAGS";
    else if (tag == OPERAND_REG) return "OPERAND_REG";
    else if (tag == OPERAND_MEM) return "OPERAND_MEM";
    else if (tag == OPERAND_LITERAL) return "OPERAND_LITERAL";
    else if (tag == OPERAND_LABEL) return "OPERAND_LABEL";
    else if (tag == OPERAND_LABELMEM) return "OPERAND_LABELMEM";
    else if (tag == OPERAND_LABELOFFSET) return "OPERAND_LABELOFFSET";
    else if (tag == OPERAND_STACK) return "OPERAND_STACK";
    else
    {
        char* str = malloc(LogI(tag, 10) + 2);
        
        sprintf(str, "%d", tag);
        DebugErrorUnhandled("OperandTagGetStr", "operand tag", str);
        free(str);
        return "<unhandled>";
    }
}

CONDITION_TAG ConditionFromOp (OP_TAG cond)
{
    if (cond == OP_EQ) return CONDITION_EQ;
    else if (cond == OP_NEQ) return CONDITION_NE;
    else if (cond == OP_GT) return CONDITION_GT;
    else if (cond == OP_GE) return CONDITION_GE;
    else if (cond == OP_LT) return CONDITION_LT;
    else if (cond == OP_LE) return CONDITION_LE;
    else return CONDITION_UNDEFINED;
}

CONDITION_TAG ConditionNegate (CONDITION_TAG cond)
{
    if (cond == CONDITION_EQ) return CONDITION_NE;
    else if (cond == CONDITION_NE) return CONDITION_EQ;
    else if (cond == CONDITION_GT) return CONDITION_LE;
    else if (cond == CONDITION_GE) return CONDITION_LT;
    else if (cond == CONDITION_LT) return CONDITION_GE;
    else if (cond == CONDITION_LE) return CONDITION_GT;
    else return CONDITION_UNDEFINED;
}
