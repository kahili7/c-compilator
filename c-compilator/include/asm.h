#ifndef X_INCLUDE_ASM
#define X_INCLUDE_ASM

#include "..\include\vector.h"
#include "..\include\operand.h"

typedef enum LABEL_TAG {
    LABEL_Return,
    LABEL_Else,
    LABEL_EndIf,
    LABEL_While,
    LABEL_For,
    LABEL_Continue,
    LABEL_Break,
    LABEL_ShortCircuit,
    LABEL_ROData,
    LABEL_Lambda,
    LABEL_PostLambda
} LABEL_TAG;

//контекст ассемблера
typedef struct AsmCTX {
    char* filename;     //имя файла
    FILE* file;         //дескриптор файла
    
    int depth;          //глубина отступа
    int lineNo;         //номер строки

    const Arch* arch;   //размерность регистров 

    Operand stackPtr;   //указатель на вершину стек
    Operand basePtr;    //указатель на базу стека
} AsmCTX;

AsmCTX* AsmInit (const char* output, const Arch* arch);
void AsmEnd (AsmCTX* ctx);
void AsmOutLn (AsmCTX* ctx, const char* format, ...);
void AsmEnter (AsmCTX* ctx);
void AsmLeave (AsmCTX* ctx);
#endif /*X_INCLUDE_ASM*/