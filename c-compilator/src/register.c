#include "..\include\register.h"
#include "..\include\debug.h"

Register Regs[REG_MAX] = {
    {1, {"undefined", "undefined", "undefined", "undefined"}, 0},
    {1, {"al", "ax", "eax", "rax"}, 0},
    {1, {"al", "bx", "ebx", "rbx"}, 0},
    {1, {"cl", "cx", "ecx", "rcx"}, 0},
    {1, {"dl", "dx", "edx", "rdx"}, 0},
    {2, {0, "si", "esi", "rsi"}, 0},
    {2, {0, "di", "edi", "rdi"}, 0},
    {8, {0, 0, 0, "r8"}, 0},
    {8, {0, 0, 0, "r9"}, 0},
    {8, {0, 0, 0, "r10"}, 0},
    {8, {0, 0, 0, "r11"}, 0},
    {8, {0, 0, 0, "r12"}, 0},
    {8, {0, 0, 0, "r13"}, 0},
    {8, {0, 0, 0, "r14"}, 0},
    {8, {0, 0, 0, "r15"}, 0},
    {2, {0, "bp", "ebp", "rbp"}, 0},
    {2, {0, "sp", "esp", "rsp"}, 0}
};

int RegIsUsed (REG_INDEX r)
{
    return Regs[r].allocatedAs != 0;
}

const Register* RegGet (REG_INDEX r)
{
    return &Regs[r];
}

Register* RegRequest (REG_INDEX r, int size)
{
    if (size == 0)
    {
        DebugError("RegRequest", "zero sized register requested, %s", RegIndexGetName(r, 8));
        size = Regs[r].size == 8 ? 8 : 4;
    }

    if (Regs[r].allocatedAs == 0 && Regs[r].size <= size)
    {
        Regs[r].allocatedAs = size;
        return &Regs[r];
    }
    else return 0;
}

void RegFree (Register* r)
{
    r->allocatedAs = 0;
}

Register* RegAlloc (int size)
{
    if (size == 0)
        return 0;

    /*Bugger RAX. Functions put their rets in there, so its just a hassle*/
    for (REG_INDEX r = REG_RBX; r <= REG_R15; r++)
    {
        if (RegRequest(r, size) != 0)
            return &Regs[r];
    }
    
    if (RegIsUsed(REG_RAX)) DebugError("RegAlloc", "no registers left");

    return RegRequest(REG_RAX, size);
}

const char* RegIndexGetName (REG_INDEX r, int size)
{
    return RegGetName(&Regs[r], size);
}

const char* RegGetStr (const Register* r)
{
    return RegGetName(r, r->allocatedAs);
}


static const char* RegGetName (const Register* r, int size)
{
    if (size == 1)
        return r->names[0];

    else if (size == 2)
        return r->names[1];

    else if (size == 4)
        return r->names[2];

    else if (size == 8)
        return r->names[3];

    else {
        DebugErrorUnhandledInt("RegGetName", "register size", size);
        return "unhandled";
    }
}