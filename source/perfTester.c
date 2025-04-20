#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "perfTester.h"
#include "hashTable.h"

/* ========================== Clock functions ========================== */
void codeClockStart(codeClock_t *clk) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &clk->start);
}

const int64_t MCS_PER_SEC = 1000000;
const int64_t MCS_PER_MSC = 1000;
const int64_t NSEC_PER_MCS = 1000;

int64_t codeClockStop(codeClock_t *clk) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &clk->end);
    clk->elapsed = (clk->end.tv_sec - clk->start.tv_sec) * MCS_PER_SEC +
                   (clk->end.tv_nsec - clk->start.tv_nsec) / NSEC_PER_MCS;

    return clk->elapsed;
}

double codeClockGetTimeSec(codeClock_t *clk) {return (double) clk->elapsed / MCS_PER_SEC;}
double codeClockGetTimeMs (codeClock_t *clk) {return (double) clk->elapsed / MCS_PER_MSC;}

/* ========================== Functions to parse files ==================== */
int64_t getFileLen(FILE *file) {
    fseek(file, 0, SEEK_END);
    int64_t len = ftell(file);
    fseek(file, 0, SEEK_SET);
    return len;
}

text_t readFileSplit(const char *fileName) {
    text_t result = {0};

    FILE *file = fopen(fileName, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", fileName);
        return result;
    }

    result.length = getFileLen(file);
    if (result.length < 0) {
        fprintf(stderr, "File is broken: non-positive length\n");
        return result;
    }

    fprintf(stderr, "Len = %ji bytes\n", result.length);

    char *text = (char*) calloc( (size_t) result.length + 1, 1); // last byte serves as terminator
    char **words = (char **) calloc( (size_t) result.length, sizeof(char *));

    size_t bytesRead = fread(text, 1, (size_t) result.length, file);
    assert(bytesRead == (size_t) result.length);

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

/* ========================== Tester function ========================== */
void testPerformance(const char *stringsFile, const char *requestsFile) {
    text_t words = readFileSplit(stringsFile);
    text_t requests = readFileSplit(requestsFile);

    hashTable ht = {};
    hashTableCtor(&ht, sizeof(int), 1500, crc32);
    HDBG(ht.printElem = printInt;)

    codeClock_t clock;

    MEASURE_TIME(clock,
        for (int idx = 0; idx < words.wordsCount; idx++) {
            *(int *)hashTableAccess(&ht, words.words[idx]) += 1;
        }
    )

    fprintf(stderr, "Loaded %ji strings in %.2f ms\n",
                words.wordsCount, codeClockGetTimeMs(&clock) );
    fprintf(stderr, "Hash table size = %zu\n", ht.size);

    hashTableCalcDistribution(&ht);

    int64_t totalFound = 0;

    MEASURE_TIME(clock,
        for (int idx = 0; idx < requests.wordsCount; idx++) {
            void *value = hashTableFind(&ht, requests.words[idx]);
            totalFound += (value != NULL);
        }
    )

    fprintf(stderr, "Processed requests in %.2f ms\n", codeClockGetTimeMs(&clock));

    fprintf(stderr, "Total requests: %ji, succesfull searches: %ji\n",
                    requests.wordsCount,                totalFound);

    // hashTableDump(&ht);

    free(words.data);
    free(words.words);
    free(requests.data);
    free(requests.words);

    hashTableDtor(&ht);
}
