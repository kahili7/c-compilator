#ifndef X_INCLUDE_REGISTER
#define X_INCLUDE_REGISTER

typedef enum REG_INDEX {
    REG_UNDEFINED,
    REG_RAX,
    REG_RBX,
    REG_RCX,
    REG_RDX,
    REG_RSI,
    REG_RDI,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15,
    REG_RBP,
    REG_RSP,
    REG_MAX
} REG_INDEX;

typedef struct Register {
    int size;   //минимальный размер в байтах 
    const char* names[4];   //байт, слово, двойное слово, четверное слово
    int allocatedAs;        //если неиспользованный то 0, в противном случае выделенный размер в байтах
} Register;

extern Register Regs[REG_MAX];

const char* RegGetStr (const Register* r);
const char* RegIndexGetName (REG_INDEX r, int size);

Register* RegAlloc (int size);
void RegFree (Register* r);

Register* RegRequest (REG_INDEX r, int size);
const Register* RegGet (REG_INDEX r);
int RegIsUsed (REG_INDEX r);

#endif /*X_INCLUDE_REGISTER*/