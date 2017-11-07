#include <hashmap.h>

#include <string.h>
#include <stdlib.h>

// typedefs
typedef void (*hashmapKeyDtor)(char* key, const void* value);
typedef void (*hashmapValueDtor)(void* value);
typedef intptr_t (*hashmapHash)(const char* key, int mapsize);
//Like strcmp, returns 0 for match
typedef int (*hashmapCmp)(const char* actual, const char* key);
typedef char* (*hashmapDup)(const char* key);


static intptr_t HashStr (const char* key, int mapsize)
{
    intptr_t hash = 0;

    for (int i = 0; key[i]; i++)
    {
        hash += key[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    intptr_t mask = mapsize-1;
    return hash & mask;
}

static intptr_t HashInt (intptr_t element, int mapsize)
{
    intptr_t hash = element;
    
    hash += hash << 10;
    hash ^= hash >> 6;
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    intptr_t mask = mapsize-1;
    return hash & mask;
}

static int Pow2ize (int x)
{
    if (sizeof(x) <= 8) return -1;
    
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    /*No-op if x is less than 33 bits
      Shift twice to avoid warnings, as this is intended.*/
    x |= (x >> 16) >> 16;
    return (x + 1);
}


static GHashMap* GHashMapInit (GHashMap* map, int size, int hashes)
{
    size = Pow2ize(size);

    map->size = size;
    map->elements = 0;
    map->keysInt = calloc(size, sizeof(char*));
    map->hashes = hashes ? calloc(size, sizeof(int)) : 0;
    map->values = calloc(size, sizeof(char*));

    return map;
}

static void GHashMapFree (GHashMap* map, int hashes)
{
    free(map->keysInt);

    if (hashes)
        free(map->hashes);

    free(map->values);

    map->keysInt = 0;
    map->hashes = 0;
    map->values = 0;
}

static void GHashMapFreeObjs (GHashMap* map, hashmapKeyDtor keyDtor, hashmapValueDtor valueDtor, int hashes)
{
    for (int index = 0; index < map->size; index++)
    {
        if (map->values[index] == 0) continue;

        if (keyDtor)
            keyDtor(map->keysStrMutable[index], map->values[index]);

        if (valueDtor)
            valueDtor(map->values[index]);
    }

    GHashMapFree(map, hashes);
}

static int GHashMapIsMatch (const GHashMap* map, int index, const char* key, int hash, hashmapCmp cmp)
{
    if (cmp)
        return map->hashes[index] == hash && !cmp(map->keysStr[index], key);
    else
        return map->keysStr[index] == key;
}

static int GHashMapFind (const GHashMap* map, const char* key, int hash, hashmapCmp cmp)
{
    for (int index = hash; index < map->size; index++)
        if (map->values[index] == 0 || GHashMapIsMatch(map, index, key, hash, cmp))
            return index;

    for (int index = 0; index < hash; index++)
        if (map->values[index] == 0 || GHashMapIsMatch(map, index, key, hash, cmp))
            return index;

    return hash;
}

static int GHashMapAdd (GHashMap* map, const char* key, void* value, hashmapHash hashf, hashmapCmp cmp, int values)
{
    if (map->elements * 2 + 1 >= map->size)
    {
        GHashMap newmap;
        GHashMapInit(&newmap, map->size * 2, cmp != 0);
        GHashMapMerge(&newmap, map, hashf, cmp, 0, values);
        GHashMapFree(map, cmp != 0);
        *map = newmap;
    }

    int hash = hashf(key, map->size);
    int index = GHashMapFind(map, key, hash, cmp);
    int present;

    /*Empty spot*/
    if (map->values[index] == 0)
    {
        map->keysStr[index] = key;
        map->elements++;
        present = 0;

        if (cmp != 0) map->hashes[index] = hash;
    }
    else
    {
        if (GHashMapIsMatch(map, index, key, hash, cmp)) return -1;
        
        present = 1;
    }

    map->values[index] = values ? value : (void*) 1;
    return present;
}

static void GHashMapMerge (GHashMap* dest, const GHashMap* src, hashmapHash hash, hashmapCmp cmp, hashmapDup dup, int values)
 {
    for (int index = 0; index < src->size; index++)
    {
        if (src->keysInt[index] == 0) continue;

        char* key = src->keysStrMutable[index];

        if (dup) key = dup(key);

        GHashMapAdd(dest, key, values ? src->values[index] : 0, hash, cmp, values);
    }
}

static void* GHashMapMap (const GHashMap* map, const char* key, hashmapHash hashf, hashmapCmp cmp)
{
    int hash = hashf(key, map->size);
    int index = GHashMapFind(map, key, hash, cmp);
    
    return GHashMapIsMatch(map, index, key, hash, cmp) ? map->values[index] : 0;
}

static int GHashMapTest (const GHashMap* map, const char* key, hashmapHash hashf, hashmapCmp cmp)
{
    int hash = hashf(key, map->size);
    int index = GHashMapFind(map, key, hash, cmp);
    
    return GHashMapIsMatch(map, index, key, hash, cmp);
}

//--- intset ---
IntSet* IntSetInit (IntSet* set, int size)
{
    return GHashMapInit(set, size, 0);
}

void IntSetFree (IntSet* set)
{
    GHashMapFree(set, 0);
}

int IntSetAdd (IntSet* set, intptr_t element)
{
    return GHashMapAdd(set, (void*) element, 0, (hashmapHash) HashInt, 0, 0);
}

void IntSetMerge (IntSet* dest, const IntSet* src)
{
    GHashMapMerge(dest, src, (hashmapHash) HashInt, 0, 0, 0);
}

int IntSetTest (const IntSet* set, intptr_t element)
{
    return GHashMapTest(set, (void*) element, (hashmapHash) HashInt, 0);
}

//--- hashset ---
HashSet* HashSetInit (HashSet* set, int size)
{
    return GHashMapInit(set, size, 1);
}

void HashSetFree (HashSet* set)
{
    GHashMapFree(set, 1);
}

void HashSetFreeObjs (HashSet* set, hashsetDtor dtor)
{
    GHashMapFreeObjs(set, (hashmapKeyDtor) dtor, 0, 1);
}

int HashSetAdd (HashSet* set, const char* element)
{
    return GHashMapAdd(set, element, 0, HashStr, strcmp, 0);
}

void HashSetMerge (HashSet* dest, HashSet* src)
{
    GHashMapMerge(dest, src, HashStr, strcmp, 0, 0);
}

void HashSetMergeDup (HashSet* dest, const HashSet* src)
{
    GHashMapMerge(dest, src, HashStr, strcmp, strdup, 0);
}

int HashSetTest (const HashSet* set, const char* element)
{
    return GHashMapTest(set, element, HashStr, strcmp);
}