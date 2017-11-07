#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "..\include\type.h"
#include "..\include\symbol.h"
#include "..\include\arch.h"
#include "..\include\debug.h"

Type* TypeCreateBasic (const Symbol* basic)
{
    Type* dt = TypeCreate(TYPE_BASIC);
    dt->basic = basic;
    return dt;
}

Type* TypeCreatePtr (Type* base)
{
    Type* dt = TypeCreate(TYPE_PTR);
    dt->base = base;
    return dt;
}

Type* TypeCreateArray (Type* base, int size)
{
    Type* dt = TypeCreate(TYPE_ARRAY);
    dt->base = base;
    dt->array = size;
    return dt;
}

Type* TypeCreateFunction (Type* returnType, Type** paramTypes, int params, int variadic)
{
    Type* dt = TypeCreate(TYPE_FUNCTION);
    dt->returnType = returnType;
    dt->paramTypes = paramTypes;
    dt->params = params;
    dt->variadic = variadic;
    return dt;
}

Type* TypeCreateInvalid ()
{
    return TypeCreate(TYPE_INVALID);
}

void TypeDestroy (Type* dt)
{
    if (dt->tag == TYPE_BASIC || dt->tag == TYPE_INVALID) ;
    else if (dt->tag == TYPE_PTR || dt->tag == TYPE_ARRAY) TypeDestroy(dt->base);
    else if (dt->tag == TYPE_FUNCTION)
    {
        TypeDestroy(dt->returnType);

        for (int i = 0; i < dt->params; i++)
            TypeDestroy(dt->paramTypes[i]);

        free(dt->paramTypes);
    }

    free(dt);
}

Type* TypeDeepDuplicate (const Type* dt)
{
    Type* copy;

    if (dt->tag == TYPE_INVALID) copy = TypeCreateInvalid();
    else if (dt->tag == TYPE_BASIC) copy = TypeCreateBasic(dt->basic);
    else if (dt->tag == TYPE_PTR) copy = TypeCreatePtr(TypeDeepDuplicate(dt->base));
    else if (dt->tag == TYPE_ARRAY) copy = TypeCreateArray(TypeDeepDuplicate(dt->base), dt->array);
    else if (dt->tag == TYPE_FUNCTION)
    {
        Type** paramTypes = calloc(dt->params, sizeof(Type*));

        for (int i = 0; i < dt->params; i++)
            paramTypes[i] = TypeDeepDuplicate(dt->paramTypes[i]);

        copy = TypeCreateFunction(TypeDeepDuplicate(dt->returnType), paramTypes, dt->params, dt->variadic);
    }
    else
    {
        DebugErrorUnhandled("typeDeepDuplicate", "type tag", TypeTagGetStr(dt->tag));
        copy = TypeCreateInvalid();
    }

    copy->qual = dt->qual;
    return copy;
}

//вывод типа
Type* TypeDeriveFrom (const Type* dt)
{
    Type* New = TypeDeepDuplicate(dt);
    return New;
}

Type* TypeDeriveFromTwo (const Type* L, const Type* R)
{
    if (TypeIsInvalid(L)) return TypeDeepDuplicate(R);
    else if (TypeIsInvalid(R)) return TypeDeepDuplicate(L);
    else
    {
        assert(TypeIsCompatible(L, R));
        return TypeDeriveFrom(L);
    }
}

Type* TypeDeriveUnified (const Type* L, const Type* R)
{
    if (TypeIsInvalid(L)) return TypeDeepDuplicate(R);
    else if (TypeIsInvalid(R)) return TypeDeepDuplicate(L);
    else
    {
        assert(TypeIsCompatible(L, R));

        if (TypeIsEqual(L, R)) return TypeDeepDuplicate(L); //== R
        else return TypeDeriveFromTwo(L, R);
    }
}

Type* TypeDeriveBase (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);

    if (TypeIsInvalid(dt) || DebugAssert("typeDeriveBase", "base", TypeIsPtr(dt) || TypeIsArray(dt)))
        return TypeCreateInvalid();
    else
        return TypeDeepDuplicate(dt->base);
}

Type* TypeDerivePtr (const Type* base)
{
    return TypeCreatePtr(TypeDeepDuplicate(base));
}

Type* TypeDeriveArray (const Type* base, int size)
{
    return TypeCreateArray(TypeDeepDuplicate(base), size);
}

Type* TypeDeriveReturn (const Type* fn)
{
    fn = TypeGetCallable(fn);

    if (DebugAssert("typeDeriveReturn", "callable param", fn != 0)) return TypeCreateInvalid();
    else return TypeDeepDuplicate(fn->returnType);
}

// вспомогательные функции
const char* TypeTagGetStr (TYPE_TAG tag)
{
    if (tag == TYPE_BASIC) return "typeBasic";
    else if (tag == TYPE_PTR) return "typePtr";
    else if (tag == TYPE_ARRAY) return "typeArray";
    else if (tag == TYPE_FUNCTION) return "typeFunction";
    else if (tag == TYPE_INVALID) return "typeInvalid";
    else
    {
        char* str = malloc(LogI(tag, 10) + 2);
        
        sprintf(str, "%d", tag);
        DebugErrorUnhandled("typeTagGetStr", "type tag", str);
        free(str);
        return "<unhandled>";
    }
}

int TypeGetSize (const Arch* arch, const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);

    if (TypeIsInvalid(dt)) return 0;
    else if (TypeIsArray(dt))
    {
        if (dt->array < 0) return 0;
        else return dt->array * TypeGetSize(arch, dt->base);
    }
    else if (TypeIsPtr(dt) || TypeIsFunction(dt)) return arch->wordsize;
    else
    {
        assert(TypeIsBasic(dt));
        return dt->basic->size;
    }
}

char* TypeToStr (const Type* dt)
{
    return TypeToStrEmbed(dt, "");
}

char* TypeToStrEmbed (const Type* dt, const char* embedded)
{
    /*базовый или неверный тип*/
    if (dt->tag == TYPE_INVALID || dt->tag == TYPE_BASIC)
    {
        char* basicStr = TypeIsInvalid(dt)
                            ? "<invalid>"
                            : (dt->basic->ident && dt->basic->ident[0]) 
                                ? dt->basic->ident
                                : dt->basic->tag == SYMBOL_STRUCT ? "<unnamed struct>" :
                                  dt->basic->tag == SYMBOL_UNION ? "<unnamed union>" :
                                  dt->basic->tag == SYMBOL_ENUM ? "<unnamed enum>" : "<unnamed type>";

        char* qualified = TypeQualifiersToStr(dt->qual, basicStr);

        if (embedded[0] == (char)0) return qualified;
        else
        {
            char* ret = malloc(strlen(embedded) + strlen(basicStr) + 2 + (dt->qual.isConst ? 6 : 0));
            
            sprintf(ret, "%s %s", qualified, embedded);
            free(qualified);
            return ret;
        }
    /*функция*/
    }
    else if (dt->tag == TYPE_FUNCTION)
    {
        char* params = 0;

        if (dt->params == 0)
            params = strdup("void");
        else
        {
            Vector paramStrs;
            VectorInit(&paramStrs, dt->params + 1);
            VectorPushFromArray(&paramStrs, (void**) dt->paramTypes, dt->params, sizeof(Type*));
            VectorMap(&paramStrs, (VectorMapper) TypeToStr, &paramStrs);

            if (dt->variadic) VectorPush(&paramStrs, "...");

            params = StrJoinWith((char**) paramStrs.buffer, paramStrs.length, ", ", malloc);

            if (dt->variadic) paramStrs.length--;

            VectorFreeObjs(&paramStrs, free);
        }

        char* format = malloc(strlen(embedded) + 2 + strlen(params) + 3);

        if (!embedded[0])
            sprintf(format, "()(%s)", params);
        else
            sprintf(format, "%s(%s)", embedded, params);

        free(params);
        
        char* ret = TypeToStrEmbed(dt->returnType, format);
        
        free(format);
        return ret;
    /*массив и указатель*/
    }
    else
    {
        char* format;

        if (TypeIsPtr(dt))
        {
            format = malloc(strlen(embedded) + 4 + (dt->qual.isConst ? 7 : 0));

            char* qualified = TypeQualifiersToStr(dt->qual, embedded);

            if (dt->base->tag == TYPE_FUNCTION)
                sprintf(format, "(*%s)", qualified);
            else
                sprintf(format, "*%s", qualified);

            free(qualified);
        }
        else
        {
            assert(TypeIsArray(DT));

            format = malloc(strlen(embedded) + (dt->array < 0 ? 0 : LogI(dt->array, 10)) + 4);

            if (dt->array < 0)
                sprintf(format, "%s[]", embedded);
            else
                sprintf(format, "%s[%d]", embedded, dt->array);
        }

        char* ret = TypeToStrEmbed(dt->base, format);
        
        free(format);
        return ret;
    }
}

const Symbol* TypeGetBasic (const Type* dt)
{
    if (!dt) return 0;

    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_BASIC ? dt->basic : 0;
}

const Type* TypeGetBase (const Type* dt)
{
    if (!dt) return 0;

    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_PTR || dt->tag == TYPE_ARRAY ? dt->base : 0;
}

const Type* TypeGetReturn (const Type* dt)
{
    if (!dt) return 0;

    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_FUNCTION ? dt->returnType : dt->tag == TYPE_PTR && TypeIsFunction(dt->base) ? TypeGetReturn(dt->base) : 0;
}

const Type* TypeGetRecord (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);

    if (dt->tag == TYPE_BASIC && dt->basic && (dt->basic->tag == SYMBOL_STRUCT || dt->basic->tag == SYMBOL_UNION))
        return dt;   
    else if (dt->tag == TYPE_PTR && TypeIsBasic(dt->base))
        return TypeGetRecord(dt->base);
    else
        return 0;
}

const Type* TypeGetCallable (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_FUNCTION ? dt : dt->tag == TYPE_PTR && dt->base->tag == TYPE_FUNCTION ? dt->base : 0;
}

int TypeGetArraySize (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_ARRAY && dt->array != ArraySizeError ? dt->array : 0;
}

int TypeSetArraySize (Type* dt, int size)
{
    dt = TypeTryThroughTypedef(dt);

    if (dt->tag != TYPE_ARRAY) return 0;

    dt->array = size;
    return 1;
}

// функции для классификации типов
int TypeIsBasic (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_BASIC || TypeIsInvalid(dt);
}

int TypeIsPtr (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_PTR || TypeIsInvalid(dt);
}

int TypeIsArray (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_ARRAY || TypeIsInvalid(dt);
}

int TypeIsFunction (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_FUNCTION || TypeIsInvalid(dt);
}

int TypeIsInvalid (const Type* dt)
{
    if (!dt) return 0;

    dt = TypeTryThroughTypedef(dt);
    return dt->tag == TYPE_INVALID;
}

int TypeIsComplete (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);  
    return !((dt->tag == TYPE_BASIC && !dt->basic->complete) || (dt->tag == TYPE_ARRAY && dt->array == ArraySizeUnspecified));
}

int TypeIsVoid (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    /*Is it a built in type of size zero (void)*/
    return (dt->tag == TYPE_BASIC && dt->basic->tag == SYMBOL_TYPE && dt->basic->size == 0) || TypeIsInvalid(dt);
}

int TypeIsNonVoid (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return (dt->tag != TYPE_BASIC || dt->basic->tag != SYMBOL_TYPE || dt->basic->size != 0) || TypeIsInvalid(dt);
}

int TypeIsStruct (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return (dt->tag == TYPE_BASIC && dt->basic->tag == SYMBOL_STRUCT) || TypeIsInvalid(dt);
}

int TypeIsUnion (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return (dt->tag == TYPE_BASIC && dt->basic->tag == SYMBOL_UNION) || TypeIsInvalid(dt);
}

TYPE_MUTAB TypeIsMutable (const Type* dt)
{
    TypeQualifiers qual = TypeQualifiersCreate();
    dt = TypeTryThroughTypedefQual(dt, &qual);

    if (qual.isConst) return TYPE_MUTCONSTQUALIFIED;
    else if (dt->tag == TYPE_BASIC && dt->basic->hasConstFields)
        return TYPE_MUTHASCONSTFIELDS;
    else
        return TYPE_MUTABLE;
}

int TypeIsNumeric (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return (dt->tag == TYPE_BASIC && (dt->basic->typeMask & TYPEMASK_NUMERIC)) || TypeIsPtr(dt) || TypeIsInvalid(dt);
}

int TypeIsOrdinal (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return (dt->tag == TYPE_BASIC && (dt->basic->typeMask & TYPEMASK_ORDINAL)) || TypeIsPtr(dt) || TypeIsInvalid(dt);
}

int TypeIsEquality (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return (dt->tag == TYPE_BASIC && (dt->basic->typeMask & TYPEMASK_EQUALITY)) || TypeIsPtr(dt) || TypeIsInvalid(dt);
}

int TypeIsAssignment (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return (dt->tag == TYPE_BASIC && (dt->basic->typeMask & TYPEMASK_ASSIGNMENT)) || TypeIsPtr(dt) || TypeIsInvalid(dt);
}

int TypeIsCondition (const Type* dt)
{
    dt = TypeTryThroughTypedef(dt);
    return (dt->tag == TYPE_BASIC && (dt->basic->typeMask & TYPEMASK_CONDITION)) || TypeIsPtr(dt) || TypeIsInvalid(dt);
}

// проверяет типы на совместимость
int TypeIsCompatible (const Type* dt, const Type* Model)
{
    dt = TypeTryThroughTypedef(dt);
    Model = TypeTryThroughTypedef(Model);

    if (TypeIsInvalid(dt) || TypeIsInvalid(Model))
    {
        return 1;

    /*If function requested, match params and return*/
    }
    else if (TypeIsFunction(Model))
    {
        /*fn ptr <=> fn*/
        if (TypeIsPtr(dt) && TypeIsFunction(dt->base))
            return TypeIsCompatible(dt->base, Model);

        else if (!TypeIsFunction(dt) || Model->params != dt->params)
            return 0;

        for (int i = 0; i < Model->params; i++)
            if (!TypeIsEqual(dt->paramTypes[i], Model->paramTypes[i]))
                return 0;

        return TypeIsEqual(dt->returnType, Model->returnType);

    /*If pointer requested, allow pointers and arrays and basic numeric types*/
    }
    else if (TypeIsPtr(Model))
    {
        if (TypeIsFunction(Model->base))
            return TypeIsFunction(dt) && TypeIsCompatible(dt, Model->base);
        else
            return TypeIsPtr(dt) || TypeIsArray(dt) || (TypeIsBasic(dt) && (dt->basic->typeMask & TYPEMASK_NUMERIC));

    /*If array requested, accept only arrays of matching size and type*/
    }
    else if (TypeIsArray(Model))
    {
        return TypeIsArray(dt) && (dt->array == Model->array || Model->array < 0 || dt->array < 0) && TypeIsCompatible(dt->base, Model->base);
    /*Basic type*/
    }
    else
    {
        if (TypeIsPtr(dt))
            return Model->basic->typeMask & TYPEMASK_NUMERIC;
        else if (Model->basic->typeMask == TYPEMASK_INTEGRAL)
            return dt->tag == TYPE_BASIC && dt->basic->typeMask == TYPEMASK_INTEGRAL;
        else
            return dt->tag == TYPE_BASIC && dt->basic == Model->basic;
    }
}

int TypeIsEqual (const Type* L, const Type* R)
{
    TypeQualifiers Lqual = TypeQualifiersCreate();
    TypeQualifiers Rqual = TypeQualifiersCreate();
    
    L = TypeTryThroughTypedefQual(L, &Lqual);
    R = TypeTryThroughTypedefQual(R, &Rqual);

    if (!TypeQualIsEqual(Lqual, Rqual)) return 0;
    else if (TypeIsInvalid(L) || TypeIsInvalid(R)) return 1;
    else if (L->tag != R->tag) return 0;
    else if (TypeIsFunction(L)) return TypeIsCompatible(L, R);
    else if (TypeIsPtr(L)) return TypeIsEqual(L->base, R->base);
    else if (TypeIsArray(L))
        return (L->array == R->array || L->array < 0 || R->array < 0) && TypeIsEqual(L->base, R->base);
    else
    {
        assert(TypeIsBasic(L));
        return L->basic == R->basic;
    }
}

// внутренние функции
static Type* TypeCreate (TYPE_TAG tag)
{
    Type* dt = malloc(sizeof(Type));
    dt->tag = tag;
    dt->qual.isConst = 0;

    dt->basic = 0;

    dt->base = 0;
    dt->array = 0;

    dt->returnType = 0;
    dt->paramTypes = 0;
    dt->params = 0;
    dt->variadic = 0;

    return dt;
}

static TypeQualifiers TypeQualifiersCreate ()
{
    return (TypeQualifiers) {0};
}

static Type* TypeTryThroughTypedef (const Type* dt)
{
    if (dt->tag == TYPE_BASIC && dt->basic && dt->basic->tag == SYMBOL_TYPEDEF)
        return TypeTryThroughTypedef(dt->basic->dt);
    else
        return (Type*) dt;
}

static Type* TypeTryThroughTypedefQual (const Type* dt, TypeQualifiers* qualOut)
{
    if (qualOut) qualOut->isConst |= dt->qual.isConst;

    if (dt->tag == TYPE_BASIC && dt->basic && dt->basic->tag == SYMBOL_TYPEDEF)
        return TypeTryThroughTypedefQual(dt->basic->dt, qualOut);
    else
        return (Type*) dt;
}

static char* TypeQualifiersToStr (TypeQualifiers qual, const char* embedded)
{
    if (qual.isConst)
    {
        char* ret = malloc(strlen("const ") + strlen(embedded) + 1);

        if (embedded[0])
            sprintf(ret, "const %s", embedded);
        else
            strcpy(ret, "const");

        return ret;
    }
    else return strdup(embedded);
}

static int TypeQualIsEqual (TypeQualifiers L, TypeQualifiers R)
{
    return L.isConst == R.isConst;
}