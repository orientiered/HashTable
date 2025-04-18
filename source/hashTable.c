#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hashTable.h"


#define FREE(ptr) free(ptr); ptr = NULL
#define CALLOC(type, nmemb) (type *) calloc(nmemb, sizeof(type))

hash_t checksum(const void *ptr)
{
    const char *cptr = (const char *) ptr;
    hash_t hash = 0;
    while (*cptr) {
        hash += (hash_t)*cptr;
        cptr++;
    }
    return hash;
}

hash_t djb2(const void *ptr)
{
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

    _VERIFY(table, HT_ERROR);

    return HT_SUCCESS;
}

hashTableStatus_t hashTableDtor(hashTable_t *table)
{
    assert(table);

    _VERIFY(table, HT_ERROR);

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

    _VERIFY(table, HT_ERROR);

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
    assert(table);
    assert(key);

    assert(table->buckets);
    assert(table->bucketsCount > 0);

    _VERIFY(table, NULL);

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
        _ERR_RET_PTR(allocateNode(table, key, bucketIdx));
        node = (table->buckets + bucketIdx)->next;
    }

    return node->value;
}

void *hashTableFind(hashTable_t *table, const char *key)
{
    assert(table);
    assert(key);
    assert(table->buckets);
    assert(table->bucketsCount);

    _VERIFY(table, NULL);

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
    if (!table)
        return HT_ERROR;

    if (table->bucketsCount == 0) {
        errprintf("Table is probably not initialized: bucketsCount = 0\n");
        return HT_NO_INIT;
    }

    if (!table->buckets) {
        errprintf("Buckets ptr is null");
        return HT_MEMORY_ERROR;
    }

    size_t size = 0;
    for (size_t bucketIdx = 0; bucketIdx < table->bucketsCount; bucketIdx++) {
        hashTableNode_t *node = table->buckets[bucketIdx].next;

        while (node) {
            size++;
            hash_t hash = table->hash(node->key);


            if (!node->key) {
                errprintf("Found node without key in bucket %zu\n", bucketIdx);
                return HT_NO_KEY;
            }

            if (table->valSize > 0 && !node->value) {
                errprintf("Found node without value in bucket %zu (valSize > 0)\n", bucketIdx);
                return HT_NO_VALUE;
            }

            if (hash % table->bucketsCount != bucketIdx) {
                errprintf("Key %s with hash %ju must be in bucket %ju, but lays in bucket %zu\n",
                             node->key,    hash,     hash % table->bucketsCount,     bucketIdx);
                return HT_WRONG_HASH;
            }


            node = node->next;
        }

    }

    if (size != table->size) {
        errprintf("Expected size to be %zu, but found only %zu elements\n", table->size, size);
        return HT_WRONG_SIZE;
    }

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
            errprintf("\t\t\"%s\" -> [%p]", current->key, current->value);
            HDBG(
                if (table->printElem ) {
                    table->printElem(current->value);
                }
            )
            errprintf("\n");
            current = current->next;
        }

    }

    return HT_SUCCESS;
}

hashTableStatus_t hashTableCalcDistribution(hashTable_t *table)
{
    assert(table);
    assert(table->bucketsCount > 0);
    assert(table->buckets);

    const int BARS_COUNT = 20;
    const int BAR_LENGTH = 20;
    int64_t bars[BARS_COUNT] = {0};

    int64_t sumOfSquares = 0, sum = 0;
    for (size_t idx = 0; idx < table->bucketsCount; idx++) {
        // calculating length of the list in current bucket
        int64_t bucketLen = 0;
        hashTableNode_t *node = table->buckets[idx].next;
        while(node) {
            bucketLen++;
            node = node->next;
        }

        sumOfSquares += bucketLen * bucketLen;
        sum          += bucketLen;

        // Adding length of the list to corresponding bar in the chart
        bars[idx * BARS_COUNT / table->bucketsCount] += bucketLen;
    }

    float meanOfSquares = (float) sumOfSquares / (float) table->bucketsCount;
    float mean = (float) sum / (float) table->bucketsCount;

    float disp = meanOfSquares - mean*mean;

    fprintf(stderr, "Average elements in bucket: %.2f\n"
                    "Dispersion: %.2f\n", mean, disp);

    fprintf(stderr, "=========== Distribution bar chart =========\n");
    for (int barIdx = 0; barIdx < BARS_COUNT; barIdx++) {
        int64_t filledChars = BARS_COUNT * BAR_LENGTH * bars[barIdx] / sum;
        fputc('|', stderr);
        while((filledChars--) > 0) fputc('#', stderr);
        fputc('\n', stderr);
    }
    fprintf(stderr, "============================================\n");

    return HT_SUCCESS;
}

