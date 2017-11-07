#include <stdio.h>
#include <stdarg.h>
#include <debug.h>

FILE* d_log;
DEBUG_MODE d_mode;
int d_depth;
int d_errors;

//установка режима отладки
DEBUG_MODE DebugSetMode (DEBUG_MODE nmode)
{
    DEBUG_MODE old = d_mode;
    d_mode = nmode;
    return old;
}

// инициализация отладки
void DebugInit (FILE* nlog)
{
    d_log = nlog;
    DebugSetMode(DEBUG_MINIMAL);
}

//запись переменных в файл
void DebugVarOut (const char* format, va_list args)
{
    (void) args, (void) format;

    #ifdef SCC_DEBUGMODE
    if (mode == DEBUG_SILENT) return;

    vfprintf(d_log, format, args);
    #endif
}

//общая запись в файл
void DebugOut (const char* format, ...)
{
    va_list args;
    va_start(args, format);
    DebugVarOut(format, args);
    va_end(args);
}

//запись сообщения в файл с глубиной
void DebugVarMsg (const char* format, va_list args)
{
    for (int i = 0; i < d_depth; i++)
        DebugOut("| ");

    DebugVarOut(format, args);
    DebugOut("\n");
}

//общая запись сообщения в файл с глубиной
void DebugMsg (const char* format, ...)
{
    va_list args;
    va_start(args, format);
    DebugVarMsg(format, args);
    va_end(args);
}

//добавить строку c глубиной
void DebugEnter (const char* str)
{
    if (d_mode <= DEBUG_COMPRESSED)
    {
        DebugMsg("+ %s", str);
        d_depth++;
    }
}

//удалить глубину
void DebugLeave ()
{
    if (d_mode <= DEBUG_COMPRESSED) d_depth--;
    if (d_mode == DEBUG_FULL) DebugMsg("-");
}

//ждать ввода
void DebugWait ()
{
    #ifdef SCC_DEBUGMODE
    if (d_mode <= DEBUG_FULL) getchar();
    #endif
}

void DebugError (const char* functionName, const char* format, ...)
{
    fprintf(d_log, "internal error(%s): ", functionName);

    va_list args;
    va_start(args, format);
    vfprintf(d_log, format, args);
    va_end(args);

    fputc('\n', d_log);

    DebugWait();
    d_errors++;
}

int DebugAssert (const char* functionName, const char* testName, int result)
{
    if (!result)
        DebugError(functionName, "%s assertion failed", testName);

    return !result;
}

void DebugErrorUnhandled (const char* functionName, const char* className, const char* classStr)
{
    DebugError(functionName, "необработанное %s: '%s'", className, classStr);
}

void DebugErrorUnhandledInt (const char* functionName, const char* className, int classInt)
{
    DebugError(functionName, "необработанное %s: %d", className, classInt);
}

void DebugErrorUnhandledChar (const char* functionName, const char* className, char classChar)
{
    DebugError(functionName, "необработанное %s: '%c'", className, classChar);
}