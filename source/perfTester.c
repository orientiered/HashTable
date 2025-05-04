#include <stdlib.h>
#include <assert.h>
#include <x86intrin.h>

#include "perfTester.h"
#include "hashTable.h"

/* ========================== Clock functions ========================== */
void codeClockStart(codeClock_t *clk) {
    clock_gettime(CODE_CLOCK_MODE, &clk->start);
    clk->clocksStart = _rdtsc();
}

const int64_t MCS_PER_SEC  = 1000000;
const int64_t MCS_PER_MSC  = 1000;
const int64_t NSEC_PER_MCS = 1000;

int64_t codeClockStop(codeClock_t *clk) {
    clk->clocksEnd = _rdtsc();
    // fprintf(stderr, "Elapsed clocks: %.3f * 10^9 \n", (double) (clk->clocksEnd - clk->clocksStart) / 1e9);
    clock_gettime(CODE_CLOCK_MODE, &clk->end);
    clk->elapsed = (clk->end.tv_sec - clk->start.tv_sec) * MCS_PER_SEC +
                   (clk->end.tv_nsec - clk->start.tv_nsec) / NSEC_PER_MCS;

    return clk->elapsed;
}

double codeClockGetTimeSec(codeClock_t *clk) {return (double) clk->elapsed / MCS_PER_SEC;}
double codeClockGetTimeMs (codeClock_t *clk) {return (double) clk->elapsed / MCS_PER_MSC;}

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
void testPerformance(const char *stringsFile, const char *requestsFile, bool printLess) {
    text_t words    = readFileSplit(stringsFile);
    text_t requests = readFileSplit(requestsFile);

    hashTable ht = {};
    hashTableCtor(&ht, sizeof(int), 1500);
    HDBG(ht.printElem = printInt;)

    codeClock_t clock;

    MEASURE_TIME(clock,
        for (int idx = 0; idx < words.wordsCount; idx++) {
            hashTableAccess(&ht, words.words[idx]); // inserting default values: 0
        }
    )

    hashTableVerify(&ht);
    // hashTableDump(&ht);

    if (!printLess) {
        fprintf(stderr, "Loaded %ji strings in %.2f ms\n",
                    words.wordsCount, codeClockGetTimeMs(&clock) );
        fprintf(stderr, "Hash table size = %zu\n", ht.size);
    }

    /* ======================= Statistics ========================================== */

    if (!printLess)
        hashTableCalcDistribution(&ht);

    /* ======================= Main test  ========================================== */

    int64_t totalFound = testRequests(&ht, requests, &clock);

    double avgTicksPerFind = (double)(clock.clocksEnd - clock.clocksStart) / (double) (requests.wordsCount * TEST_LOOPS);
    double timeInMs = codeClockGetTimeMs(&clock);

    if (!printLess) {
        fprintf(stderr, "Processed requests in %.2f ms\n", timeInMs);
        fprintf(stderr, "Avg ticks per search: %.2f\n", avgTicksPerFind); 

        fprintf(stderr, "Total requests: %ji, succesfull searches: %ji\n",
                                            requests.wordsCount,                totalFound);
    } else {
        fprintf(stderr, "%.2f %.2f\n", timeInMs, avgTicksPerFind);
    }

    // hashTableDump(&ht);

    if (!printLess) {
        FILE *result = fopen("wordCounts.txt", "w");
        assert(result);

        #if HASH_TABLE_ARCH == 2
        for (int bidx = 0; bidx < ht.bucketsCount; bidx++) {
            hashTableBucket_t bucket = ht.buckets[bidx];
            for (int idx = 0; idx < bucket.size; idx++) {
                hashTableNode_t *node = bucket.elements+idx;
                fprintf(result, "%s %d\n", &node->keyMM, *(int *)node->value);
            }
        }

        hashTableBucket_t bucket = ht.longKeys;
        for (int idx = 0; idx < bucket.size; idx++) {
            hashTableNode_t *node = bucket.elements+idx;
            fprintf(result, "%s %d\n", node->keyPtr, *(int *)node->value);
        }
        #else
        for (int bidx = 0; bidx < ht.bucketsCount; bidx++) {
            hashTableNode_t *node = ht.buckets[bidx].next;
            while (node) {
                fprintf(result, "%s %d\n", node->key, *(int *)node->value);
                node = node->next;
            }

        }
        
        #endif

        fclose(result);
    }

    textDtor(&words);
    textDtor(&requests);

    hashTableDtor(&ht);
}
