#ifndef X_INCLUDE_UTIL
#define X_INCLUDE_UTIL

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

int LogI (int x, int base);
char* StrJoinWith (char** strs, int n, const char* separator, void* (*allocator)(size_t));
#endif /*X_INCLUDE_UTIL*/