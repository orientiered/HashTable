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
void testPerformance(const char *stringsFile, const char *requestsFile) {
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

    fprintf(stderr, "Loaded %ji strings in %.2f ms\n",
                words.wordsCount, codeClockGetTimeMs(&clock) );
    fprintf(stderr, "Hash table size = %zu\n", ht.size);

    /* ======================= Statistics ========================================== */

    hashTableCalcDistribution(&ht);

    /* ======================= Main test  ========================================== */

    int64_t totalFound = testRequests(&ht, requests, &clock);

    fprintf(stderr, "Processed requests in %.2f ms\n", codeClockGetTimeMs(&clock));
    double avgTicksPerFind = (double)(clock.clocksEnd - clock.clocksStart) / (double) (requests.wordsCount * TEST_LOOPS);
    fprintf(stderr, "Avg ticks per search: %.2f\n", avgTicksPerFind); 

    fprintf(stderr, "Total requests: %ji, succesfull searches: %ji\n",
                    requests.wordsCount,                totalFound);

    // hashTableDump(&ht);

    textDtor(&words);
    textDtor(&requests);

    hashTableDtor(&ht);
}
