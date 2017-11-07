#include "..\include\ast.h"
#include "..\include\type.h"

Ast* AstCreate (AST_TAG tag, TokenLocation location)
{
    Ast *Node = calloc(1, sizeof(Ast));
    Node->tag = tag;
    Node->location = location;
    return Node;
}

void AstAddChild (Ast* Parent, Ast* Child)
{
    if (Parent->firstChild == 0)
    {
        Parent->firstChild = Child;
        Parent->lastChild = Child;
    }
    else
    {
        Child->prevSibling = Parent->lastChild;
        Parent->lastChild->nextSibling = Child;
        Parent->lastChild = Child;
    }

    Parent->children++;
}

void AstDestroy (Ast* Node)
{
    for (Ast *Current = Node->firstChild, *Next = Current ? Current->nextSibling : 0; Current; Current = Next, Next = Next ? Next->nextSibling : 0)
        AstDestroy(Current);

    if (Node->l) AstDestroy(Node->l);

    if (Node->r && Node->tag != AST_USING) AstDestroy(Node->r);

    if (Node->dt) TypeDestroy(Node->dt);

    free(Node->literal);
    free(Node);
}

Ast* AstCreateInvalid (TokenLocation location)
{
    return AstCreate(AST_INVALID, location);
}

Ast* AstCreateMarker (TokenLocation location, MARKER_TAG marker)
{
    Ast* Node = AstCreate(AST_MARKER, location);
    Node->marker = marker;
    return Node;
}

Ast* AstCreateEmpty (TokenLocation location)
{
    return AstCreate(AST_EMPTY, location);
}

Ast* AstCreateUsing (TokenLocation location, char* name)
{
    Ast* Node = AstCreate(AST_USING, location);
    
    Node->litTag = LITERAL_STR;
    Node->literal = (void*) name;
    return Node;
}

Ast* AstCreateFnImpl (TokenLocation location, Ast* decl)
{
    Ast* Node = AstCreate(AST_FNIMPL, location);
    
    Node->l = decl;
    return Node;
}

Ast* AstCreateType (TokenLocation location, Ast* basic, Ast* expr)
{
    Ast* Node = AstCreate(AST_TYPE, location);
    
    Node->l = basic;
    Node->r = expr;
    return Node;
}

Ast* AstCreateDecl (TokenLocation location, Ast* basic)
{
    Ast* Node = AstCreate(AST_DECL, location);
    
    Node->l = basic;
    return Node;
}

Ast* AstCreateParam (TokenLocation location, Ast* basic, Ast* expr)
{
    Ast* Node = AstCreate(AST_PARAM, location);
    
    Node->l = basic;
    Node->r = expr;
    return Node;
}

Ast* AstCreateStruct (TokenLocation location, Ast* name)
{
    Ast* Node = AstCreate(AST_STRUCT, location);
    
    Node->l = name;
    return Node;
}

Ast* AstCreateUnion (TokenLocation location, Ast* name)
{
    Ast* Node = AstCreate(AST_UNION, location);
    
    Node->l = name;
    return Node;
}

Ast* AstCreateEnum (TokenLocation location, Ast* name)
{
    Ast* Node = AstCreate(AST_ENUM, location);
    
    Node->l = name;
    return Node;
}

Ast* AstCreateConst (TokenLocation location, Ast* r)
{
    Ast* Node = AstCreate(AST_CONST, location);
    
    Node->r = r;
    return Node;
}

Ast* AstCreateBOP (TokenLocation location, Ast* l, OP_TAG o, Ast* r)
{
    Ast* Node = AstCreate(AST_BOP, location);
    
    Node->l = l;
    Node->o = o;
    Node->r = r;
    return Node;
}

Ast* AstCreateUOP (TokenLocation location, OP_TAG o, Ast* r)
{
    Ast* Node = AstCreate(AST_UOP, location);
    
    Node->o = o;
    Node->r = r;
    return Node;
}

Ast* AstCreateTOP (TokenLocation location, Ast* cond, Ast* l, Ast* r)
{
    Ast* Node = AstCreate(AST_TOP, location);
    
    AstAddChild(Node, cond);
    Node->l = l;
    Node->r = r;
    return Node;
}

Ast* AstCreateIndex (TokenLocation location, Ast* base, Ast* index)
{
    Ast* Node = AstCreate(AST_INDEX, location);
    
    Node->l = base;
    Node->r = index;
    return Node;
}

Ast* AstCreateCall (TokenLocation location, Ast* function)
{
    Ast* Node = AstCreate(AST_CALL, location);
    
    Node->l = function;
    return Node;
}

Ast* AstCreateCast (TokenLocation location, Ast* result, Ast* r)
{
    Ast* Node = AstCreate(AST_CAST, location);
    
    Node->l = result;
    Node->r = r;
    return Node;
}

Ast* AstCreateSizeof (TokenLocation location, Ast* r)
{
    Ast* Node = AstCreate(AST_SIZEOF, location);
    
    Node->r = r;
    return Node;
}

Ast* AstCreateLiteral (TokenLocation location, LITERAL_TAG litTag)
{
    Ast* Node = AstCreate(AST_LITERAL, location);
    
    Node->litTag = litTag;
    return Node;
}

Ast* AstCreateLiteralIdent (TokenLocation location, char* ident)
{
    Ast* Node = AstCreateLiteral(location, LITERAL_IDENT);
    
    Node->literal = (void*) ident;
    return Node;
}

Ast* AstCreateAssert (TokenLocation location, Ast* expr)
{
    Ast* Node = AstCreate(AST_ASSERT, location);
    
    Node->r = expr;
    return Node;
}

int AstIsValueTag (AST_TAG tag)
{
    return tag == AST_BOP || tag == AST_UOP || tag == AST_TOP
           || tag == AST_CALL || tag == AST_INDEX || tag == AST_CAST
           || tag == AST_SIZEOF || tag == AST_LITERAL || tag == AST_VAStart
           || tag == AST_VAEnd || tag == AST_VAArg || tag == AST_VACopy
           || tag == AST_ASSERT;
}

const char* AstTagGetStr (AST_TAG tag)
{
    if (tag == AST_UNDEFINED) return "AST_UNDEFINED";
    else if (tag == AST_INVALID) return "AST_INVALID";
    else if (tag == AST_MARKER) return "AST_MARKER";
    else if (tag == AST_EMPTY) return "AST_EMPTY";
    else if (tag == AST_MODULE) return "AST_MODULE";
    else if (tag == AST_USING) return "AST_USING";
    else if (tag == AST_FNIMPL) return "AST_FNIMPL";
    else if (tag == AST_DECL) return "AST_DECL";
    else if (tag == AST_PARAM) return "AST_PARAM";
    else if (tag == AST_STRUCT) return "AST_STRUCT";
    else if (tag == AST_UNION) return "AST_UNION";
    else if (tag == AST_ENUM) return "AST_ENUM";
    else if (tag == AST_TYPE) return "AST_TYPE";
    else if (tag == AST_CONST) return "AST_CONST";
    else if (tag == AST_CODE) return "AST_CODE";
    else if (tag == AST_BRANCH) return "AST_BRANCH";
    else if (tag == AST_LOOP) return "AST_LOOP";
    else if (tag == AST_ITER) return "AST_ITER";
    else if (tag == AST_RETURN) return "AST_RETURN";
    else if (tag == AST_BREAK) return "AST_BREAK";
    else if (tag == AST_CONTINUE) return "AST_CONTINUE";
    else if (tag == AST_BOP) return "AST_BOP";
    else if (tag == AST_UOP) return "AST_UOP";
    else if (tag == AST_TOP) return "AST_TOP";
    else if (tag == AST_INDEX) return "AST_INDEX";
    else if (tag == AST_CALL) return "AST_CALL";
    else if (tag == AST_CAST) return "AST_CAST";
    else if (tag == AST_SIZEOF) return "AST_SIZEOF";
    else if (tag == AST_LITERAL) return "AST_LITERAL";
    else if (tag == AST_ELLIPSIS) return "AST_ELLIPSIS";
    else if (tag == AST_VAStart) return "AST_VAStart";
    else if (tag == AST_VAEnd) return "AST_VAEnd";
    else if (tag == AST_VAArg) return "AST_VAArg";
    else if (tag == AST_VACopy) return "AST_VACopy";
    else if (tag == AST_ASSERT) return "AST_ASSERT";
    else
    {
        char* str = malloc(LogI(tag, 10)+2);
        
        sprintf(str, "%d", tag);
        DebugErrorUnhandled("AstTagGetStr", "AST tag", str);
        free(str);
        return "<unhandled>";
    }
}

const char* LiteralTagGetStr (LITERAL_TAG tag)
{
    if (tag == LITERAL_UNDEFINED) return "LITERAL_UNDEFINED";
    else if (tag == LITERAL_IDENT) return "LITERAL_IDENT";
    else if (tag == LITERAL_INT) return "LITERAL_INT";
    else if (tag == LITERAL_CHAR) return "LITERAL_CHAR";
    else if (tag == LITERAL_STR) return "LITERAL_STR";
    else if (tag == LITERAL_BOOL) return "LITERAL_BOOL";
    else if (tag == LITERAL_COMPOUND) return "LITERAL_COMPOUND";
    else if (tag == LITERAL_INIT) return "LITERAL_INIT";
    else if (tag == LITERAL_LAMBDA) return "LITERAL_LAMBDA";
    else
    {
        char* str = malloc(LogI(tag, 10) + 2);
        
        sprintf(str, "%d", tag);
        DebugErrorUnhandled("LiteralTagGetStr", "literal tag", str);
        free(str);
        return "<unhandled>";
    }
}

const char* OpTagGetStr (OP_TAG tag)
{
    if (tag == OP_UNDEFINED) return "<undefined>";
    else if (tag == OP_COMMA) return ",";
    else if (tag == OP_ASSIGN) return "=";
    else if (tag == OP_ANDASSIGN) return "&=";
    else if (tag == OP_ORASSIGN) return "|=";
    else if (tag == OP_XORASSIGN) return "^=";
    else if (tag == OP_SHRASSIGN) return ">>=";
    else if (tag == OP_SHLASSIGN) return "<<=";
    else if (tag == OP_PLUSASSIGN) return "+=";
    else if (tag == OP_MINASSIGN) return "-=";
    else if (tag == OP_MULASSIGN) return "*=";
    else if (tag == OP_DIVASSIGN) return "/=";
    else if (tag == OP_MODASSIGN) return "%=";
    else if (tag == OP_TERNARY) return "ternary ?:";
    else if (tag == OP_ANDAND) return "&&";
    else if (tag == OP_OROR) return "||";
    else if (tag == OP_AND) return "&";
    else if (tag == OP_OR) return "|";
    else if (tag == OP_XOR) return "^";
    else if (tag == OP_EQ) return "==";
    else if (tag == OP_NEQ) return "!=";
    else if (tag == OP_GT) return ">";
    else if (tag == OP_GE) return ">=";
    else if (tag == OP_LT) return "<";
    else if (tag == OP_LE) return "<=";
    else if (tag == OP_SHR) return ">>";
    else if (tag == OP_SHL) return "<<";
    else if (tag == OP_PLUS) return "+";
    else if (tag == OP_MIN) return "-";
    else if (tag == OP_MUL) return "*";
    else if (tag == OP_DIV) return "/";
    else if (tag == OP_MOD) return "%";
    else if (tag == OP_NOT) return "!";
    else if (tag == OP_TILDE) return "~";
    else if (tag == OP_UNARYPLUS) return "+";
    else if (tag == OP_UNARYMIN) return "-";
    else if (tag == OP_DEREF) return "*";
    else if (tag == OP_ADDRESS) return "&";
    else if (tag == OP_PREPLUSPLUS) return "++";
    else if (tag == OP_PREMINMIN) return "--";
    else if (tag == OP_PLUSPLUS) return "++";
    else if (tag == OP_MINMIN) return "--";
    else if (tag == OP_INDEX) return "[]";
    else if (tag == OP_CALL) return "()";
    else if (tag == OP_MEMBER) return ".";
    else if (tag == OP_MEMBERDEREF) return "->";
    else if (tag == OP_ASSERT) return "assert";
    else
    {
        char* str = malloc(LogI((int) tag, 10) + 2);
        
        sprintf(str, "%d", tag);
        DebugErrorUnhandled("OpTagGetStr", "operator tag", str);
        free(str);
        return "<unhandled>";
    }
}

int OpIsNumeric (OP_TAG o)
{
    return o == OP_PLUS || o == OP_MIN || o == OP_PLUSASSIGN || o == OP_MINASSIGN
           || o == OP_MUL || o == OP_DIV || o == OP_MOD
           || o == OP_MULASSIGN || o == OP_DIVASSIGN || o == OP_MODASSIGN
           || o == OP_AND || o == OP_OR || o == OP_XOR
           || o == OP_ANDASSIGN || o == OP_ORASSIGN || o == OP_XORASSIGN
           || o == OP_SHL || o == OP_SHR || o == OP_SHLASSIGN || o == OP_SHRASSIGN;
}

int OpIsBitwise (OP_TAG o)
{
    return o == OP_AND || o == OP_OR || o == OP_XOR
           || o == OP_ANDASSIGN || o == OP_ORASSIGN || o == OP_XORASSIGN;
}

int OpIsOrdinal (OP_TAG o)
{
    return o == OP_GT || o == OP_LT || o == OP_GE || o == OP_LE;
}

int OpIsEquality (OP_TAG o)
{
    return o == OP_EQ || o == OP_NEQ;
}

int OpIsAssignment (OP_TAG o)
{
    return o == OP_ASSIGN
           || o == OP_PLUSASSIGN || o == OP_MINASSIGN
           || o == OP_MULASSIGN || o == OP_DIVASSIGN || o == OP_MODASSIGN
           || o == OP_ANDASSIGN || o == OP_ORASSIGN || o == OP_XORASSIGN
           || o == OP_SHLASSIGN || o == OP_SHRASSIGN;
}

int OpIsLogical (OP_TAG o)
{
    return o == OP_ANDAND || o == OP_OROR;
}

int OpIsMember (OP_TAG o)
{
    return o == OP_MEMBER || o == OP_MEMBERDEREF;
}

int OpIsDeref (OP_TAG o)
{
    return o == OP_MEMBERDEREF;
}