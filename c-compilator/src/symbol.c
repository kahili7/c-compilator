#include "..\include\type.h"
#include "..\include\symbol.h"

Symbol* SymbolInit ()
{
    return SymbolCreate(SYMBOL_SCOPE);
}

void SymbolEnd (Symbol* Global)
{
    SymbolDestroy(Global);
}

Symbol* SymbolCreateScope (Symbol* Parent)
{
    return SymbolCreateParented(SYMBOL_SCOPE, Parent);
}

Symbol* SymbolCreateModuleLink (Symbol* parent, const Symbol* module)
{
    Symbol* Symbol = SymbolCreateParented(SYMBOL_MODULELINK, parent);
    
    VectorPush(&Symbol->children, (Symbol*) module);
    return Symbol;
}

Symbol* SymbolCreateType (Symbol* Parent, const char* ident, int size, TYPEMASK_TAG typeMask)
{
    Symbol* Symbol = SymbolCreateParented(SYMBOL_TYPE, Parent);
    
    Symbol->ident = strdup(ident);
    Symbol->size = size;
    Symbol->typeMask = typeMask;
    Symbol->complete = 1;
    return Symbol;
}

Symbol* SymbolCreateNamed (SYMBOL_TAG tag, Symbol* Parent, const char* ident)
{
    Symbol* Symbol = SymbolCreateParented(tag, Parent);
    
    Symbol->ident = strdup(ident);

    if (tag == SYMBOL_STRUCT) Symbol->typeMask = TYPEMASK_STRUCT;
    else if (tag == SYMBOL_UNION) Symbol->typeMask = TYPEMASK_UNION;
    else if (tag == SYMBOL_ENUM) Symbol->typeMask = TYPEMASK_ENUM;

    return Symbol;
}

void SymbolChangeParent (Symbol* Symbol, Symbol* parent)
{
    VectorSet(&Symbol->parent->children, Symbol->nthChild, SymbolCreateLink(Symbol));

    /*Add it to the new parent*/
    SymbolAddChild(parent, Symbol);
}

int SymbolIsFunction (const Symbol* Symbol)
{
    return Symbol->tag == SYMBOL_ID && TypeIsFunction(Symbol->dt) && (Symbol->storage == STORAGE_STATIC || Symbol->storage == STORAGE_EXTERN);
}

const Symbol* SymbolGetNthParam (const Symbol* fn, int n)
{
    int paramNo = 0;

    for (int i = 0; i < fn->children.length; i++)
    {
        const Symbol* child = VectorGet(&fn->children, i);

        if (child->tag == SYMBOL_PARAM)
            if (paramNo++ == n) return child;
    }

    return 0;
}

Symbol* SymbolChild (const Symbol* Scope, const char* look)
{

    for (int n = 0; n < Scope->children.length; n++)
    {
        Symbol* Current = VectorGet(&Scope->children, n);

        /*Found it?*/
        if (Current->ident && !strcmp(Current->ident, look)) return Current;

        /*Anonymous inside a struct/union?*/
        if (Current->ident && !Current->ident[0] && (Current->parent->tag == SYMBOL_STRUCT || Current->parent->tag == SYMBOL_UNION))
        {
            Symbol* Found = SymbolChild(Current, look);

            if (Found) return Found;
        }

        /*Included module?*/
        if (Current->tag == SYMBOL_MODULELINK)
        {
            Symbol* Found = SymbolChild(Current->children.buffer[0], look);

            if (Found) return Found;
        /*Reparented symbol?*/
        }
        else if (Current->tag == SYMBOL_LINK)
        {
            Symbol* Found = SymbolChild(Current, look);

            if (Found) return Found;
        }
    }

    return 0;
}

Symbol* SymbolFind (const Symbol* Scope, const char* look)
{
    for (;Scope;Scope = Scope->parent)
    {
        Symbol* Found = SymbolChild(Scope, look);

        if (Found) return Found;
    }

    return 0;
}

const char* SymbolTagGetStr (SYMBOL_TAG tag)
{
    if (tag == SYMBOL_UNDEFINED) return "undefined";
    else if (tag == SYMBOL_SCOPE) return "scope";
    else if (tag == SYMBOL_MODULELINK) return "module link";
    else if (tag == SYMBOL_LINK) return "sym link";
    else if (tag == SYMBOL_TYPE) return "type";
    else if (tag == SYMBOL_TYPEDEF) return "typedef";
    else if (tag == SYMBOL_STRUCT) return "struct";
    else if (tag == SYMBOL_UNION) return "union";
    else if (tag == SYMBOL_ENUM) return "enum";
    else if (tag == SYMBOL_ENUMCONSTANT) return "enum constant";
    else if (tag == SYMBOL_ID) return "id";
    else if (tag == SYMBOL_PARAM) return "parameter";
    else
    {
        char* str = malloc(LogI(tag, 10) + 2);
        
        sprintf(str, "%d", tag);
        DebugErrorUnhandled("SymbolTagGetStr", "symbol tag", str);
        free(str);
        return "<unhandled>";
    }
}

const char* StorageTagGetStr (STORAGE_TAG tag)
{
    if (tag == STORAGE_UNDEFINED) return "storageUndefined";
    else if (tag == STORAGE_AUTO) return "storageAuto";
    else if (tag == STORAGE_STATIC) return "storageStatic";
    else if (tag == STORAGE_EXTERN) return "storageExtern";
    else
    {
        char* str = malloc(LogI(tag, 10) + 2);
        
        sprintf(str, "%d", tag);
        DebugErrorUnhandled("StorageTagGetStr", "storage tag", str);
        free(str);
        return "<undefined>";
    }
}

//внутренние функции
static Symbol* SymbolCreate (SYMBOL_TAG tag)
{
    Symbol *sym = malloc(sizeof(Symbol));
    sym->tag = tag;
    sym->ident = 0;

    VectorInit(&sym->decls, 2);
    sym->impl = 0;

    sym->storage = SYMBOL_UNDEFINED;
    sym->dt = 0;

    sym->size = 0;
    sym->typeMask = TYPEMASK_NONE;
    sym->complete = 0;

    VectorInit(&sym->children, 4);
    sym->parent = 0;

    sym->label = 0;
    sym->offset = 0;
    sym->constValue = 0;
    sym->hasConstFields = 0;

    return Symbol;
}

static Symbol* SymbolCreateParented (SYMBOL_TAG tag, Symbol* Parent)
{
    Symbol* Symbol = SymbolCreate(tag);
    
    SymbolAddChild(Parent, Symbol);
    return Symbol;
}

static void SymbolDestroy (Symbol *sym)
{
    free(sym->ident);

    VectorFree(&sym->decls);

    if (sym->tag != SYMBOL_MODULELINK && sym->tag != SYMBOL_LINK) VectorFreeObjs(&sym->children, (VectorDtor) SymbolDestroy);
    else
        VectorFree(&Symbol->children);

    if ((sym->tag == SYMBOL_ID || sym->tag == SYMBOL_PARAM || sym->tag == SYMBOL_ENUMCONSTANT || sym->tag == SYMBOL_TYPEDEF) && sym->dt)
        TypeDestroy(sym->dt);

    if (sym->tag == SYMBOL_ID && (sym->storage == STORAGE_STATIC || sym->storage == STORAGE_EXTERN))
        free(sym->label);

    free(Symbol);
}

static void SymbolAddChild (Symbol* Parent, Symbol* Child)
{
    Child->parent = Parent;
    Child->nthChild = VectorPush(&Parent->children, Child);
}

static Symbol* SymbolCreateLink (Symbol* Child)
{
    Symbol* Link = SymbolCreate(SYMBOL_LINK);
    
    VectorPush(&Link->children, (Symbol*) Child);
    return Link;
}