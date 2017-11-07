#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "..\include\lexer.h"

LexerCTX* LexerInit (const char* filename)
{
    LexerCTX* ctx = (LexerCTX *)malloc(sizeof(LexerCTX));
    ctx->stream = StreamInit(filename);
    ctx->line = 1;
    ctx->lineChar = 1;

    ctx->token = TOK_UNDEFINED;
    ctx->keyword = KEYWORD_UNDEFINED;
    ctx->punct = PUNCT_UNDEFINED;

    ctx->buffsz = 128;
    ctx->buffer = (char *)malloc(sizeof(char)*ctx->buffsz);
    return ctx;
}

void LexerEnd (LexerCTX* ctx)
{
    StreamEnd(ctx->stream);
    free(ctx->buffer);
    free(ctx);
}

static void LexerEat (LexerCTX* ctx, char c)
{
    if (ctx->length + 1 == ctx->buffsz) ctx->buffer = realloc(ctx->buffer, ctx->buffsz *= 2);

    ctx->buffer[ctx->length++] = c;
}

static void LexerEatNext (LexerCTX* ctx)
{
    LexerEat(ctx, StreamNext(ctx->stream));
}

static int LexerTryEatNext (LexerCTX* ctx, char c)
{
    if (ctx->stream->current == c)
    {
        LexerEatNext(ctx);
        return 1;
    } else return 0;
}

static void LexerSkipInsignificants (LexerCTX* ctx)
{
    while (1)
    {
        switch (ctx->stream->current)
        {
            /*Whitespace*/
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                StreamNext(ctx->stream);
                break;

            /*C preprocessor is treated as a comment*/
            case '#':
                /*Eat until a new line*/
                while (ctx->stream->current != '\n' && ctx->stream->current != 0)
                    StreamNext(ctx->stream);

                StreamNext(ctx->stream);
                break;

            /*Comment?*/
            case '/':
                StreamNext(ctx->stream);

                /*C comment*/
                if (ctx->stream->current == '*')
                {
                    StreamNext(ctx->stream);

                    do {
                        while (ctx->stream->current != '*' && ctx->stream->current != 0)
                            StreamNext(ctx->stream);

                        if (ctx->stream->current == 0)
                            break;

                        StreamNext(ctx->stream);
                    } while (ctx->stream->current != '/' && ctx->stream->current != 0);

                    StreamNext(ctx->stream);
                    
                /*C++ Comment*/
                }
                else if (ctx->stream->current == '/')
                {
                    StreamNext(ctx->stream);

                    while (ctx->stream->current != '\n' && ctx->stream->current != '\r' && ctx->stream->current != 0)
                        StreamNext(ctx->stream);

                /*Fuck, we just ate an important character. Backtrack!*/
                }
                else
                {
                    StreamNext(ctx->stream);
                    return;
                }

                break;

        /*Not insignificant, leave*/
        default:
            return;
        }
    }
}

static void LexerPunct (LexerCTX* ctx)
{
    ctx->token = TOK_PUNCT;

    switch (ctx->buffer[0])
    {
        case '{': ctx->punct = PUNCT_LBRACE; break;
        case '}': ctx->punct = PUNCT_RBRACE; break;
        case '(': ctx->punct = PUNCT_LPAREN; break;
        case ')': ctx->punct = PUNCT_RPAREN; break;
        case '[': ctx->punct = PUNCT_LBRACKET; break;
        case ']': ctx->punct = PUNCT_RBRACKET; break;
        case ';': ctx->punct = PUNCT_SEMICOLON; break;
        case '.':
            ctx->punct = PUNCT_PERIOD;

            if (ctx->stream->current == '.')
            {
                LexerEatNext(ctx);

                if (ctx->stream->current == '.')
                {
                    ctx->punct = PUNCT_ELLIPSIS;
                    LexerEatNext(ctx);
                /*Oops, it's just two dots, backtrack*/
                }
                else
                {
                    StreamPrev(ctx->stream);
                    ctx->length--;
                }
            }

            break;

        case ',': ctx->punct = PUNCT_COMMA; break;

        case '=': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_EQ : PUNCT_ASSIGN; break;
        case '!': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_NEQ : PUNCT_NOT; break;
        case '>': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_GE 
                                : LexerTryEatNext(ctx, '>') ? (LexerTryEatNext(ctx, '=') ? PUNCT_SHRASSIGN : PUNCT_SHR) : PUNCT_GE; break;
        case '<': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_LE
                                : LexerTryEatNext(ctx, '<') ? (LexerTryEatNext(ctx, '=') ? PUNCT_SHLASSIGN : PUNCT_SHL) : PUNCT_LE; break;

        case '?': ctx->punct = PUNCT_QUESTION; break;
        case ':': ctx->punct = PUNCT_COLON; break;

        case '&': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_ANDASSIGN : LexerTryEatNext(ctx, '&') ? PUNCT_ANDAND : PUNCT_AND; break;
        case '|': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_ORASSIGN : LexerTryEatNext(ctx, '|') ? PUNCT_OROR : PUNCT_OR; break;
        case '^': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_XORASSIGN : PUNCT_TILDE; break;
        case '~': ctx->punct = PUNCT_TILDE; break;

        case '+': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_PLUSASSIGN : LexerTryEatNext(ctx, '+') ? PUNCT_PLUSPLUS : PUNCT_PLUS; break;
        case '-': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_MINASSIGN
                               : LexerTryEatNext(ctx, '-') ? PUNCT_MINMIN
                               : LexerTryEatNext(ctx, '>') ? PUNCT_ARROW : PUNCT_MIN; break;
        case '*': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_MULASSIGN : PUNCT_MUL; break;
        case '/': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_DIVASSIGN : PUNCT_DIV; break;
        case '%': ctx->punct = LexerTryEatNext(ctx, '=') ? PUNCT_MODASSIGN : PUNCT_MOD; break;

        default: ctx->token = TOK_OTHER;
    }
}

static KEYWORD_TAG KeywordMatch (const char* str, int n, const char* look, KEYWORD_TAG kw)
{
    return !strcmp(str + n + 1, look + n + 1) ? kw : KEYWORD_UNDEFINED;
}

static KEYWORD_TAG KeywordMatch2 (const char* str, int n, const char* look, KEYWORD_TAG kw, const char* look2, KEYWORD_TAG kw2)
{
    return !strcmp(str + n + 1, look + n + 1) ? kw : !strcmp(str + n + 1, look2 + n + 1) ? kw2 : KEYWORD_UNDEFINED;
}

static KEYWORD_TAG LookKeyword (const char* str, int length)
{
    int longest = strlen("immutable")+1;

    if (length > longest) return KEYWORD_UNDEFINED;

    switch (str[0])
    {
        case 'd': return KeywordMatch(str, 0, "do", KEYWORD_DO);
        case 'r': return KeywordMatch(str, 0, "return", KEYWORD_RETURN);
        case 'w': return KeywordMatch(str, 0, "while", KEYWORD_WHILE);

        case 'a': return KeywordMatch(str, 0, "auto", KEYWORD_AUTO);
        case 'b': return KeywordMatch(str, 0, "break", KEYWORD_BREAK);
        case 't': return KeywordMatch(str, 0, "typedef", KEYWORD_BREAK);
        
        //case 'a': return keywordMatch2(str, 0, "assert", keywordAssert, "auto", keywordAuto);
        //case 'b': return keywordMatch2(str, 0, "bool", keywordBool, "break", keywordBreak);
        //case 't': return keywordMatch2(str, 0, "true", keywordTrue, "typedef", keywordTypedef);
        case 'u': return KeywordMatch(str, 0, "union", KEYWORD_UNION);

        case 'c':
            switch (str[1])
            {
                case 'h': return KeywordMatch(str, 1, "char", KEYWORD_CHAR);
                case 'o':
                    if (str[2] != 'n') return KEYWORD_UNDEFINED;

                    return KeywordMatch2(str, 2, "const", KEYWORD_CONST, "continue", KEYWORD_CONTINUE);

                default: return KEYWORD_UNDEFINED;
            }

        case 'e':
            switch (str[1])
            {
                case 'l': return KeywordMatch(str, 1, "else", KEYWORD_ELSE);
                case 'n': return KeywordMatch(str, 1, "enum", KEYWORD_ENUM);
                case 'x': return KeywordMatch(str, 1, "extern", KEYWORD_EXTERN);
                default: return KEYWORD_UNDEFINED;
            }

        case 'f': return KeywordMatch(str, 0, "for", KEYWORD_FOR);
        case 'i': return KeywordMatch2(str, 0, "if", KEYWORD_IF, "int", KEYWORD_INT);

        case 's':
            switch (str[1])
            {
                case 'i': return KeywordMatch(str, 1, "sizeof", KEYWORD_SIZEOF);
                case 't': return KeywordMatch2(str, 1, "static", KEYWORD_STATIC, "struct", KEYWORD_STRUCT);
                default: return KEYWORD_UNDEFINED;
            }

        case 'v':
            switch (str[1])
            {
                /*case 'a':
                    if (str[2] == '_')
                    {
                        switch (str[3]) {
                            case 'a': return keywordMatch(str, 3, "va_arg", keywordVAArg);
                            case 'c': return keywordMatch(str, 3, "va_copy", keywordVACopy);
                            case 'e': return keywordMatch(str, 3, "va_end", keywordVAEnd);
                            case 's': return keywordMatch(str, 3, "va_start", keywordVAStart);
                            default: return keywordUndefined;
                        }

                    } else
                        return keywordUndefined;*/

                case 'o': return KeywordMatch(str, 1, "void", KEYWORD_VOID);
                default: return KEYWORD_UNDEFINED;
            }

        default: return KEYWORD_UNDEFINED;
    }
}

void LexerNext (LexerCTX* ctx)
{
    if (ctx->token == TOK_EOF) return;

    LexerSkipInsignificants(ctx);

    ctx->line = ctx->stream->line;
    ctx->lineChar = ctx->stream->lineChar;

    ctx->length = 0;
    ctx->keyword = KEYWORD_UNDEFINED;
    ctx->punct = PUNCT_UNDEFINED;
    
    // конец потока
    if (ctx->stream->current == 0) ctx->token = TOK_EOF;

    /*Ident or keyword*/
    else if (isalpha(ctx->stream->current) || ctx->stream->current == '_')
    {
        LexerEatNext(ctx);

        while (isalnum(ctx->stream->current) || ctx->stream->current == '_')
            LexerEatNext(ctx);

        LexerEat(ctx, 0);

        ctx->keyword = LookKeyword(ctx->buffer, ctx->length);
        ctx->token = ctx->keyword != KEYWORD_UNDEFINED ? TOK_KEYWORD : TOK_IDENT;
        
    /*Number*/
    }
    else if (isdigit(ctx->stream->current))
    {
        ctx->token = TOK_INT;

        while (isdigit(ctx->stream->current))
            LexerEatNext(ctx);

    /*String/character*/
    }
    else if (ctx->stream->current == '"' || ctx->stream->current == '\'')
    {
        ctx->token = ctx->stream->current == '"' ? TOK_STR : TOK_CHR;
        StreamNext(ctx->stream);

        while (ctx->stream->current != (ctx->token == TOK_STR ? '"' : '\'') && ctx->stream->current != 0)
        {
            if (ctx->stream->current == '\\')
                LexerEatNext(ctx);

            LexerEatNext(ctx);
        }

        StreamNext(ctx->stream);

    /*Punctuation or an unrecognised character*/
    }
    else
    {
        LexerEatNext(ctx);

        /*Assume punctuation*/
        LexerPunct(ctx);
    }

    LexerEat(ctx, 0);
}