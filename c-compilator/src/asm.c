#include <stdlib.h>
#include <string.h>

#include "..\include\asm.h"
#include "..\include\debug.h"
#include "..\include\arch.h"
#include "..\include\register.h"

AsmCTX* AsmInit (const char* output, const Arch* arch)
{
    AsmCTX* ctx = malloc(sizeof(AsmCTX));
    
    ctx->filename = strdup(output);
    ctx->file = fopen(output, "w");
    ctx->lineNo = 1;
    ctx->depth = 0;
    ctx->arch = arch;
    ctx->stackPtr = OperandCreateReg(RegRequest(REG_RSP, arch->wordsize));
    ctx->basePtr = OperandCreateReg(RegRequest(REG_RBP, arch->wordsize));
    return ctx;
}

void AsmEnd (AsmCTX* ctx)
{
    free(ctx->filename);
    fclose(ctx->file);
    OperandFree(ctx->stackPtr);
    OperandFree(ctx->basePtr);
    free(ctx);
}

void AsmOutLn (AsmCTX* ctx, const char* format, ...)
{
    for (int i = 0; i < 4*ctx->depth; i++)
        fputc(' ', ctx->file);

    va_list args[2];
    va_start(args[0], format);
    va_copy(args[1], args[0]);
    DebugVarMsg(format, args[0]);
    vfprintf(ctx->file, format, args[1]);
    va_end(args[1]);
    va_end(args[0]);

    fputc('\n', ctx->file);
    ctx->lineNo++;
}

void AsmEnter (AsmCTX* ctx)
{
    ctx->depth++;
}

void AsmLeave (AsmCTX* ctx)
{
    if (ctx->depth > 0)
        ctx->depth--;
}