#ifndef X_INCLUDE_HASHMAP
#define X_INCLUDE_HASHMAP

typedef struct GHashMap {
    int size;       //количество элементов
    int elements;
    
    union {
        const char** keysStr;
        /*We don't know whether the user intends the map to take ownership of
          keys (freed with FreeObjs), so provide a mutable version*/
        char** keysStrMutable;
        intptr_t* keysInt;  //
    };
    
    int* hashes;
    void** values;  //массив значений
} GHashMap;

typedef GHashMap HashMap;
typedef GHashMap IntMap;

typedef GHashMap HashSet;
typedef GHashMap IntSet;

typedef void (*hashmapKeyDtor)(char* key, const void* value);
typedef void (*hashmapValueDtor)(void* value);


int IntSetTest (const IntSet* set, intptr_t element);
void IntSetMerge (IntSet* dest, const IntSet* src);
int IntSetAdd (IntSet* set, intptr_t element);
void IntSetFree (IntSet* set);
IntSet* IntSetInit (IntSet* set, int size);


typedef void (*hashsetDtor)(char* element);

int HashSetTest (const HashSet* set, const char* element);
void HashSetMergeDup (HashSet* dest, const HashSet* src);
void HashSetMerge (HashSet* dest, HashSet* src);
int HashSetAdd (HashSet* set, const char* element);
void HashSetFreeObjs (HashSet* set, hashsetDtor dtor);
void HashSetFree (HashSet* set);
HashSet* HashSetInit (HashSet* set, int size);
#endif /*X_INCLUDE_HASHMAP*/