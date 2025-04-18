#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "hashTable.h"

typedef struct {
    char *data;
    int64_t length;
    char **words;
    int64_t wordsCount;
} text_t;

static int smallTest();

static int64_t getFileSize(const int fd);
int64_t getFileLen(FILE *file);

text_t readFileSplit(const char *fileName);

int mapFile(const char *fileName, const char **text, int64_t *length);
int unmapFile(const char *text, int64_t length);

#define MEASURE_TIME(start, stop, ...) \
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start); \
    __VA_ARGS__                                     \
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);

int main() {
    text_t words = readFileSplit("testStrings.txt");
    text_t requests = readFileSplit("testRequests.txt");

    hashTable ht = {};
    _ERR_RET(hashTableCtor(&ht, sizeof(int), 1500, NULL));
    HDBG(ht.printElem = printInt;)

    struct timespec start, stop;

    MEASURE_TIME(start, stop,
        for (int idx = 0; idx < words.wordsCount; idx++) {
            *(int *)hashTableAccess(&ht, words.words[idx]) += 1;
        }
    )

    double elapsedMs = (double)(stop.tv_sec - start.tv_sec) * 1e3 + (double)(stop.tv_nsec - start.tv_nsec) * 1e-6;
    fprintf(stderr, "Loaded %ji strings in %.2f ms\n", words.wordsCount, elapsedMs);
    fprintf(stderr, "Hash table size = %zu\n", ht.size);

    int64_t totalFound = 0;

    MEASURE_TIME(start, stop,
        for (int idx = 0; idx < requests.wordsCount; idx++) {
            void *value = hashTableFind(&ht, requests.words[idx]);
            totalFound += (value != NULL);
        }
    )

    elapsedMs = (double)(stop.tv_sec - start.tv_sec) * 1e3 + (double)(stop.tv_nsec - start.tv_nsec) * 1e-6;
    fprintf(stderr, "Processed requests in %.2f ms\n", elapsedMs);

    fprintf(stderr, "Total requests: %ji, succesfull searches: %ji\n",
                    requests.wordsCount,                totalFound);

    // hashTableDump(&ht);

    free(words.data);
    free(words.words);
    free(requests.data);
    free(requests.words);

    hashTableDtor(&ht);

    return 0;
}

int64_t getFileLen(FILE *file) {
    fseek(file, 0, SEEK_END);
    int64_t len = ftell(file);
    fseek(file, 0, SEEK_SET);
    return len;
}

static int64_t getFileSize(const int fd) {
    struct stat st;
    int errCode = fstat(fd, &st);
    return st.st_size;
}

text_t readFileSplit(const char *fileName) {
    text_t result = {0};

    FILE *file = fopen(fileName, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", fileName);
        return result;
    }

    result.length = getFileLen(file);

    fprintf(stderr, "Len = %ji bytes\n", result.length);

    char *text = (char*) calloc(result.length + 1, 1); // last byte serves as terminator
    char **words = (char **) calloc(result.length, sizeof(char *));

    size_t bytesRead = fread(text, 1, result.length, file);
    assert(bytesRead == result.length);

    fclose(file);

    int64_t wordCount = 0;
    char *textPtr = text;
    int shift = 0;
    sscanf(textPtr, "%*[^a-zA-Z]%n", &shift);
    textPtr += shift;
    while (true) {
        int wordLen = 0;
        while (*(textPtr + wordLen) && !isspace(*(textPtr+wordLen)) ) wordLen++;
        shift = wordLen;
        while(*(textPtr + shift) && isspace(*(textPtr + shift)) ) shift++;

        if (wordLen == 0)
            break;

        words[wordCount++] = textPtr;
        *(textPtr + wordLen) = '\0';

        textPtr += shift;

    }

    fprintf(stderr, "Total words: %ji\n", wordCount);
    result.wordsCount = wordCount;
    result.data = text;
    result.words = words;

    return result;
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
