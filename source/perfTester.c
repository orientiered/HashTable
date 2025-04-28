#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <x86intrin.h>

#include "perfTester.h"
#include "hashTable.h"

/* ========================== Clock functions ========================== */
void codeClockStart(codeClock_t *clk) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &clk->start);
    clk->clocksStart = _rdtsc();
}

const int64_t MCS_PER_SEC  = 1000000;
const int64_t MCS_PER_MSC  = 1000;
const int64_t NSEC_PER_MCS = 1000;

int64_t codeClockStop(codeClock_t *clk) {
    clk->clocksEnd = _rdtsc();
    fprintf(stderr, "Elapsed clocks: %.3f * 10^9 \n", (double) (clk->clocksEnd - clk->clocksStart) / 1e9);
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

/* ========================== Tester functions ========================== */

/* Main stress test function */
int64_t __attribute__ ((noinline)) testRequests(hashTable_t *ht, text_t requests, codeClock_t *clock) {
    int64_t totalFound = 0;

    MEASURE_TIME(*clock,
        for (int loop = 0; loop < TEST_LOOPS; loop++) {
            for (int idx = 0; idx < requests.wordsCount; idx++) {
                void *value = hashTableFind(ht, requests.words[idx]);
                if (value) {
                    (*(int *)value)++; 
                    totalFound++;
                }
            }
        }
    )

    return totalFound;
}

/* Wrapper for testRequests that loads test data from given files */
void testPerformance(const char *stringsFile, const char *requestsFile) {
    text_t words    = readFileSplit(stringsFile);
    text_t requests = readFileSplit(requestsFile);

    hashTable ht = {};
    hashTableCtor(&ht, sizeof(int), 1500);
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

    /* ======================= Statistics ========================================== */
    hashTableCalcDistribution(&ht);

    size_t less16 = 0, less32 = 0;
    for (size_t bucketIdx = 0; bucketIdx < ht.bucketsCount; bucketIdx++) {
        hashTableNode_t *node = ht.buckets[bucketIdx].next;
        while(node) {
            if (node->len < 16) {
                less16++;
                less32++;
            } else if (node->len < 32) {
                less32++;
            }
            node = node->next;
        }
    }

    fprintf(stderr, "length < 16: %zu, length < 32: %zu\nTable size: %zu\n", 
                                    less16, less32, ht.size);


    /* ====================================== Main test ================================== */

    int64_t totalFound = testRequests(&ht, requests, &clock);

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
