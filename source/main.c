#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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


//TODO: delete code below
static int smallTest();

static int64_t getFileSize(const int fd);

int mapFile(const char *fileName, const char **text, int64_t *length);
int unmapFile(const char *text, int64_t length);

static int64_t getFileSize(const int fd) {
    struct stat st;
    int errCode = fstat(fd, &st);
    return st.st_size;
}

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

int mapFile(const char *fileName, const char **text, int64_t *length) {

    // opeping flie
    int fileDesc = open(fileName, O_RDWR);
    if (fileDesc == -1) return 1;

    // getting length
    int64_t fileLength = getFileSize(fileDesc);

    // mapping file to virtual memory
    *text = (const char *) mmap(NULL, fileLength, PROT_READ, MAP_SHARED,
                                      fileDesc, 0);
    *length = fileLength;

    // we don't need file anymore
    close(fileDesc);
    return 0;
}
int unmapFile(const char *text, int64_t length) {
    return munmap((void *) text, length);
}
