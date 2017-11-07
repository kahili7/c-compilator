#ifndef X_INCLUDE_LEXER
#define X_INCLUDE_LEXER

#include "stream.h"
#include "tokens.h"

typedef struct LexerCTX {
    StreamCTX *stream;
    int line;
    int lineChar;
    
    char *buffer;
    int buffsz;
    int length;
    
    TOKEN_TAG   token;
    KEYWORD_TAG keyword;
    PUNCT_TAG punct;
} LexerCTX;

LexerCTX* LexerInit (const char* filename);
void LexerEnd (LexerCTX* ctx);
void LexerNext (LexerCTX* ctx);
#endif /*X_INCLUDE_LEXER*/