#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hashTable.h"

#include <immintrin.h>

#define FREE(ptr) free(ptr); ptr = NULL
#define CALLOC(type, nmemb) (type *) calloc(nmemb, sizeof(type))

/* ================================================= */
/* There are two versions of this file               */
/* They use different structure of hashTable         */
/* One of them is deactivated with define            */
/* ================================================= */

#if HASH_TABLE_ARCH == 1

/* ============ Hash functions ======================================= */
hash_t checksum(const void *ptr)
{
    const char *cptr = (const char *) ptr;
    hash_t hash = 0;
    while (*cptr) {
        hash += (hash_t)*cptr;
        cptr++;
    }
    return hash;
}

hash_t djb2(const void *ptr)
{
    const char *cptr = (const char *) ptr;

    hash_t hash = 5381;
    while (*cptr) {
        hash = ((hash << 5) + hash) + (hash_t)*cptr;
        cptr++;
        //hash = 33*hash + c
    }
    return hash;
}

/* Source: https://github.com/halloweeks/CRC-32/blob/main/crc32.h */
static const hash_t crc32_table[256] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D 
};

hash_t crc32(const void *data) 
{
	hash_t crc = 0xFFFFFFFF;
	
    const uint8_t *ptr = (const uint8_t *)data;

	for (; *ptr; ptr++) {
		crc = (crc >> 8) ^ crc32_table[(crc ^ *ptr) & 0xFF];
	}
	
	return crc ^ 0xFFFFFFFF;
}

// Calculate crc32 hash of 16 bytes of data
hash_t fastCrc32_16(const void *data) {
    hash_t crc = 0xFFFFFFFF;
    asm("crc32q  (%[ptr]), %[crc]\n\t"
        "crc32q 8(%[ptr]), %[crc]\n" 
      : [crc] "+r" (crc)
      : [ptr] "r" (data));
    return crc;
}

/* ================== Allocators ==================================================== */
// expects bucketsCount and valueSize to be set already
static hashTableStatus_t allocateBuckets(hashTable_t *table)
{
    assert(table);
    assert(table->bucketsCount > 0);

    hashTableNode_t *buckets = CALLOC(hashTableNode_t, table->bucketsCount);
    if (!buckets) {
        hprintf("Failed to allocate buckets array\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }

    table->buckets = buckets;

    return HT_SUCCESS;
}

static hashTableStatus_t deallocateBuckets(hashTable_t *table)
{
    assert(table);

    FREE(table->buckets);

    return HT_SUCCESS;
}

static hashTableStatus_t allocateNode(hashTable_t *table, const char *key, hashTableNode_t *bucketHead, hashTableNode_t **nodePtr)
{
    assert(table);
    assert(key);

    assert(bucketHead);
    assert(table->buckets);

    hashTableNode_t *newNode = CALLOC(hashTableNode, 1);

    if (!newNode) {
        hprintf("Failed to allocate node\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }

    hashTableNode_t *headNode = bucketHead;

    newNode->next  = headNode->next;
    headNode->next = newNode;

    size_t len   = strlen(key) + 1;
    size_t alloc_len = len;

    #if defined(CMP_LEN_FIRST)
        newNode->len = len-1; // without terminating byte
    #endif

    #if defined(FAST_STRCMP)
        // size in aligned_alloc must be multiple of alignment
        alloc_len += (KEY_ALIGNMENT - len % KEY_ALIGNMENT) % KEY_ALIGNMENT;
        // allocating aligned memory to later use with SIMD
        newNode->key = (char *) aligned_alloc(KEY_ALIGNMENT, alloc_len);
    #else
        newNode->key = (char *) calloc(alloc_len, 1);
    #endif
    

    if (!newNode->key) {
        hprintf("Failed to allocate memory for key\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }
    #if defined(FAST_STRCMP)
        // zeroing bytes in allocated memory
        memset(newNode->key, 0, alloc_len);
    #endif

    memcpy(newNode->key, key, len);

    newNode->value = calloc(table->valSize, 1);
    if (!newNode->value) {
        hprintf("Failed to allocate memory for value\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }

    if (nodePtr)
        *nodePtr = newNode;

    return HT_SUCCESS;
}

static hashTableStatus_t deallocateNode(hashTable_t *table, hashTableNode_t *node)
{
    assert(table);
    assert(node);

    free(node->value);
    free(node->key);
    free(node);

    return HT_SUCCESS;
}


/* ===================================== Constructor and destructor ========================================== */

hashTableStatus_t hashTableCtor(hashTable_t *table, size_t valueSize, size_t bucketsCount)
{
    assert(table);
    assert(bucketsCount > 0);

    table->bucketsCount = bucketsCount;
    table->valSize = valueSize;

    _ERR_RET(allocateBuckets(table));

    table->size = 0;

    _VERIFY(table, HT_ERROR);

    return HT_SUCCESS;
}

hashTableStatus_t hashTableDtor(hashTable_t *table)
{
    assert(table);

    _VERIFY(table, HT_ERROR);

    for (size_t idx = 0; idx < table->bucketsCount; idx++) {
        hashTableNode_t *nextNode = (table->buckets + idx)->next;
        while (nextNode) {
            hashTableNode_t *current = nextNode;
            nextNode = current->next;

            _ERR_RET(deallocateNode(table, current));
        }
    }

    _ERR_RET(deallocateBuckets(table));

    return HT_SUCCESS;
}


/* ===================================== Hash table functions ================================ */

#if defined(FAST_STRCMP)

#ifdef SSE
    typedef __m128i MMi_t;
    #define _MM_LOAD(ptr) _mm_load_si128(ptr)
    #define _MM_CMP_MOVEMASK(a, b) _mm_movemask_epi8(_mm_cmpeq_epi8(a,b))
    static const uint32_t _MM_MASK_CONSTANT = 0xFFFF;
#elif defined(AVX2)
    typedef __m256i MMi_t;
    #define _MM_LOAD(ptr) _mm256_load_si256(ptr)
    #define _MM_CMP_MOVEMASK(a, b) _mm256_movemask_epi8(_mm256_cmpeq_epi8(a,b))
    static const uint32_t _MM_MASK_CONSTANT = 0xFFFFFFFF;
#elif defined(AVX512)
    typedef __m512i MMi_t;
    #define _MM_LOAD(ptr) _mm512_load_si512(ptr)
    #define _MM_CMP_MOVEMASK(a, b) _mm512_cmpeq_epi16_mask(a,b)
    static const uint32_t _MM_MASK_CONSTANT = 0xFFFFFFFF;
#endif

static int fastStrcmp(MMi_t a, MMi_t *bptr) {
    MMi_t b = _MM_LOAD(bptr);
    uint32_t cmpMask = (uint32_t) _MM_CMP_MOVEMASK(a, b);
    return (int) (cmpMask ^ _MM_MASK_CONSTANT);
}

#endif

static hashTableNode_t *bucketSearch(hashTableNode_t *bucket, const char *key, const size_t keyLen) {
    // Getting first node with actual values
    hashTableNode_t *node = bucket->next;

    #ifndef FAST_STRCMP
        while (node) {
            if (CMP_LEN_OPT(node->len == keyLen &&) strcmp(node->key, key) == 0 )
                return node;

            node = node->next;
        }
    #else
        if (keyLen >= SMALL_STR_LEN) {
            // Key doesn't fit in SIMD register
            while (node) {
                if (CMP_LEN_OPT(node->len == keyLen &&) strcmp(node->key, key) == 0 )
                    return node;

                node = node->next;
            }

        } else {
            // Creating local aligned array of chars for key
            alignas(KEY_ALIGNMENT) char keyCopy[SMALL_STR_LEN] = "";
            // Copying key to it
            memcpy(keyCopy, key, keyLen+1);
            // Loading key to SIMD register
            MMi_t searchKey = _MM_LOAD((MMi_t *) keyCopy);

            while (node) {
                //! Alignment of key is guaranteed by aligned_calloc with KEY_ALIGNMENT
                if (CMP_LEN_OPT(node->len == keyLen &&) fastStrcmp(searchKey, (MMi_t *) node->key) == 0)
                    return node;

                node = node->next;

            }
        }

    #endif

    return NULL;
}

/// @brief Core function of hashTable
/// Search element in table, return pointer to it (or NULL) and write pointer of corresponding bucket   
static hashTableNode_t *hashTableGetBucketAndElement(hashTable_t *table, const char *key, hashTableNode_t **bucketPtr) {
    assert(table);
    assert(key);
    assert(table->buckets);
    assert(table->bucketsCount);

    _VERIFY(table, NULL);

    #if defined(FAST_STRCMP) || defined(CMP_LEN_FIRST)
        // Calculating length of the key string
        const size_t keyLen = strlen(key);
    #endif

    // Calculating hash of the string
    // hash_t keyHash   = _HASH_FUNC(key, keyLen);
    hash_t keyHash   = _HASH_FUNC(key);
    // Determining index of the corresponding bucket
    size_t bucketIdx = keyHash % table->bucketsCount;

    // Bucket - head node of the list
    hashTableNode_t *bucket = (table->buckets + bucketIdx);
    if (bucketPtr)
        *bucketPtr = bucket;


    return bucketSearch(bucket, key, keyLen);
}


/// @brief Insert element in hashTable
hashTableStatus_t hashTableInsert(hashTable_t *table, const char *key, const void *value)
{
    assert(table);
    assert(key);
    assert(table->valSize==0 || value);

    assert(table->buckets);
    assert(table->bucketsCount > 0);

    _VERIFY(table, HT_ERROR);

    hashTableNode_t *bucket = NULL;
    hashTableNode_t *node = hashTableGetBucketAndElement(table, key, &bucket);


    if (!node) {
        table->size++;
        _ERR_RET(allocateNode(table, key, bucket, &node) );
    }

    memcpy(node->value, value, table->valSize);

    return HT_SUCCESS;
}

/// @brief Access element in hash table or insert it with default value
/// @return Ptr to element or NULL in case of error
void *hashTableAccess(hashTable_t *table, const char *key)
{
    assert(table);
    assert(key);

    assert(table->buckets);
    assert(table->bucketsCount > 0);

    _VERIFY(table, NULL);

    hashTableNode_t *bucket = NULL;
    hashTableNode_t *node = hashTableGetBucketAndElement(table, key, &bucket);

    if (!node) {
        table->size++;
        _ERR_RET_PTR(allocateNode(table, key, bucket, &node));
    }

    return node->value;
}


void *hashTableFind(hashTable_t *table, const char *key)
{
    assert(table);
    assert(key);
    assert(table->buckets);
    assert(table->bucketsCount);

    _VERIFY(table, NULL);

    hashTableNode_t *node = hashTableGetBucketAndElement(table, key, NULL);

    return node ? node->value : node;
}

hashTableStatus_t hashTableVerify(hashTable_t *table)
{
    if (!table)
        return HT_ERROR;

    if (table->bucketsCount == 0) {
        errprintf("Table is probably not initialized: bucketsCount = 0\n");
        return HT_NO_INIT;
    }

    if (!table->buckets) {
        errprintf("Buckets ptr is null");
        return HT_MEMORY_ERROR;
    }

    size_t size = 0;
    for (size_t bucketIdx = 0; bucketIdx < table->bucketsCount; bucketIdx++) {
        hashTableNode_t *node = table->buckets[bucketIdx].next;

        while (node) {
            size++;
            const size_t keyLen = strlen(node->key);

            hash_t hash = _HASH_FUNC(node->key);


            if (!node->key) {
                errprintf("Found node without key in bucket %zu\n", bucketIdx);
                return HT_NO_KEY;
            }

            #if defined(CMP_LEN_FIRST)
                if (keyLen != node->len) {
                    errprintf("Wrong len of key %s\n", node->key);
                    return HT_NO_KEY;
                }
            #endif

            if (table->valSize > 0 && !node->value) {
                errprintf("Found node without value in bucket %zu (valSize > 0)\n", bucketIdx);
                return HT_NO_VALUE;
            }

            if (hash % table->bucketsCount != bucketIdx) {
                errprintf("Key %s with hash %ju must be in bucket %ju, but lays in bucket %zu\n",
                             node->key,    hash,     hash % table->bucketsCount,     bucketIdx);
                return HT_WRONG_HASH;
            }


            node = node->next;
        }

    }

    if (size != table->size) {
        errprintf("Expected size to be %zu, but found only %zu elements\n", table->size, size);
        return HT_WRONG_SIZE;
    }

    return HT_SUCCESS;
}

hashTableStatus_t hashTableDump(hashTable_t *table)
{
    if (!table) {
        errprintf("Null pointer passed");
        return HT_ERROR;
    }

    errprintf("hashTable_t[%p] dump:\n"
              "\tbucketsCount %zu\n"
              "\tvalSize      %zu\n"
              "\tsize         %zu\n",
              table, table->bucketsCount, table->valSize, table->size);

    errprintf("Buckets[%p]:\n", table->buckets);
    for (size_t bucketIdx = 0; bucketIdx < table->bucketsCount; bucketIdx++) {
        hashTableNode_t *current = (table->buckets + bucketIdx)->next;
        if (current) errprintf("\t#%zu \n", bucketIdx);
        while (current) {
            errprintf("\t\t\"%s\" -> [%p]", current->key, current->value);
            HDBG(
                if (table->printElem ) {
                    table->printElem(current->value);
                }
            )
            errprintf("\n");
            current = current->next;
        }

    }

    return HT_SUCCESS;
}

hashTableStatus_t hashTableCalcDistribution(hashTable_t *table)
{
    assert(table);
    assert(table->bucketsCount > 0);
    assert(table->buckets);

    const int BARS_COUNT = 20;
    const int BAR_LENGTH = 20;
    int64_t bars[BARS_COUNT] = {0};

    int64_t sumOfSquares = 0, sum = 0;
    for (size_t idx = 0; idx < table->bucketsCount; idx++) {
        // calculating length of the list in current bucket
        int64_t bucketLen = 0;
        hashTableNode_t *node = table->buckets[idx].next;
        while(node) {
            bucketLen++;
            node = node->next;
        }

        sumOfSquares += bucketLen * bucketLen;
        sum          += bucketLen;

        // Adding length of the list to corresponding bar in the chart
        bars[idx * BARS_COUNT / table->bucketsCount] += bucketLen;
    }

    float meanOfSquares = (float) sumOfSquares / (float) table->bucketsCount;
    float mean = (float) sum / (float) table->bucketsCount;

    float disp = sqrt(meanOfSquares - mean*mean);

    fprintf(stderr, "Average elements in bucket: %.2f\n"
                    "Dispersion: %.2f\n", mean, disp);

    fprintf(stderr, "=========== Distribution bar chart =========\n");
    for (int barIdx = 0; barIdx < BARS_COUNT; barIdx++) {
        int64_t filledChars = BARS_COUNT * BAR_LENGTH * bars[barIdx] / sum;
        fputc('|', stderr);
        while((filledChars--) > 0) fputc('#', stderr);
        fputc('\n', stderr);
    }
    fprintf(stderr, "============================================\n");

    return HT_SUCCESS;
}

#endif