#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int LogI (int x, int base) 
{
    int n = 0;

    while (x >= base) 
    {
        x /= base;
        n++;
    }

    return n;
}

char* StrJoinWith (char** strs, int n, const char* separator, void* (*allocator)(size_t))
{
    if (n <= 0) return calloc(1, sizeof(char));

    int seplen = strlen(separator);
    int firstlen = strlen(strs[0]);
    int length = firstlen + seplen * (n - 1) + 1;

    for (int i = 1; i < n; i++)
        length += strlen(strs[i]);

    char* joined = strcpy(allocator(length), strs[0]);
    int charno = firstlen;

    for (int i = 1; i < n; i++)
        charno += sprintf(joined + charno, "%s%s", separator, strs[i]);

    return joined;
}

char* StrJoin (char** strs, int n, void* (*allocator)(size_t))
{
    return StrJoinWith(strs, n, "", allocator);
}