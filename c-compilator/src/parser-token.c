#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\parser-internal.h"
#include "..\include\tokens.h"
#include "..\include\debug.h"
#include "..\include\error.h"

int TokenIsKeyword (const ParserCTX* ctx, KEYWORD_TAG keyword)
{
    return ctx->lexer->keyword == keyword;
}

int TokenIsPunct (const ParserCTX* ctx, PUNCT_TAG punct)
{
    return ctx->lexer->punct == punct;
}

int TokenIsIdent (const ParserCTX* ctx)
{
    return ctx->lexer->token == TOK_IDENT;
}

int TokenIsInt (const ParserCTX* ctx)
{
    return ctx->lexer->token == TOK_INT;
}

int TokenIsString (const ParserCTX* ctx)
{
    return ctx->lexer->token == TOK_STR;
}

int TokenIsChar (const ParserCTX* ctx)
{
    return ctx->lexer->token == TOK_CHR;
}

void TokenNext (ParserCTX* ctx)
{
    LexerNext(ctx->lexer);
    ctx->location = (TokenLocation) {ctx->filename,
                                     ctx->lexer->line,
                                     ctx->lexer->lineChar};
}

void TokenMatch (ParserCTX* ctx)
{
    DebugMsg("совпадение:%d:%d: '%s'", ctx->location.line, ctx->location.lineChar, ctx->lexer->buffer);
    TokenNext(ctx);
}

char* TokenDupMatch (ParserCTX* ctx)
{
    char* old = strdup(ctx->lexer->buffer);

    TokenMatch(ctx);
    return old;
}

static char* TokenTagGetStr (TOKEN_TAG tag)
{
    if (tag == TOK_UNDEFINED) return "<undefined>";
    else if (tag == TOK_OTHER) return "other";
    else if (tag == TOK_EOF) return "end of file";
    else if (tag == TOK_KEYWORD) return "keyword";
    else if (tag == TOK_IDENT) return "identifier";
    else if (tag == TOK_INT) return "integer";
    else if (tag == TOK_STR) return "string";
    else if (tag == TOK_CHR) return "character";
    else {
        char* str = malloc(logi(tag, 10)+2);
        sprintf(str, "%d", tag);
        DebugErrorUnhandled("tokenTagGetStr", "token tag", str);
        free(str);
        return "<unhandled>";
    }
}