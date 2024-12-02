#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hashTable.h>
#include <assert.h>

#include "hashTable.h"


//djb2 hash
static uint64_t strHash(const char *str) {
    if (!str) return 0x1DED0BEDBAD0C0DE;
    uint64_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + *str;
        str++;
        //hash = 33*hash + c
    }
    return hash;
}

hashTable_t hashTableCtor(size_t size) {
    hashTable_t table;
    table.size = size;
    table.elements = (hashTableNode_t **) calloc(size, sizeof(hashTableNode_t*));
    return table;
}

enum hashTableStatus hashTableDtor(hashTable_t *table) {
    assert(table);
    hashTableClear(table);

    free(table->elements);
    return HASH_TABLE_SUCCESS;
}

enum hashTableStatus hashTableInsert(hashTable_t *table, const char *name, const void *elem, size_t elemSize) {
    hashTablePrint("Inserting '%s', hash = %zu\n", name, strHash(name));

    uint64_t idx = strHash(name) % table->size;
    hashTablePrint("Idx = %zu\n", idx);

    hashTableNode_t **current = table->elements + idx;
    hashTableNode_t *nullNode = NULL;
    hashTableNode_t **prev = &nullNode;
    while (*current) {
        printf("current: %p, prev: %p\n", *current, *prev);
        if (strcmp(name, (*current)->name) == 0)
            return HASH_TABLE_SUCCESS;

        prev = current;
        current = &(*current)->next;
    }
    printf("current: %p, prev: %p\n", *current, *prev);

    *current = (hashTableNode_t *)calloc(1, sizeof(hashTableNode_t));

    (*current)->dataSize = elemSize;
    (*current)->data = calloc(1, elemSize);
    memcpy((*current)->data, elem, elemSize);

    (*current)->name = (char *) calloc(strlen(name) + 1, sizeof(char));
    strcpy((*current)->name, name);

    if (*prev) (*prev)->next = *current;
    printf("current: %p, prev: %p\n", *current, *prev);
    if (*prev)printf("prev->next = %p\n", (*prev)->next);

    return HASH_TABLE_SUCCESS;
}

void *hashTableFind(hashTable_t *table, const char *name) {
    assert(table);
    assert(name);

    uint64_t idx = strHash(name) % table->size;
    hashTableNode_t *current = table->elements[idx];
    while (current && strcmp(name, current->name) != 0)
        current = current->next;

    return (current) ? current->data : NULL;
}

enum hashTableStatus hashTableRemove(hashTable_t *table, const char *name) {
    assert(table);
    assert(name);

    uint64_t idx = strHash(name) % table->size;
    hashTableNode_t **current = table->elements + idx;
    hashTableNode_t *nullNode = NULL;
    hashTableNode_t **prev = &nullNode;
    while (*current) {
        if (strcmp(name, (*current)->name) == 0)
            break;

        *prev = *current;
        *current = (*current)->next;
    }

    if (!*current) return HASH_TABLE_SUCCESS;

    hashTableNode_t *next = (*current)->next;
    free((*current)->name);
    free((*current)->data);
    free(*current);
    *current = next;
    if (*prev) (*prev)->next = *current;

    return HASH_TABLE_SUCCESS;
}

enum hashTableStatus hashTableClear(hashTable_t *table) {
    assert(table);

    for (size_t idx = 0; idx < table->size; idx++) {
        hashTableNode_t *current = table->elements[idx];
        while (current != NULL) {
            hashTableNode_t *next = current->next;
            free(current->data);
            free(current->name);
            free(current);
            current = next;
        }
    }

    return HASH_TABLE_SUCCESS;
}

enum hashTableStatus hashTableDump(hashTable_t *table) {
    fprintf(stderr, "hashTable_t[%p] DUMP\n", table);
    fprintf(stderr, "size = %zu\n", table->size);

    if (!table) return HASH_TABLE_SUCCESS;
    for (size_t idx = 0; idx < table->size; idx++) {
        fprintf(stderr, "%3zu: \n", idx);
        hashTableNode_t *current = table->elements[idx];
        while (current) {
            fprintf(stderr, "\t[%p] \"%s\": data = [%p], dataSize = %zu, next = [%p]\n", current, current->name, current->data, current->dataSize, current->next);
            current = current->next;
        }

    }

    fprintf(stderr, "hashTable_t DUMP end\n");
    return HASH_TABLE_SUCCESS;
}

enum hashTableStatus hashTableVerify(hashTable_t *table) {
    return HASH_TABLE_SUCCESS;
}
