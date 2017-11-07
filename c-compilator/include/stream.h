#ifndef X_INCLUDE_STREAM
#define X_INCLUDE_STREAM

#include <stdio.h>

typedef struct StreamCTX {
    FILE *file;
    
    char current;
    int line;
    int lineChar;
    
} StreamCTX;

StreamCTX* StreamInit (const char* filename);
void StreamEnd (StreamCTX* ctx);

char StreamNext (StreamCTX* ctx);
char StreamPrev (StreamCTX* ctx);


#endif /*X_INCLUDE_STREAM*/