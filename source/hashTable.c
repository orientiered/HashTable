#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hashTable.h"


#define FREE(ptr) free(ptr); ptr = NULL
#define CALLOC(type, nmemb) (type *) calloc(nmemb, sizeof(type))

static hash_t djb2(const void *ptr) {
    const char *cptr = (const char *) ptr;

    hash_t hash = 5381;
    while (*cptr) {
        hash = ((hash << 5) + hash) + (hash_t)*cptr;
        cptr++;
        //hash = 33*hash + c
    }
    return hash;
}

// expects bucketsCount and valueSize to be set already
static hashTableStatus_t allocateBuckets(hashTable_t *table)
{
    assert(table);
    assert(table->bucketsCount > 0);

    hashTableNode_t *buckets = CALLOC(hashTableNode_t, table->bucketsCount);
    if (!buckets) {
        hprintf("Failed to allocate buckets array\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }

    table->buckets = buckets;

    return HT_SUCCESS;
}

static hashTableStatus_t deallocateBuckets(hashTable_t *table)
{
    assert(table);

    FREE(table->buckets);

    return HT_SUCCESS;
}

static hashTableStatus_t allocateNode(hashTable_t *table, const char *key, const size_t bucketIdx)
{
    assert(table);
    assert(key);

    assert(bucketIdx < table->bucketsCount);
    assert(table->buckets);

    hashTableNode_t *newNode = CALLOC(hashTableNode, 1);

    if (!newNode) {
        hprintf("Failed to allocate node\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }

    hashTableNode_t *headNode = table->buckets + bucketIdx;

    newNode->next  = headNode->next;
    headNode->next = newNode;

    size_t len   = strlen(key) + 1;
    newNode->key = CALLOC(char, len);
    if (!newNode->key) {
        hprintf("Failed to allocate memory for key\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }
    memcpy(newNode->key, key, len);

    newNode->value = calloc(table->valSize, 1);
    if (!newNode->value) {
        hprintf("Failed to allocate memory for value\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }

    return HT_SUCCESS;
}

static hashTableStatus_t deallocateNode(hashTable_t *table, hashTableNode_t *node)
{
    assert(table);
    assert(node);

    free(node->value);
    free(node->key);
    free(node);

    return HT_SUCCESS;
}

hashTableStatus_t hashTableCtor(hashTable_t *table, size_t valueSize, size_t bucketsCount, hashFunc_t hash)
{
    assert(table);
    assert(bucketsCount > 0);

    table->bucketsCount = bucketsCount;
    table->valSize = valueSize;

    _ERR_RET(allocateBuckets(table));

    table->size = 0;

    table->hash = hash;
    if (hash == NULL)
        table->hash = djb2;

    return HT_SUCCESS;
}

hashTableStatus_t hashTableDtor(hashTable_t *table)
{
    assert(table);

    for (size_t idx = 0; idx < table->bucketsCount; idx++) {
        hashTableNode_t *nextNode = (table->buckets + idx)->next;
        while (nextNode) {
            hashTableNode_t *current = nextNode;
            nextNode = current->next;

            _ERR_RET(deallocateNode(table, current));
        }
    }

    _ERR_RET(deallocateBuckets(table));

    return HT_SUCCESS;
}

/// @brief Insert element in hashTable
hashTableStatus_t hashTableInsert(hashTable_t *table, const char *key, const void *value)
{
    assert(table);
    assert(key);
    assert(table->valSize==0 || value);

    assert(table->buckets);
    assert(table->bucketsCount > 0);

    hash_t keyHash   = table->hash(key);
    size_t bucketIdx = keyHash % table->bucketsCount;

    hashTableNode_t *node = table->buckets + bucketIdx;

    while (node) {
        if (node->key && strcmp(node->key, key) == 0 )
            break;

        node = node->next;
    }


    if (!node) {
        table->size++;
        _ERR_RET(allocateNode(table, key, bucketIdx));
        node = (table->buckets + bucketIdx)->next;
    }

    memcpy(node->value, value, table->valSize);

    return HT_SUCCESS;
}

/// @brief Access element in hash table or insert it with default value
/// @return Ptr to element or NULL in case of error
void *hashTableAccess(hashTable_t *table, const char *key)
{
    return NULL;
}

void *hashTableFind(hashTable_t *table, const char *key)
{
    assert(table);
    assert(key);
    assert(table->buckets);
    assert(table->bucketsCount);

    hash_t keyHash   = table->hash(key);
    size_t bucketIdx = keyHash % table->bucketsCount;

    hashTableNode_t *node = table->buckets + bucketIdx;

    while (node) {
        if (node->key && strcmp(node->key, key) == 0 )
            return node->value;

        node = node->next;
    }

    return NULL;
}

hashTableStatus_t hashTableVerify(hashTable_t *table)
{
    return HT_SUCCESS;
}

hashTableStatus_t hashTableDump(hashTable_t *table)
{
    if (!table) {
        errprintf("Null pointer passed");
        return HT_ERROR;
    }

    errprintf("hashTable_t[%p] dump:\n"
              "\tbucketsCount %zu\n"
              "\tvalSize      %zu\n"
              "\tsize         %zu\n",
              table, table->bucketsCount, table->valSize, table->size);

    errprintf("Buckets[%p]:\n", table->buckets);
    for (size_t bucketIdx = 0; bucketIdx < table->bucketsCount; bucketIdx++) {
        hashTableNode_t *current = (table->buckets + bucketIdx)->next;
        if (current) errprintf("\t#%zu \n", bucketIdx);
        while (current) {
            errprintf("\t\t\"%s\" -> %p\n", current->key, current->value);
            current = current->next;
        }

    }

    return HT_SUCCESS;
}

hashTableStatus_t hashTableCalcDistibution(hashTable_t *table)
{
    return HT_SUCCESS;
}

