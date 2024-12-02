#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>

typedef struct hashTableNode {
    char *name;
    void *data;
    size_t dataSize;
    struct hashTableNode *next;
} hashTableNode_t;

typedef struct {
    size_t size;
    hashTableNode_t **elements;
} hashTable_t;

enum hashTableStatus {
    HASH_TABLE_SUCCESS,
    HASH_TABLE_ERROR
};

hashTable_t hashTableCtor(size_t size);
enum hashTableStatus hashTableDtor(hashTable_t *table);

/// @brief Insert element in hash table. If element with given name already exists, nothing is done
hashTableNode_t *hashTableInsert(hashTable_t *table, const char *name, const void *elem, size_t elemSize);

void *hashTableFind(hashTable_t *table, const char *name);

enum hashTableStatus hashTableRemove(hashTable_t *table, const char *name);
enum hashTableStatus hashTableClear(hashTable_t *table);

enum hashTableStatus hashTableDump(hashTable_t *table);
enum hashTableStatus hashTableVerify(hashTable_t *table);

#if defined(HASH_TABLE_DEBUG) && !defined(NDEBUG)
    #define hashTableDbg(...) __VA_ARGS__
    #define hashTablePrint(...) fprintf(stderr, __VA_ARGS__);
#else
    #define hashTableDbg(...)
    #define hashTablePrint(...)
#endif

#endif
