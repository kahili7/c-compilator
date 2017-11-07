#ifndef X_INCLUDE_ARCH
#define X_INCLUDE_ARCH

#include "..\include\vector.h"
#include "..\include\symbol.h"

typedef void (*ArchSymbolMangler)(Symbol*);

typedef enum OS_TAG {
    OS_LINUX,
    OS_WINDOWS
} OS_TAG;

typedef struct Arch {
    int wordsize;           //размер слова - зависит от архитектуры
    
    Vector scratchRegs;
    Vector calleeSaveRegs;  //список регистров для сохранения
    
    char *asflags;
    char *ldflags;
    
    ArchSymbolMangler symbolMangler;
} Arch;

void ArchInit (Arch* arch);
void ArchFree (Arch* arch);
void ArchSetup (Arch* arch, OS_TAG os, int wordsize);
#endif /*X_INCLUDE_ARCH*/