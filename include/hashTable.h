#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <stdio.h>

#define errprintf(...) fprintf(stderr, __VA_ARGS__)
#define TODO(msg) do {errprintf("TODO at %s:%d : %s", __FILE__, __LINE__, msg); abort();} while(0)
#ifndef NDEBUG
    #define HDBG(...) __VA_ARGS__
    #define hprintf(...) fprintf(stderr, __VA_ARGS__)
#else
    #define HDBG(...)
    #define hprintf(...)
#endif

typedef struct hashTableNode {
    struct hashTableNode *next;
    void  *value;
    char  *key;
} hashTableNode_t;

typedef uint64_t hash_t;
typedef hash_t (*hashFunc_t)(const void *ptr);

typedef struct hashTable {
    hashTableNode_t *buckets;
    size_t bucketsCount;

    size_t valSize;

    size_t size;

    hashFunc_t hash;
    HDBG(int (*printElem)(const void *ptr);)
} hashTable_t;

HDBG(

static int printInt(const void *ptr) {return errprintf("%d", *(const int *)ptr);}
static int printStr(const void *ptr) {return errprintf("%s", (const char*)ptr);}
static int printFloat(const void *ptr) {return errprintf("%f", *(const float*)ptr);}

)

typedef enum hashTableStatus {
    HT_SUCCESS      = 0,
    HT_MEMORY_ERROR = 1,
    HT_WRONG_HASH   = 2,
    HT_WRONG_SIZE   = 3,
    HT_NO_INIT      = 4,
    HT_NO_KEY       = 5,
    HT_NO_VALUE     = 6,
    HT_ERROR
} hashTableStatus_t;


hashTableStatus_t hashTableCtor(hashTable_t *table, size_t valueSize, size_t bucketsCount, hashFunc_t hash);
hashTableStatus_t hashTableDtor(hashTable_t *table);

/// @brief Insert element in hashTable or rewrite it's value if already inserted
hashTableStatus_t hashTableInsert(hashTable_t *table, const char *key, const void *value);

/// @brief Access element in hash table or insert it with default value
/// @return Ptr to element or NULL in case of error
void *hashTableAccess(hashTable_t *table, const char *key);

void *hashTableFind(hashTable_t *table, const char *key);

hashTableStatus_t hashTableVerify(hashTable_t *table);
hashTableStatus_t hashTableDump(hashTable_t *table);
hashTableStatus_t hashTableCalcDistribution(hashTable_t *table);


/*========================== Debugging tools =================================*/
static void htStackTrace(const char *file, int line, const char *function);

static void htStackTrace(const char *file, int line, const char *function) {
    static int callCounter = 0;

    callCounter++;

    if (callCounter == 1) {
        hprintf("Error occured in hashTable\n");
        hprintf("Stack trace:\n");
    }

    hprintf("#%2d at %s:%d in %s \n", callCounter, file, line, function);

    return;
}

#define _ERR_RET(expr) \
    {                                           \
        hashTableStatus_t status = (expr);      \
        if (status != HT_SUCCESS) {             \
            htStackTrace(__FILE__, __LINE__, __PRETTY_FUNCTION__); \
            return status;                      \
        }                                       \
    }


#define _ERR_RET_PTR(expr) \
    {                                           \
        hashTableStatus_t status = (expr);      \
        if (status != HT_SUCCESS) {             \
            htStackTrace(__FILE__, __LINE__, __PRETTY_FUNCTION__); \
            return NULL;                        \
        }                                       \
    }


// #define HASH_TABLE_VERIFY

#ifdef HASH_TABLE_VERIFY
    #define _VERIFY(table, ret) do {                             \
        hashTableStatus_t status = hashTableVerify(table);  \
        if (status != HT_SUCCESS) {                         \
            hashTableDump(table);                           \
            htStackTrace(__FILE__, __LINE__, __PRETTY_FUNCTION__); \
            return ret;                                     \
        }                                                   \
    } while(0)

#else
    #define _VERIFY(table, ret)
#endif

#endif
