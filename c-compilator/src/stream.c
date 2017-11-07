#include <string.h>
#include <stdio.h>

#include "..\include\stream.h"

StreamCTX* StreamInit(const char* filename)
{
    StreamCTX* ctx = malloc(sizeof(StreamCTX));
    ctx->file = fopen(filename, "r");
    
    ctx->current = 0;
    ctx->line = 1;
    ctx->lineChar = 0;
    
    StreamNext(ctx);
    
    return ctx;
}

void StreamEnd (StreamCTX* ctx)
{
    fclose(ctx->file);
    free(ctx);
}

char StreamNext (StreamCTX* ctx)
{
    char old = ctx->current;
    ctx->current = fgetc(ctx->file);

    if (feof(ctx->file) || ctx->current == '\255') ctx->current = 0;

    ctx->lineChar++;

    if (old == '\n')
    {
        ctx->line++;
        ctx->lineChar = 1;
    }
    else if (old == '\t') ctx->lineChar += 3;

    return old;
}

char StreamPrev (StreamCTX* ctx)
{
    char old = ctx->current;

    fseek(ctx->file, -2, SEEK_CUR);
    StreamNext(ctx);

    ctx->lineChar--;

    if (ctx->current == '\n')
    {
        ctx->line--;
        ctx->lineChar = -1;

    }
    else if (ctx->current == '\t') ctx->lineChar -= 3;

    return old;
}

