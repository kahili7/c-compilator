#include <stdlib.h>
#include <string.h>

#include "..\include\vector.h"

Vector* VectorInit (Vector* v, int initialCapacity)
{
    v->length = 0;
    v->capacity = initialCapacity;
    v->buffer = malloc(initialCapacity*sizeof(void*));
    return v;
}

void VectorFree (Vector* v)
{
    free(v->buffer);
    v->length = 0;
    v->capacity = 0;
    v->buffer = 0;
}

void VectorFreeObjs (Vector* v, VectorDtor dtor)
{
    VectorMap(v, (VectorMapper) dtor, v);
    VectorFree(v);
}

static void VectorResize (Vector* v, int size) {
    v->capacity = size;
    v->buffer = realloc(v->buffer, v->capacity*sizeof(void*));
}

int VectorPush (Vector* v, void* item)
{
    if (v->length == v->capacity) VectorResize(v, v->capacity*2);

    v->buffer[v->length] = item;
    return v->length++;
}

void* VectorPop (Vector* v)
{
    return v->buffer[--v->length];
}

Vector* VectorPushFromArray (Vector* v, void** array, int length, int elementSize)
{
    if (v->capacity < v->length + length) VectorResize(v, v->capacity + length*2);

    if (elementSize == sizeof(void*)) memcpy(v->buffer + v->length, array, length*elementSize);
    else
        for (int i = 0; i < length; i++)
            memcpy(v->buffer+i, (char*) array + i*elementSize, elementSize);

    v->length += length;
    return v;
}

Vector* vectorPushFromVector (Vector* dest, const Vector* src)
{
    return VectorPushFromArray(dest, src->buffer, src->length, sizeof(void*));
}

int VectorFind (Vector* v, void* item)
{
    for (int i = 0; i < v->length; i++)
    {
        if (v->buffer[i] == item) return i;
    }

    return -1;
}

void* VectorRemoveReorder (Vector* v, int n)
{
    if (v->length <= n) return 0;

    void* last = VectorPop(v);
    VectorSet(v, n, last);
    return last;
}

void* VectorGet (const Vector* v, int n)
{
    if (n < v->length && n >= 0)
        return v->buffer[n];
    else
        return 0;
}

int VectorSet (Vector* v, int n, void* value)
{
    if (n < v->length)
    {
        v->buffer[n] = value;
        return 0;
    }
    else
        return 1;
}

void VectorMap (Vector* dest, VectorMapper f, Vector* src)
{
    int upto = src->length > dest->capacity ? dest->capacity : src->length;

    for (int n = 0; n < upto; n++)
        dest->buffer[n] = f(src->buffer[n]);

    dest->length = upto;
}