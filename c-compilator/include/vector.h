#ifndef X_INCLUDE_VECTOR
#define X_INCLUDE_VECTOR

typedef struct Vector {
    int length;
    int capacity;
    void** buffer;
} Vector;

typedef void (*VectorDtor)(void*); ///For use with vectorFreeObjs
typedef void* (*VectorMapper)(void*); ///For use with vectorMap

Vector* VectorInit (Vector* v, int initialCapacity);

void VectorFree (Vector* v);
void VectorFreeObjs (Vector* v, void (*dtor)(void*));

int VectorPush (Vector* v, void* item);
Vector* VectorPushFromArray (Vector* v, void** array, int length, int elementSize);
Vector* VectorPushFromVector (Vector* dest, const Vector* src);
void* VectorPop (Vector* v);

int VectorFind (Vector* v, void* item);

void* VectorRemoveReorder (Vector* v, int n);

void* VectorGet (const Vector* v, int n);
int VectorSet (Vector* v, int n, void* value);

/**
 * Maps dest[n] to f(src[n]) for n in min(dest->length, src->length).
 */
void VectorMap (Vector* dest, void* (*f)(void*), Vector* src);
#endif /*X_INCLUDE_VECTOR*/