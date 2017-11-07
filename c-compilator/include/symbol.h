#ifndef X_INCLUDE_SYMBOL
#define X_INCLUDE_SYMBOL

#include "vector.h"

typedef enum SYMBOL_TAG {
    SYMBOL_UNDEFINED,
    SYMBOL_SCOPE,
    SYMBOL_MODULELINK,
    SYMBOL_LINK,
    SYMBOL_TYPE,
    SYMBOL_TYPEDEF,
    SYMBOL_STRUCT,
    SYMBOL_UNION,
    SYMBOL_ENUM,
    SYMBOL_ENUMCONSTANT,
    SYMBOL_ID,
    SYMBOL_PARAM
} SYMBOL_TAG;

// теги хранения со сроком жизни
typedef enum STORAGE_TAG {
    STORAGE_UNDEFINED,
    STORAGE_AUTO,
    STORAGE_STATIC,
    STORAGE_EXTERN
} STORAGE_TAG;

typedef enum TYPEMASK_TAG {
    TYPEMASK_NONE,
    TYPEMASK_NUMERIC = 1 << 0,  //маска для арифметических операций
    ///Ordinal describes whether it has a defined order, and therefore
    ///can be compared with <, <=, >, >=
    TYPEMASK_ORDINAL = 1 << 1,
    ///Equality describes whether equality can be tested with != and ==
    TYPEMASK_EQUALITY = 1 << 2,
    ///Assignment describes whether you can assign new values directly
    ///with =
    TYPEMASK_ASSIGNMENT = 1 << 3,
    ///Condition describes whether the type can be tested for boolean
    ///truth
    TYPEMASK_CONDITION = 1 << 4,
    //обобщенные маски
    TYPEMASK_INTEGRAL = TYPEMASK_NUMERIC | TYPEMASK_ORDINAL | TYPEMASK_EQUALITY | TYPEMASK_ASSIGNMENT | TYPEMASK_CONDITION,
    TYPEMASK_BOOL = TYPEMASK_EQUALITY | TYPEMASK_ASSIGNMENT | TYPEMASK_CONDITION,
    TYPEMASK_STRUCT = TYPEMASK_ASSIGNMENT,
    TYPEMASK_UNION = TYPEMASK_ASSIGNMENT,
    TYPEMASK_ENUM = TYPEMASK_INTEGRAL
} TYPEMASK_TAG;

typedef struct Symbol {
    SYMBOL_TAG tag;
    char* ident;
    
    Vector decls;
    const Ast *impl;
    
    union {
        /*symId symParam symTypedef symEnumConstant*/
        struct {
            Type *dt;
            /*symId symParam*/
            STORAGE_TAG storage;
        };
        
        /*symType symStruct symUnion symEnum*/
        struct {
            int size;
            ///A mask defining operator capabilities
            TYPEMASK_TAG typeMask;
            int complete;
        };
    };
       
    Symbol *parent;     //родитель
    Vector children;    //вектор детей
    int nthChild;       //позиция в дереве

    union {
        /*symId: storageStatic storageExtern*/
        ///Label associated with this symbol in the assembly
        char *label;
        /*symId: storageAuto symParam*/
        ///Offset in bytes, from the top of the stack frame or struct/union
        int offset;
        /*symEnumConstants*/
        int constValue;
        /*symStruct*/
        int hasConstFields;
    };
} Symbol;
#endif /*X_INCLUDE_SYMBOL*/