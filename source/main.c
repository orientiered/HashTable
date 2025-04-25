#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "hashTable.h"
#include "perfTester.h"




int main() {

    testPerformance("testStrings.txt", "testRequests.txt");

    return 0;
}


static int smallTest();

static int smallTest() {
    hashTable_t ht = {};
    _ERR_RET(hashTableCtor(&ht, sizeof(int), 3));

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
