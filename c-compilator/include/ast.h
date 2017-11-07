#ifndef X_INCLUDE_AST
#define X_INCLUDE_AST

#include "..\include\type.h"
#include "..\include\parser-internal.h"

// абстрактное синтаксическое дерево

typedef enum AST_TAG {
    AST_UNDEFINED,
    AST_INVALID,
    AST_MARKER,
    AST_EMPTY,
    AST_MODULE,
    AST_USING,
    AST_FNIMPL,
    AST_TYPE,
    AST_DECL,
    AST_PARAM,
    AST_STRUCT,
    AST_UNION,
    AST_ENUM,
    AST_CONST,
    AST_BOP,
    AST_UOP,
    AST_TOP,
    AST_INDEX,
    AST_CALL,
    AST_CAST,
    AST_SIZEOF,
    AST_LITERAL,
    AST_ASSERT,
    AST_VAStart,
    AST_VAEnd,
    AST_VAArg,
    AST_VACopy,
    AST_CODE,
    AST_BRANCH,
    AST_LOOP,
    AST_ITER,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_ELLIPSIS
} AST_TAG;

typedef enum OP_TAG {
    OP_UNDEFINED,
    OP_COMMA,
    OP_ASSIGN,
    
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NEQ,
            
    OP_ANDAND,
    OP_OROR,
            
    OP_AND,
    OP_OR,
    OP_XOR,        
    OP_ANDASSIGN,
    OP_ORASSIGN,
    OP_XORASSIGN,
            
    OP_TERNARY,
    
    OP_SHL,
    OP_SHR,
    OP_SHLASSIGN,
    OP_SHRASSIGN,
            
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_MULASSIGN,
    OP_DIVASSIGN,
    OP_MODASSIGN,
    
    OP_UNARYPLUS,
    OP_UNARYMIN,       
    OP_PLUS,
    OP_MIN,
    OP_PLUSASSIGN,
    OP_MINASSIGN,
            
    OP_NOT,
    OP_TILDE,
    
    OP_PREPLUSPLUS,
    OP_PREMINMIN,
    OP_PLUSPLUS,
    OP_MINMIN,
    
    OP_DEREF,
    OP_ADDRESS,
    OP_INDEX,
    OP_CALL,
    OP_MEMBER,
    OP_MEMBERDEREF,
    OP_ASSERT
} OP_TAG;

typedef enum MARKER_TAG {
    MARKER_UNDEFINED,
    MARKER_AUTO,
    MARKER_STATIC,
    MARKER_EXTERN,
    MARKER_ArrayDesignatedInit,
    MARKER_StructDesignatedInit
} MARKER_TAG;

typedef enum LITERAL_TAG {
    LITERAL_UNDEFINED,
    LITERAL_IDENT,
    LITERAL_INT,
    LITERAL_CHAR,
    LITERAL_BOOL,
    LITERAL_STR,
    LITERAL_COMPOUND,
    LITERAL_INIT,
    LITERAL_LAMBDA
} LITERAL_TAG;

typedef struct Ast {
    AST_TAG tag;
    
    TokenLocation location;
    
    Ast *firstChild;
    Ast *lastChild;
    Ast *nextSibling;
    Ast *prevSibling;
    int children;
    
    // бинарное дерево
    Ast *l;
    Ast *r;
    OP_TAG o;
    Type *dt;
    Symbol *symbol;
    
    union {
        /*astMarker*/
        MARKER_TAG marker;
        
        /*astLiteral*/
        struct {
            LITERAL_TAG litTag;
            void *literal;
        };
    };
} Ast;

// функции
int OpIsDeref (OP_TAG o);
int OpIsMember (OP_TAG o);
int OpIsLogical (OP_TAG o);
int OpIsAssignment (OP_TAG o);
int OpIsEquality (OP_TAG o);
int OpIsOrdinal (OP_TAG o);
int OpIsBitwise (OP_TAG o);
int OpIsNumeric (OP_TAG o);

const char* OpTagGetStr (OP_TAG tag);
const char* LiteralTagGetStr (LITERAL_TAG tag);
const char* AstTagGetStr (AST_TAG tag);

int AstIsValueTag (AST_TAG tag);
Ast* AstCreateAssert (TokenLocation location, Ast* expr);
Ast* AstCreateLiteralIdent (TokenLocation location, char* ident);
Ast* AstCreateLiteral (TokenLocation location, LITERAL_TAG litTag);
Ast* AstCreateSizeof (TokenLocation location, Ast* r);
Ast* AstCreateCast (TokenLocation location, Ast* result, Ast* r);
Ast* AstCreateCall (TokenLocation location, Ast* function);
Ast* AstCreateIndex (TokenLocation location, Ast* base, Ast* index);
Ast* AstCreateTOP (TokenLocation location, Ast* cond, Ast* l, Ast* r);
Ast* AstCreateUOP (TokenLocation location, OP_TAG o, Ast* r);
Ast* AstCreateBOP (TokenLocation location, Ast* l, OP_TAG o, Ast* r);
Ast* AstCreateConst (TokenLocation location, Ast* r);
Ast* AstCreateEnum (TokenLocation location, Ast* name);
Ast* AstCreateUnion (TokenLocation location, Ast* name);
Ast* AstCreateStruct (TokenLocation location, Ast* name);
Ast* AstCreateParam (TokenLocation location, Ast* basic, Ast* expr);
Ast* AstCreateDecl (TokenLocation location, Ast* basic);
Ast* AstCreateType (TokenLocation location, Ast* basic, Ast* expr);
Ast* AstCreateFnImpl (TokenLocation location, Ast* decl);
Ast* AstCreateUsing (TokenLocation location, char* name);
Ast* AstCreateEmpty (TokenLocation location);
Ast* AstCreateMarker (TokenLocation location, MARKER_TAG marker);
Ast* AstCreateInvalid (TokenLocation location);
Ast* AstCreate (AST_TAG tag, TokenLocation location);

void AstDestroy (Ast* Node);

void AstAddChild (Ast* Parent, Ast* Child);
#endif /*X_INCLUDE_AST*/