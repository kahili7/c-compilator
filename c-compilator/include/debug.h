#ifndef X_INCLUDE_DEBUG
#define X_INCLUDE_DEBUG
#include <stdio.h>
#include <stdarg.h>

typedef enum DEBUG_MODE {
    DEBUG_FULL,
    DEBUG_COMPRESSED,
    DEBUG_MINIMAL,
    DEBUG_SILENT
} DEBUG_MODE;


void DebugWait ();
void DebugLeave ();
void DebugEnter (const char* str);
void DebugMsg (const char* format, ...);
void DebugVarMsg (const char* format, va_list args);
void DebugOut (const char* format, ...);
void DebugVarOut (const char* format, va_list args);
void DebugInit (FILE* nlog);
DEBUG_MODE DebugSetMode (DEBUG_MODE nmode);

void DebugErrorUnhandledChar (const char* functionName, const char* className, char classChar);
void DebugErrorUnhandledInt (const char* functionName, const char* className, int classInt);
void DebugErrorUnhandled (const char* functionName, const char* className, const char* classStr);
int DebugAssert (const char* functionName, const char* testName, int result);
void DebugError (const char* functionName, const char* format, ...);
#endif /*X_INCLUDE_DEBUG*/