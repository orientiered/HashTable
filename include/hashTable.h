#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <stdio.h>
#include <immintrin.h>

#define errprintf(...) fprintf(stderr, __VA_ARGS__)
#define TODO(msg) do {errprintf("TODO at %s:%d : %s", __FILE__, __LINE__, msg); abort();} while(0)
#ifndef NDEBUG
    #define HDBG(...) __VA_ARGS__
    #define hprintf(...) fprintf(stderr, __VA_ARGS__)
#else
    #define HDBG(...)
    #define hprintf(...)
#endif


/* ============================ Debug defines        ================================ */

// #define HASH_TABLE_VERIFY

/* ============================ Optimization defines ================================ */

/*! Hash table architecture version. Read more in README.md */
#define HASH_TABLE_ARCH 1

/*! Uses SIMD optimized strcmp that compares strings up to SMALL_STR_LEN              */
#define FAST_STRCMP

/*! Adds field len in hashTableNode and improves strcmp by comparing length first     */
// #define CMP_LEN_FIRST

/*! Assume that passed keys are aligned and have trailing zeros until the end of aligned block */
// #define ALIGNED_KEYS
#define ALTERNATIVE_KEY_LOAD

/*! Store short values (up to SMALL_STR_LEN bytes) in the node*/
// #define SHORT_VALUES_IN_NODE

/*! Which SIMD instruction set is used for fastStrcmp                                 */
//! Note: SSE is fastest 
#define SSE

/*! Use hardware-optimized hash function                                              */
// #define FAST_CRC32


#ifndef CMP_LEN_FIRST
    #define CMP_LEN_OPT(...)  
#else
    #define CMP_LEN_OPT(...) __VA_ARGS__
#endif

/* ====================== Hash functions =================================== */

typedef uint64_t hash_t;
typedef hash_t (*hashFunc_t)(const void *ptr);

hash_t checksum(const void *ptr);   ///< sum of ascii characters of the string
hash_t djb2(const void *ptr);
hash_t crc32(const void *data);

#ifdef FAST_CRC32
extern "C" { 
    hash_t fastCrc32u(const void *data);
    hash_t fastCrc32(const void *data, const size_t len);

    hash_t fastCrc32_16(const void *data);
    hash_t fastCrc32_32(const void *data);
    hash_t fastCrc32_64(const void *data);

    #ifdef SSE
        #define FAST_CRC32_2k fastCrc32_16
    #elif defined(AVX2)
        #define FAST_CRC32_2k fastCrc32_32
    #elif defined(AVX512) 
        #define FAST_CRC32_2k fastCrc32_64
    #endif

}
    #ifdef ALIGNED_KEYS
        #define _HASH_FUNC FAST_CRC32_2k
    #else
        #define _HASH_FUNC fastCrc32u
    #endif
#else
    #define _HASH_FUNC crc32
#endif

/* ====================== Alignment for strings (only with FAST_STRCMP) =============== */

#if defined(SSE)
typedef __m128i MMi_t;
static const size_t KEY_ALIGNMENT = 16; // alignment of keys in bytes
static const size_t SMALL_STR_LEN = 16; // lenght of string with terminating byte that fits into simd register
#elif defined(AVX2)
typedef __m256i MMi_t;
static const size_t KEY_ALIGNMENT = 32; // alignment of keys in bytes
static const size_t SMALL_STR_LEN = 32; // lenght of string with terminating byte that fits into simd register
#elif defined(AVX512)
typedef __m512i MMi_t;
static const size_t KEY_ALIGNMENT = 64; // alignment of keys in bytes
static const size_t SMALL_STR_LEN = 64;
#else
#error Define either SSE, AVX2 or AVX512
#endif


/* ========================= Struct definitions ============================= */

#if HASH_TABLE_ARCH == 2

union StrOrPtr {
    MMi_t MM;
    char *Ptr;
};

union ImmOrPtr {
    MMi_t MM;
    void *Ptr;
};

typedef struct hashTableNode {
    union StrOrPtr key; ///< Key is stored in node when it doesn't exceed SMALL_STR_LEN
                        ///< Otherwise we use pointer to string stored somewhere else

    #ifdef SHORT_VALUES_IN_NODE
    union ImmOrPtr value;   ///< Data stored in element (or ptr to it)
    #else
    void  *value;
    #endif
    CMP_LEN_OPT(uint32_t len;)
} hashTableNode_t;

typedef struct hashTableBucket {
    hashTableNode_t *elements;  ///< Array of nodes with key and value
    size_t size;                ///< Number of nodes in bucket
} hashTableBucket_t;

typedef struct hashTable {
    hashTableBucket_t *buckets; ///< Array of buckets 
    size_t bucketsCount;        ///< Number of buckets

    hashTableBucket_t longKeys; ///< Separate array for elements with long keys 

    size_t valSize;             ///< Size of data stored in element
    size_t size;                ///< Number of elements

    HDBG(int (*printElem)(const void *ptr);)
} hashTable_t;

#elif HASH_TABLE_ARCH == 1

typedef struct hashTableNode {
    struct hashTableNode *next;
    void  *value;
    char  *key;
#if defined(CMP_LEN_FIRST)
    uint32_t len;
#endif
} hashTableNode_t;


typedef struct hashTable {
    hashTableNode_t *buckets;
    size_t bucketsCount;

    size_t valSize;

    size_t size;

    HDBG(int (*printElem)(const void *ptr);)
} hashTable_t;

#endif

HDBG(
static int printInt(const void *ptr) {return errprintf("%d", *(const int *)ptr);}
static int printStr(const void *ptr) {return errprintf("%s", (const char*)ptr);}
static int printFloat(const void *ptr) {return errprintf("%f", *(const float*)ptr);}
)

typedef enum hashTableStatus {
    HT_SUCCESS      = 0,
    HT_MEMORY_ERROR = 1,
    HT_WRONG_HASH   = 2,
    HT_WRONG_SIZE   = 3,
    HT_NO_INIT      = 4,
    HT_NO_KEY       = 5,
    HT_NO_VALUE     = 6,
    HT_ERROR
} hashTableStatus_t;


/* ====================== Hash table functions ================================================ */
/*!
    @brief Construct new hashTable
    @param valueSize Size of the data in bytes that correspond to key
    @param bucketsCount Number of buckets to create in hashTable. In future may become starting number of buckets. 
*/
hashTableStatus_t hashTableCtor(hashTable_t *table, size_t valueSize, size_t bucketsCount);
/// @brief Destruct hashTable and free it's memory
hashTableStatus_t hashTableDtor(hashTable_t *table);

//! If ALIGNED_KEYS is defined, following functions expect key to be aligned on KEY_ALIGNMENT boundary 
//! and have trailing zeros up to the end of the aligned block

/// @brief Insert element in hashTable or rewrite it's value if already inserted
hashTableStatus_t hashTableInsert(hashTable_t *table, const char *key, const void *value);

/// @brief Access element in hash table or insert it with default value
/// @return Ptr to element or NULL in case of error
void *hashTableAccess(hashTable_t *table, const char *key);

/// @brief Find value by key in hash table
/// @return Ptr to value of NULL if there's no element with given key
void *hashTableFind(hashTable_t *table, const char *key);

/// @brief Extract ptr to value from given node of hashTable
void *getValueFromNode(const hashTable_t *table, hashTableNode_t *node);

/// @brief Check whether table is built correctly
hashTableStatus_t hashTableVerify(hashTable_t *table);

/// @brief Print dump of given hash table to stderr 
hashTableStatus_t hashTableDump(hashTable_t *table);

/// @brief Calculate disperion of size of buckets and plot graph of it's distribution
hashTableStatus_t hashTableCalcDistribution(hashTable_t *table);

/// @brief Dump number of elements in each bucket to the file
hashTableStatus_t hashTableDumpDistribution(hashTable_t *table, const char *fileName);

/*========================== Debugging tools =================================*/
static void htStackTrace(const char *file, int line, const char *function);

static void htStackTrace(const char *file, int line, const char *function) {
    static int callCounter = 0;

    callCounter++;

    if (callCounter == 1) {
        hprintf("Error occured in hashTable\n");
        hprintf("Stack trace:\n");
    }

    hprintf("#%2d at %s:%d in %s \n", callCounter, file, line, function);

    return;
}

#define _ERR_RET(expr) \
    {                                           \
        hashTableStatus_t status = (expr);      \
        if (status != HT_SUCCESS) {             \
            htStackTrace(__FILE__, __LINE__, __PRETTY_FUNCTION__); \
            return status;                      \
        }                                       \
    }


#define _ERR_RET_PTR(expr) \
    {                                           \
        hashTableStatus_t status = (expr);      \
        if (status != HT_SUCCESS) {             \
            htStackTrace(__FILE__, __LINE__, __PRETTY_FUNCTION__); \
            return NULL;                        \
        }                                       \
    }



#ifdef HASH_TABLE_VERIFY
    #define _VERIFY(table, ret) do {                             \
        hashTableStatus_t status = hashTableVerify(table);  \
        if (status != HT_SUCCESS) {                         \
            hashTableDump(table);                           \
            htStackTrace(__FILE__, __LINE__, __PRETTY_FUNCTION__); \
            return ret;                                     \
        }                                                   \
    } while(0)

#else
    #define _VERIFY(table, ret)
#endif

#endif
