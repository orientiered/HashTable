#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "hashTable.h"

static int smallTest();

static int64_t getFileSize(const int fd) {
    struct stat st;
    int errCode = fstat(fd, &st);
    return st.st_size;
}

int loadFile(const char *fileName, const char **text, int *length) {

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

int unloadFile(const char *text, int length) {
    return munmap((void *) text, length);
}

int main() {
    const char *text = NULL;
    int  textLen = 0;
    loadFile("test.txt", &text, &textLen);


    hashTable ht = {};
    _ERR_RET(hashTableCtor(&ht, sizeof(int), 150, NULL));
    HDBG(ht.printElem = printInt;)

    const char *textPtr = text;
    char strBuffer[256];
    int shift = 0;
    while (sscanf(textPtr, "%s%n", strBuffer, &shift) == 1) {
        textPtr += shift;
        *(int *)hashTableAccess(&ht, strBuffer) += 1;
    }

    hashTableDump(&ht);

    unloadFile(text, textLen);
    hashTableDtor(&ht);

    return 0;
}

static int smallTest() {
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
