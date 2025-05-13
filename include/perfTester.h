#ifndef PERF_TESTER_H
#define PERF_TESTER_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>

static const int TEST_LOOPS = 10;
const int HASH_TABLE_SIZE = 1500;

#define ALIGN_USER_KEYS

typedef struct {
    char *data;
    size_t len;
} miniString_t;

typedef struct {
    char *data;
    int64_t length;
    miniString_t *words;

    int64_t wordsCount;
} text_t;

typedef struct {
    struct timespec start;
    struct timespec end;
    int64_t clocksStart;
    int64_t clocksEnd;
    int64_t elapsed; // in microseconds
} codeClock_t;

void codeClockStart(codeClock_t *clk);
int64_t codeClockStop(codeClock_t *clk);

double codeClockGetTimeSec(codeClock_t *clk);
double codeClockGetTimeMs (codeClock_t *clk);

int64_t getFileLen(FILE *file);

text_t readFileSplitAligned(const char *fileName);
text_t readFileSplitUnaligned(const char *fileName);

void textDtor(text_t *text);

void testPerformance(const char *stringsFile, const char *requestsFile, bool printLess);

#define CODE_CLOCK_MODE CLOCK_THREAD_CPUTIME_ID

#define MEASURE_TIME(clock, ...) \
    codeClockStart(&clock);      \
    __VA_ARGS__                  \
    codeClockStop(&clock);

#endif
