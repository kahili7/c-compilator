#ifndef X_INCLUDE_PARSER_INTERNAL
#define X_INCLUDE_PARSER_INTERNAL

#include "lexer.h"

typedef struct TokenLocation {
    const char* filename;
    int line;
    int lineChar;
} TokenLocation;

typedef struct ParserCTX {
    LexerCTX *lexer;
    TokenLocation location;
    
    char* filename;
    const char* fullname;
    char* path;
    
    Symbol* module;
    Symbol* scope;
    
    int breakLevel;

    int errors;
    int warnings;

    int lastErrorLine;
};
#endif /*X_INCLUDE_PARSER_INTERNAL*/