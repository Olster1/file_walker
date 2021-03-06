#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#if !defined arrayCount
#define arrayCount(array1) (sizeof(array1) / sizeof(array1[0]))
#endif 

#if !defined invalidCodePathStr
#define invalidCodePathStr(msg) { printf(msg); exit(0); }
#endif

#if !defined assert 
#if 1 //easy assert
#define assert(statement) if(!(statement)) {printf("Something went wrong at %d", __LINE__); exit(0);}
#else 
#define assert(statement) if(!(statement)) { int *i_ = 0; *i_ = 0; }
#endif
#endif

typedef struct Pool Pool;
typedef struct Pool {
    unsigned char *memory;
    unsigned int count;
    unsigned int indexAt;
    
    u64 inValid; //size of pool can only be max 64 slots 
    unsigned int id;  //unique to this array. Just an index;
    Pool *next;
} Pool;

typedef struct ValidIndex ValidIndex; 
typedef struct ValidIndex {
    int index;
    Pool *pool; 
    
    ValidIndex *next;
    ValidIndex *prev;
} ValidIndex;

#define INCREMENT_COUNT 64
typedef struct {
    size_t sizeofType;
    Pool *poolHash[1096]; //should this be a hash table;
    Pool *latestPool; 
    u32 poolIdAt;
    
    int count;
    int indexAt;
    
    ValidIndex freeIndexesSent;
    ValidIndex *freeList;
} Array_Dynamic;


typedef struct {
    void *memory;
    
    int count;
    int totalCount;
    
    int sizeOfMember;
} InfiniteAlloc;

void expandMemoryArray_(InfiniteAlloc *arena, int count) {
    DEBUG_TIME_BLOCK()
    if((arena->count + count) >= arena->totalCount) {
        int newCount = arena->totalCount + count + 1028;
        void *newMem = calloc(newCount*arena->sizeOfMember, 1);
        assert(newMem);
        
        if(arena->memory) {
            memcpy(newMem, arena->memory, arena->count*arena->sizeOfMember);
            free(arena->memory);
        }
        arena->memory = newMem;
        arena->totalCount = newCount;
    }
}

#define expandMemoryArray(arena) expandMemoryArray_(arena, 1)

#define initInfinteAlloc(member) initInfinteAlloc_(sizeof(member)) 
InfiniteAlloc initInfinteAlloc_(int sizeOfMember) {
    DEBUG_TIME_BLOCK()
    InfiniteAlloc result = {0};
    result.sizeOfMember = sizeOfMember;
    
    return result;
}

#define addElementInifinteAlloc_(arena, data) addElementInifinteAllocWithCount_(arena, data, 1)

#define addElementInfinteAlloc_notPointer(arena, data) addElementInifinteAllocWithCount_(arena, (void *)&data, 1)


#define getElementFromAlloc(arena, index, type) (type *)getElementFromAlloc_(arena, index);
void *getElementFromAlloc_(InfiniteAlloc *arena, int index)  {
    assert(index >= 0 && index < arena->count); 
    u8 *memAt = ((u8 *)arena->memory) + (arena->sizeOfMember*index);
    return memAt;
}

void *addElementInifinteAllocWithCount_(InfiniteAlloc *arena, void *data, int count) {
    DEBUG_TIME_BLOCK()
    assert(arena->sizeOfMember > 0);
    expandMemoryArray_(arena, count);
    assert((arena->count + count) < arena->totalCount);
    u8 *memAt = (u8 *)arena->memory + (arena->sizeOfMember*arena->count);
    arena->count += count;
    if(data) {
        memcpy(memAt, data, arena->sizeOfMember*count);
    } else {
        memset(memAt, 0, arena->sizeOfMember*count);
    }
    return memAt;
}



void releaseInfiniteAlloc(InfiniteAlloc *arena) {
    DEBUG_TIME_BLOCK()
    if(arena->memory) {
        free(arena->memory);
        memset(arena, 0, sizeof(InfiniteAlloc));
    }
}

bool isInfinteAllocActive(InfiniteAlloc *arena) {
    DEBUG_TIME_BLOCK()
    return (bool)arena->memory;
}

Pool *initPool(Array_Dynamic *array, size_t sizeofType, unsigned int id) {
    DEBUG_TIME_BLOCK()
    Pool *pool = (Pool *)calloc(sizeof(Pool), 1);
    memset(pool, 0, sizeof(Pool));

    pool->count = INCREMENT_COUNT;
    size_t initialSizeBytes = sizeofType*pool->count;
    pool->memory = (unsigned char *)malloc(initialSizeBytes);
    memset(pool->memory, 0, initialSizeBytes);
    pool->id = id;
    
    //add it to the the hash table
    Pool **poolHashPtr = array->poolHash + id;
    while(*poolHashPtr) {
        poolHashPtr = &((*poolHashPtr)->next);
    } 
    assert(!(*poolHashPtr));
    
    *poolHashPtr = pool;
    //
    
    return pool;
}

#define initArray(array, type) initArray_(array, sizeof(type))
void initArray_(Array_Dynamic *array, size_t sizeofType) {
    DEBUG_TIME_BLOCK()
    memset(array, 0, sizeof(Array_Dynamic));
    array->sizeofType = sizeofType;
    //set up sentinel
    ValidIndex *sent = &array->freeIndexesSent;
    array->freeIndexesSent.next = array->freeIndexesSent.prev = sent;
    assert(array->freeIndexesSent.next == &array->freeIndexesSent);
    ////
}

bool isElmValid(Pool *pool, u64 index) {
    DEBUG_TIME_BLOCK()
   
    bool result = !(pool->inValid & (u64)((u64)1 << (u64)index));
    return result;
}

#define addElement(array, data) addElement_(array, &data, sizeof(data))

int addElement_(Array_Dynamic *array, void *elmData, size_t sizeofData) {
    DEBUG_TIME_BLOCK()
    assert(sizeofData == array->sizeofType);
    
    Pool *pool = array->latestPool;
    
    if(!pool) { //we don't set any pools up to begin with to save any memory being used. (low cost to use them!) Oliver 20/02/18
        assert(array->poolIdAt == 0);
        pool = array->latestPool = initPool(array, array->sizeofType, array->poolIdAt++);
    }
    
    int indexAt;

    if(array->freeIndexesSent.next != &array->freeIndexesSent) { //something on the list 
        ValidIndex *validIndex = array->freeIndexesSent.next; 
        indexAt = validIndex->index;
        pool = validIndex->pool;
        assert(pool);
        //remove from linked list 
        assert(validIndex);
        assert(validIndex != &array->freeIndexesSent);
        assert(validIndex->prev == &array->freeIndexesSent);
        array->freeIndexesSent.next = validIndex->next; //move to next item; 
        validIndex->next->prev = validIndex->prev;
        
        
        assert(pool);
        //put on free list
        validIndex->prev = 0;
        validIndex->next = array->freeList;
        array->freeList = validIndex;
        assert(!isElmValid(pool, indexAt));
    } else {
        if(pool->indexAt >= pool->count) {
            //add new pool 
            pool = array->latestPool = initPool(array, array->sizeofType, array->poolIdAt++);
            printf("%s\n", "added new pool");
        } 
        printf("%d\n", pool->indexAt);
        printf("%d\n", pool->count);
        //add element
        assert(pool->indexAt < pool->count);
        
        indexAt = pool->indexAt++;
        array->count++;
    }
    
    void *at = ((unsigned char *)pool->memory) + (array->sizeofType*indexAt);
    
    if(elmData) {
        memcpy(at, elmData, array->sizeofType);
    }
    
    assert(indexAt < INCREMENT_COUNT);
    if(indexAt > 32) {
        int i = 0;
    }

    pool->inValid &= (u64)(~((u64)1 << (u64)indexAt));
    
    int absIndex = (pool->id*INCREMENT_COUNT) + indexAt;

    if(absIndex >= array->count) {
        array->count = absIndex + 1;
    }
    
    return absIndex;
}

Pool *getPool(Array_Dynamic *array, int poolIndex) {
    DEBUG_TIME_BLOCK()
    //hash table would avoid looking at all the arrays
    Pool *result = array->poolHash[poolIndex % arrayCount(array->poolHash)];
    while(result) {
        if(result->id == poolIndex) {
            break;
        }
        result = result->next;
    }
    
    return result;
}

typedef struct {
    Pool *pool;
    int indexAt;
} PoolInfo;

PoolInfo getPoolInfo(Array_Dynamic *array, int absIndex) {
    DEBUG_TIME_BLOCK()
    int poolAt = absIndex / INCREMENT_COUNT;
    // assert(absIndex < INCREMENT_COUNT);
    
    PoolInfo result = {0};
    result.pool = getPool(array, poolAt);
    result.indexAt = absIndex - (poolAt*INCREMENT_COUNT);
    assert(result.indexAt >= 0);
    return result;
}



void *getElement(Array_Dynamic *array, unsigned int absIndex) {
    DEBUG_TIME_BLOCK()
    void *elm = 0;
    PoolInfo info = getPoolInfo(array, absIndex);
    if(info.pool && info.indexAt < (info.pool->indexAt) && info.indexAt >= 0 && isElmValid(info.pool, info.indexAt)) {
        
        elm = info.pool->memory + (info.indexAt*array->sizeofType);
    } else {
        // assert(false);
    }
    return elm;
}

void *getEmptyElement(Array_Dynamic *array) {
    DEBUG_TIME_BLOCK()
    int index = addElement_(array, 0, array->sizeofType);
    void *result = getElement(array, index);
    memset(result, 0, array->sizeofType);
    return result;
}

typedef struct {
    void *elm; 
    int absIndex;
} ArrayElementInfo;

ArrayElementInfo getEmptyElementWithInfo(Array_Dynamic *array) {
    DEBUG_TIME_BLOCK()
    ArrayElementInfo result = {0};
    result.absIndex = addElement_(array, 0, array->sizeofType);
    result.elm = getElement(array, result.absIndex);
    return result;
}

void removeElement_unordered(Array_Dynamic *array, int absIndex) {
    DEBUG_TIME_BLOCK()
    PoolInfo info = getPoolInfo(array, absIndex);
    
    Pool *pool = info.pool;
    int index = info.indexAt;
    if(pool && index < pool->indexAt && index >= 0 && isElmValid(info.pool, info.indexAt)) {
        
        void *elm = pool->memory + (index*array->sizeofType);
        unsigned int lastIndex = --pool->indexAt;
        if(lastIndex != index) {
            assert(index < lastIndex);
            void *elm2 = pool->memory + (lastIndex*array->sizeofType);
            //get element from the end. 
            memcpy(elm, elm2, array->sizeofType);
        }
        assert(info.indexAt < INCREMENT_COUNT);
        pool->inValid |= (u64)((u64)1 << (u64)info.indexAt);
        array->count--;
    } else {
        invalidCodePathStr("index not valid");
    }
    
}

//
void *getLastElement(Array_Dynamic *array) { //returns the last element on the list. [array->count - 1]
    DEBUG_TIME_BLOCK()
    void *lastElm = getElement(array, array->count - 1);
    assert(lastElm);
    return lastElm;
}

//This is when we want to keep the order consistent. i.e. we're using indexes as ids etc. 
void removeElement_ordered(Array_Dynamic *array, int absIndex) {
    DEBUG_TIME_BLOCK()
    PoolInfo info = getPoolInfo(array, absIndex);
    if(info.pool && info.indexAt < info.pool->indexAt && info.indexAt >= 0 && isElmValid(info.pool, info.indexAt)) {
        
        ValidIndex *validInd = 0;
        if(array->freeList) {
            //get off free list
            validInd = array->freeList;
            array->freeList = validInd->next;
        } else {
            //alloc new valid index
            validInd = (ValidIndex *)calloc(sizeof(ValidIndex), 1);
        }
        assert(validInd);
        //assgin info
        validInd->index = info.indexAt;
        validInd->pool = info.pool;
        if(info.indexAt > 32) {
            int i = 0;
        }
        validInd->pool->inValid |= (u64)((u64)1 << (u64)info.indexAt);
        
        //
        validInd->prev = array->freeIndexesSent.prev;
        validInd->next = &array->freeIndexesSent;

        //append to end of list. So we can pull off from the beginning. 
        array->freeIndexesSent.prev->next = validInd;
        array->freeIndexesSent.prev = validInd;
        printf("removed thing\n");

        if(absIndex == array->count - 1) {
            // is last member on the array. 
            array->count--;
        }
    } else {
        assert(false);
    }
}

typedef enum {
    REMOVE_ORDERED,
    REMOVE_UNORDERED,

} RemoveType;

void removeSectionOfElements(Array_Dynamic *array, RemoveType type, int min, int max) {
    DEBUG_TIME_BLOCK()
    for(int i = min; i < max; ++i) {
        if(type == REMOVE_ORDERED) {
            removeElement_ordered(array, i);
        } else {
            assert(type == REMOVE_UNORDERED);
            removeElement_unordered(array, i);
        }
    }
    if(max >= array->count) {
        array->count = min;
    }
    printf("%d\n", array->count);
}


void freeArray(Array_Dynamic *array) {
    DEBUG_TIME_BLOCK()
    //also free all the free lists and other lists.
    
    for(int i = 0; i < arrayCount(array->poolHash); i++) {
        Pool *pool = array->poolHash[i];
        while(pool) {
            Pool *nextPool = pool->next;
            free(pool);
            pool = nextPool;
        }    
    }
    
    ValidIndex *currentIndex = array->freeIndexesSent.next;
    while(currentIndex != &array->freeIndexesSent) {
        ValidIndex *nextIndex = currentIndex->next;
        free(currentIndex);
        currentIndex = nextIndex;
    }
    
    currentIndex = array->freeList;
    while(currentIndex) {
        ValidIndex *nextIndex = currentIndex->next;
        free(currentIndex);
        currentIndex = nextIndex;
    }
    
}


