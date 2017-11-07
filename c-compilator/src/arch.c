#include "..\include\arch.h"
#include "..\include\symbol.h"
#include "..\include\register.h"

void ArchInit (Arch* arch)
{
    arch->wordsize = 0;

    VectorInit(&arch->scratchRegs, 4);
    VectorInit(&arch->calleeSaveRegs, 4);
    arch->asflags = 0;
    arch->ldflags = 0;
    
    arch->symbolMangler = 0;
}

void ArchFree (Arch* arch)
{
    VectorFree(&arch->scratchRegs);
    VectorFree(&arch->calleeSaveRegs);

    free(arch->asflags);
    free(arch->ldflags);

    arch->asflags = 0;
    arch->ldflags = 0;
}

void ArchSetup (Arch* arch, OS_TAG os, int wordsize)
{
    arch->wordsize = wordsize;

    /*сохранеие регистров для вызова*/
    ArchSetupRegs(arch, os);

    if (os == OS_LINUX) arch->symbolMangler = ManglerLinux;
    else if (os == OS_WINDOWS)  arch->symbolMangler = ManglerWindows;
    else
    {
        DebugErrorUnhandledInt("ArchSetup", "OS", (int) os);
        arch->symbolMangler = ManglerLinux;
    }

    /*флаги для AS/LD*/
    ArchSetupDriverFlags(arch, os);
}

//внутренние функции
static void ManglerLinux (Symbol* Symbol)
{
    Symbol->label = strdup(Symbol->ident);
}

static void ManglerWindows (Symbol* Symbol)
{
    Symbol->label = malloc(strlen(Symbol->ident) + 2);
    
    sprintf(Symbol->label, "_%s", Symbol->ident);
}

static void ArchSetupRegs (Arch* arch, OS_TAG os)
{
    /*32-bit*/
    if (arch->wordsize == 4)
    {
        VectorPushFromArray(&arch->scratchRegs, (void**) (REG_INDEX[3]) {REG_RAX, REG_RCX, REG_RDX}, 3, sizeof(REG_INDEX));
        VectorPushFromArray(&arch->calleeSaveRegs, (void**) (REG_INDEX[3]) {REG_RBX, REG_RSI, REG_RDI}, 3, sizeof(REG_INDEX));
    /*64-bit*/
    }
    else if (arch->wordsize == 8)
    {
        REG_INDEX scratchRegs[7] = {REG_RAX, REG_RCX, REG_RDX, REG_R8, REG_R9, REG_R10, REG_R11};
        REG_INDEX calleeSaveRegs[5] = {REG_RBX, REG_R12, REG_R13, REG_R14, REG_R15};

        VectorPushFromArray(&arch->scratchRegs, (void**) scratchRegs, sizeof(scratchRegs)/sizeof(REG_INDEX), sizeof(REG_INDEX));
        VectorPushFromArray(&arch->calleeSaveRegs, (void**) calleeSaveRegs, sizeof(scratchRegs)/sizeof(REG_INDEX), sizeof(REG_INDEX));

        /*сохранить RDI & RSI*/
        Vector* RSIandRDI = os == OS_WINDOWS ? &arch->scratchRegs : &arch->calleeSaveRegs;
        
        VectorPush(RSIandRDI, (void*) REG_RSI);
        VectorPush(RSIandRDI, (void*) REG_RDI);
    }
    else
        DebugErrorUnhandledInt("ArchSetupRegs", "размер аппартного слова", arch->wordsize);
}

static void ArchSetupDriverFlags (Arch* arch, OS_TAG os)
{
    (void) os;

    if (arch->wordsize == 4)
    {
        arch->asflags = strdup("-m32");
        arch->ldflags = strdup("-m32");

    }
    else if (arch->wordsize == 8)
    {
        arch->asflags = strdup("-m64");
        arch->ldflags = strdup("-m64");

    }
    else {
        arch->asflags = strdup("");
        arch->ldflags = strdup("");
    }
}