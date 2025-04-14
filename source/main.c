#include <stdio.h>

#include "hashTable.h"

int main() {

    hashTable_t ht = {};
    _ERR_RET(hashTableCtor(&ht, sizeof(int), 3, NULL));

    char str[64];
    for (int i = 0; i < 5; i++) {
        int val = i;
        scanf(" %s", str);
        _ERR_RET(hashTableInsert(&ht, str, &val));
    }

    hashTableDump(&ht);
    printf("Search\n");

    for (int i = 0; i < 10; i++) {
        scanf(" %s", str);
        void *val = hashTableFind(&ht, str);
        if (val) printf("%d\n", *(int *)val);
        else printf("Not found\n");
    }


    hashTableDtor(&ht);
    return 0;
}
