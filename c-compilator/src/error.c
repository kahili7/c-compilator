#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\ast.h"
#include "..\include\type.h"
#include "..\include\error.h"
#include "..\include\lexer.h"
#include "..\include\parser-internal.h"

static void VErrorF (const char* format, va_list args)
{
    const char* consoleNormal  = "\e[1;0m";
    const char* consoleRed     = "\e[1;31m";
    const char* consoleGreen   = "\e[1;32m";
    const char* consoleYellow  = "\e[1;33m";
    const char* consoleBlue    = "\e[1;34m";
    const char* consoleMagenta = "\e[1;35m";
    const char* consoleCyan    = "\e[1;36m";
    const char* consoleWhite   = "\e[1;37m";

    const char* colourString = consoleWhite;
    const char* colourNumber = consoleMagenta;
    const char* colourOp     = consoleCyan;
    const char* colourType   = consoleGreen;
    const char* colourIdent  = consoleWhite;
    const char* colourTag    = consoleCyan;

    int flen = strlen(format);

    int upto = 0;

    for (int i = 0; i < flen; i++)
    {
        if (format[i] == '$')
        {
            printf("%.*s", i-upto, format+upto);
            upto = i+2;
            i++;

            /*Regular string*/
            if (format[i] == 's') printf("%s", va_arg(args, const char*));

            /*Highlighted string*/
            else if (format[i] == 'h') printf("%s%s%s", colourString, va_arg(args, const char*), consoleNormal);

            /*Red string*/
            else if (format[i] == 'r') printf("%s%s%s", consoleRed, va_arg(args, const char*), consoleNormal);

            /*Operator*/
            else if (format[i] == 'o') printf("%s%s%s", colourOp, OpTagGetStr(va_arg(args, OP_TAG)), consoleNormal);

            /*Integer*/
            else if (format[i] == 'd')
                printf("%s%d%s", colourNumber, va_arg(args, int), consoleNormal);

            /*Raw type*/
            else if (format[i] == 't')
            {
                char* typeStr = TypeToStr(va_arg(args, const Type*));
                
                printf("%s%s%s", colourType, typeStr, consoleNormal);
                free(typeStr);

            /*Type with name*/
            }
            else if (format[i] == 'T')
            {
                const Type* dt = va_arg(args, const type*);
                const char* ident = va_arg(args, const char*);

                char* identStr = StrJoin((char**) (const char* []) {colourIdent, ident, colourType}, 3, malloc);
                char* typeStr = TypeToStrEmbed(dt, identStr);
                
                printf("%s%s%s", colourType, typeStr, consoleNormal);
                free(identStr);
                free(typeStr);

            /*Named symbol*/
            }
            else if (format[i] == 'n')
            {
                const Symbol* Symbol = va_arg(args, const sym*);
                const char* ident = Symbol->ident ? Symbol->ident : "";

                if (Symbol->tag != SYMBOL_ID && Symbol->tag != SYMBOL_PARAM)
                    printf("%s%s%s %s%s", colourTag, SymbolTagGetStr(Symbol->tag), colourIdent, ident, consoleNormal);

                else if (Symbol->dt)
                    ErrorF("$T", Symbol->dt, ident);

                else
                    printf("%s%s%s", colourIdent, ident, consoleNormal);

            /*AST node*/
            }
            else if (format[i] == 'a')
            {
                const Ast* Node = va_arg(args, const Ast*);

                if (Node->symbol && Node->symbol->ident && Node->symbol->dt)
                    if (Node->symbol->tag == SYMBOL_ID || Node->symbol->tag == SYMBOL_PARAM)
                        ErrorF("$T", Node->dt, Node->symbol->ident);

                    else
                        ErrorF("$n", Node->symbol);
                else
                    ErrorF("$t", Node->dt);

            /*Symbol tag, semantic "class"*/
            }
            else if (format[i] == 'c')
            {
                const Symbol* Symbol = va_arg(args, const Symbol*);
                const char* classStr;

                if (Symbol->tag == SYMBOL_ID || Symbol->tag == SYMBOL_PARAM)
                {
                    if (Symbol->dt && !TypeIsInvalid(Symbol->dt))
                        classStr = TypeIsFunction(Symbol->dt) ? "function" : "variable";
                    else
                        classStr = "\b";
                }
                else
                    classStr = SymbolTagGetStr(Symbol->tag);

                printf("%s%s%s", colourTag, classStr, consoleNormal);
            }
            else if (format[i] == '\0')
                break;
            else
                DebugErrorUnhandledChar("VErrorF", "format specifier", format[i]);
        }
    }

    printf("%.*s", flen-upto, &format[upto]);
}

void ErrorF (const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VErrorF(format, args);
    va_end(args);
}