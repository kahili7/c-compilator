#ifndef X_INCLUDE_TYPE
#define X_INCLUDE_TYPE

#include "..\include\symbol.h"
#include "..\include\arch.h"

typedef enum TYPE_TAG {
    TYPE_BASIC,
    TYPE_PTR,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_INVALID
} TYPE_TAG;

typedef enum TYPE_MUTAB {
    TYPE_MUTCONSTQUALIFIED,
    TYPE_MUTHASCONSTFIELDS,
    TYPE_MUTABLE
} TYPE_MUTAB;

enum {
    ArraySizeError = -2,
    ArraySizeUnspecified = -1   //размер массива неопределен
};

typedef struct TypeQualifiers {
    int isConst;
} TypeQualifiers;

typedef struct Type {
    TYPE_TAG tag;
    
    TypeQualifiers qual;    //квалификатор типа
    
    union {
        /*базовый тип*/
        const Symbol* basic;
        
        /*тип указатель или массив*/
        struct {
            Type* base;     //указатель на массив
            long array;     //размер массива
        };
        
        /*тип функция*/
        struct {
            Type* returnType;   //вовращаемый тип
            Type** paramTypes;  //массив типов параметров
            int params;         //количество типов параметров
            int variadic;       //...
        };
    };
} Type;

const char* TypeTagGetStr (TYPE_TAG tag);
int TypeGetSize (const Arch* arch, const Type* dt);

char* TypeToStr (const Type* dt);
char* TypeToStrEmbed (const Type* dt, const char* embedded);

const Symbol* TypeGetBasic (const Type* dt);
const Type* TypeGetBase (const Type* dt);
const Type* TypeGetReturn (const Type* dt);
const Type* TypeGetRecord (const Type* dt);
const Type* TypeGetCallable (const Type* dt);

int TypeGetArraySize (const Type* dt);
int TypeSetArraySize (Type* dt, int size);

int TypeIsBasic (const Type* dt);
int TypeIsPtr (const Type* dt);
int TypeIsArray (const Type* dt);
int TypeIsFunction (const Type* dt);
int TypeIsInvalid (const Type* dt);
int TypeIsComplete (const Type* dt);
int TypeIsVoid (const Type* dt);
int TypeIsNonVoid (const Type* dt);
int TypeIsStruct (const Type* dt);
int TypeIsUnion (const Type* dt);
TYPE_MUTAB TypeIsMutable (const Type* dt);
int TypeIsNumeric (const Type* dt);
int TypeIsOrdinal (const Type* dt);
int TypeIsEquality (const Type* dt);
int TypeIsAssignment (const Type* dt);
int TypeIsCondition (const Type* dt);
int TypeIsCompatible (const Type* dt, const Type* Model);
int TypeIsEqual (const Type* L, const Type* R);

Type* TypeCreateBasic (const Symbol* basic);
Type* TypeCreatePtr (Type* base);
Type* TypeCreateArray (Type* base, int size);
Type* TypeCreateFunction (Type* returnType, Type** paramTypes, int params, int variadic);
Type* TypeCreateInvalid ();
void TypeDestroy (Type* dt);

Type* TypeDeepDuplicate (const Type* dt);

Type* TypeDeriveFrom (const Type* dt);
Type* TypeDeriveFromTwo (const Type* L, const Type* R);
Type* TypeDeriveUnified (const Type* L, const Type* R);
Type* TypeDeriveBase (const Type* dt);
Type* TypeDerivePtr (const Type* base);
Type* TypeDeriveArray (const Type* base, int size);
Type* TypeDeriveReturn (const Type* fn);
#endif /*X_INCLUDE_TYPE*/