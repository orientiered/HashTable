#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hashTable.h"

#include <immintrin.h>
#include <sys/cdefs.h>

#define FREE(ptr) do {free(ptr); ptr = NULL;} while(0)
#define CALLOC(type, nmemb) (type *) calloc(nmemb, sizeof(type))

/* ================================================= */
/* There are two versions of this file               */
/* They use different structure of hashTable         */
/* One of them is deactivated with define            */
/* ================================================= */

#if HASH_TABLE_ARCH == 2

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
hash_t fastCrc32_16(const void *data) 
{
    hash_t crc = 0xFFFFFFFF;
    asm("crc32q  (%[ptr]), %[crc]\n\t"
        "crc32q 8(%[ptr]), %[crc]\n" 
      : [crc] "+r" (crc)
      : [ptr] "r" (data));
    return crc;
}

hash_t fastCrc32_32(const void *data) 
{
    hash_t crc = 0xFFFFFFFF;
    asm("crc32q   (%[ptr]), %[crc]\n\t"
        "crc32q  8(%[ptr]), %[crc]\n\t" 
        "crc32q 16(%[ptr]), %[crc]\n\t" 
        "crc32q 24(%[ptr]), %[crc]\n" 
      : [crc] "+r" (crc)
      : [ptr] "r" (data));
    return crc;
}

hash_t fastCrc32_64(const void *data) 
{
    hash_t crc = 0xFFFFFFFF;
    asm("crc32q   (%[ptr]), %[crc]\n\t"
        "crc32q  8(%[ptr]), %[crc]\n\t" 
        "crc32q 16(%[ptr]), %[crc]\n\t" 
        "crc32q 24(%[ptr]), %[crc]\n\t" 
        "crc32q 32(%[ptr]), %[crc]\n\t" 
        "crc32q 40(%[ptr]), %[crc]\n\t" 
        "crc32q 48(%[ptr]), %[crc]\n\t" 
        "crc32q 56(%[ptr]), %[crc]\n" 
      : [crc] "+r" (crc)
      : [ptr] "r" (data));
    return crc;
}

/* ==================================================================================== */
void *getValueFromNode(const hashTable_t *table, hashTableNode_t *node) {
    assert(table);
    assert(node);

    #ifdef SHORT_VALUES_IN_NODE
    const bool longValue = table->valSize > SMALL_STR_LEN;
    void *value = (!longValue) ? &node->value.MM : node->value.Ptr;
    #else
    void *value = node->value;
    #endif

    return value;
}
/* ================== Allocators ==================================================== */
static hashTableStatus_t deallocateNode(hashTable_t *table, hashTableNode_t *node, bool longKey);


// expects bucketsCount and valueSize to be set already
static hashTableStatus_t allocateBuckets(hashTable_t *table)
{
    assert(table);
    assert(table->bucketsCount > 0);

    hashTableBucket_t *buckets = CALLOC(hashTableBucket_t, table->bucketsCount);
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

    for (size_t idx = 0; idx < table->bucketsCount; idx++) {
        hashTableBucket_t *bucket = table->buckets + idx;

        for (size_t elemIdx = 0; elemIdx <  bucket->size; elemIdx++) {
            hashTableNode_t *node = bucket->elements + elemIdx;
            deallocateNode(table, node, false);
        }
        FREE(bucket->elements);
    }

    FREE(table->buckets);

    return HT_SUCCESS;
}

static hashTableStatus_t deallocateLongKeys(hashTable_t *table) 
{
    hashTableBucket_t *bucket = &table->longKeys;

    for (size_t elemIdx = 0; elemIdx <  bucket->size; elemIdx++) {
        hashTableNode_t *node = bucket->elements + elemIdx;
        deallocateNode(table, node, true);
    }
    FREE(bucket->elements);

    return HT_SUCCESS;
}

static hashTableStatus_t allocateNode(hashTable_t *table, const char *key, hashTableBucket_t *bucket, hashTableNode_t **nodePtr)
{
    assert(table);
    assert(table->buckets);
    assert(key);
    assert(bucket);

    size_t keyLen = strlen(key);

    // Allocating new node in array
    const size_t newSize = ++bucket->size;
    bucket->elements = (hashTableNode_t *) realloc(bucket->elements, newSize * sizeof(hashTableNode_t) );
    if (!bucket->elements) {
        hprintf("Failed to reallocate bucket\n");
        _ERR_RET(HT_MEMORY_ERROR);
    }

    // Prepairing new node
    hashTableNode_t *newNode = bucket->elements + newSize - 1;
    memset(newNode, 0, sizeof(hashTableNode_t));

    // Allocating place for value
    // If element is smaller than 16 bytes, then were store it in the node
    bool needCalloc = true;

    #ifdef SHORT_VALUES_IN_NODE
        needCalloc = table->valSize > SMALL_STR_LEN;
    #endif

    if (needCalloc) {
        void *newValue = calloc(1, table->valSize);
        if (!newValue) {
            hprintf("Failed to allocate memory for value\n");
            _ERR_RET(HT_MEMORY_ERROR);
        }
        #ifdef SHORT_VALUES_IN_NODE
        newNode->value.Ptr = newValue;
        #else
        newNode->value     = newValue;
        #endif
    }

    // Copying key 
    if (keyLen < SMALL_STR_LEN) {
        memcpy(&newNode->key.MM, key, keyLen);
    } else {
        char *newKey = CALLOC(char, keyLen+1);
        memcpy(newKey, key, keyLen);
        if (!newKey) {
            hprintf("Failed to allocate memory for key\n");
            _ERR_RET(HT_MEMORY_ERROR);
        }
        newNode->key.Ptr = newKey;
    }

    CMP_LEN_OPT(newNode->len = keyLen);

    *nodePtr = newNode;

    return HT_SUCCESS;
}

static hashTableStatus_t deallocateNode(hashTable_t *table, hashTableNode_t *node, bool longKey) 
{
    assert(table);
    assert(node);

    if (longKey)
        FREE(node->key.Ptr);

    #ifdef SHORT_VALUES_IN_NODE
        if (table->valSize > SMALL_STR_LEN) {
            FREE(node->value.Ptr);
        }   
    #else
        FREE(node->value);
    #endif

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

    _ERR_RET(deallocateBuckets(table));
    _ERR_RET(deallocateLongKeys(table));

    return HT_SUCCESS;
}


/* ===================================== Hash table functions ================================ */

#ifdef SSE
    #define _MM_LOAD(ptr) _mm_load_si128(ptr)
    #define _MM_CMP_MOVEMASK(a, b) _mm_movemask_epi8(_mm_cmpeq_epi8(a,b))
    static const uint32_t _MM_MASK_CONSTANT = 0xFFFF;
#elif defined(AVX2)
    #define _MM_LOAD(ptr) _mm256_load_si256(ptr)
    #define _MM_CMP_MOVEMASK(a, b) _mm256_movemask_epi8(_mm256_cmpeq_epi8(a,b))
    static const uint32_t _MM_MASK_CONSTANT = 0xFFFFFFFF;
#elif defined(AVX512)
    #define _MM_LOAD(ptr) _mm512_load_si512(ptr)
    #define _MM_CMP_MOVEMASK(a, b) _mm512_cmpeq_epi16_mask(a,b)
    static const uint32_t _MM_MASK_CONSTANT = 0xFFFFFFFF;
#endif

static int fastStrcmp(MMi_t a, MMi_t b) {
    uint32_t cmpMask = (uint32_t) _MM_CMP_MOVEMASK(a, b);
    return (int) (cmpMask ^ _MM_MASK_CONSTANT);
}

static hashTableNode_t *hashTableLongKeySearch(hashTableBucket_t *longKeys, const char *key) {

    hashTableNode_t *node = longKeys->elements;
    size_t bucketSize = longKeys->size;

    const size_t keyLen = strlen(key);  // TODO: length of key is known before this function is called

    for (size_t idx = 0; idx < bucketSize; idx++) {
        if (CMP_LEN_OPT(node->len == keyLen &&) strncmp(key, node->key.Ptr, keyLen) == 0)
            return node;
        
        node++;
    }
    return NULL;
}

/// @brief Search element with short key in given bucket
static hashTableNode_t *bucketSearch(const hashTableBucket_t *bucket, const char *key, const size_t keyLen) {
    assert(keyLen < SMALL_STR_LEN);
    assert(bucket);
    assert(key);

    #ifndef ALIGNED_KEYS
        #if defined(ALTERNATIVE_KEY_LOAD) && defined(SSE)
        // 16 0xFF and 16 0x00
        const uint8_t mask[32] = 
            {255, 255, 255, 255, 255, 255, 255, 255, 
             255, 255, 255, 255, 255, 255, 255, 255};

        MMi_t searchKey = _mm_loadu_si128((const __m128i_u *) key);
        MMi_t maskReg   = _mm_loadu_si128((const __m128i_u *) (mask + (16 - keyLen)) );
        searchKey = _mm_and_si128(searchKey, maskReg);

        #else 
        // Creating local aligned array of chars for key
        alignas(KEY_ALIGNMENT) char keyCopy[SMALL_STR_LEN] = "";
        // Copying key to it
        memcpy(keyCopy, key, keyLen);
        // Loading key to SIMD register
        MMi_t searchKey = _MM_LOAD((MMi_t *) keyCopy);
        #endif
    #else
        assert( (size_t)key % KEY_ALIGNMENT == 0);
        MMi_t searchKey = _MM_LOAD((const MMi_t *) key);
    #endif

    hashTableNode_t *node = bucket->elements;
    size_t bucketSize = bucket->size;

    for (size_t idx = 0; idx < bucketSize; idx++) {
        if (CMP_LEN_OPT(node->len == keyLen &&) fastStrcmp(searchKey, node->key.MM) == 0)
            return node;
        
        node++;
    }

    return NULL;
}

static hashTableNode_t *bucketSearch_NOINTRIN(const hashTableBucket_t *bucket, const char *key, const size_t keyLen) {
    hashTableNode_t *node = bucket->elements;
    const size_t bucketSize = bucket->size;

    for (size_t idx = 0; idx < bucketSize; idx++) {
        if (CMP_LEN_OPT(node->len == keyLen &&) strncmp(key, (const char *) &node->key.MM, keyLen + 1) == 0)
            return node;
        
        node++;
    }
    return NULL;
}


/// @brief Core function of hashTable
/// Search element in table, return pointer to it (or NULL) and write pointer of corresponding bucket   
static hashTableNode_t *hashTableGetBucketAndElement(hashTable_t *table, const char *key, hashTableBucket_t **bucketPtr) {
    assert(table);
    assert(key);
    assert(table->buckets);
    assert(table->bucketsCount);

    _VERIFY(table, NULL);

    const size_t keyLen = strlen(key);

    // long key -> search in separate array
    if (keyLen >= SMALL_STR_LEN) {
        if (bucketPtr)
            *bucketPtr = &table->longKeys;
        return hashTableLongKeySearch(&table->longKeys, key);
    }

    hash_t keyHash = _HASH_FUNC(key); 
    // Determining index of the corresponding bucket
    size_t bucketIdx = keyHash % table->bucketsCount;

    // Bucket - head node of the list
    hashTableBucket_t *bucket = (table->buckets + bucketIdx);
    if (bucketPtr)
        *bucketPtr = bucket;

    #ifndef FAST_STRCMP
    return bucketSearch_NOINTRIN(bucket, key, keyLen);
    #endif
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

    hashTableBucket_t *bucket = NULL;
    hashTableNode_t *node = hashTableGetBucketAndElement(table, key, &bucket);

    if (!node) {
        table->size++;
        _ERR_RET(allocateNode(table, key, bucket, &node) );
    }

    void *dest = getValueFromNode(table, node);
    memcpy(dest, value, table->valSize);

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

    hashTableBucket_t *bucket = NULL;
    hashTableNode_t *node = hashTableGetBucketAndElement(table, key, &bucket);

    if (!node) {
        table->size++;
        _ERR_RET_PTR(allocateNode(table, key, bucket, &node));
    }

    return getValueFromNode(table, node);
}


void *hashTableFind(hashTable_t *table, const char *key)
{
    assert(table);
    assert(key);
    assert(table->buckets);
    assert(table->bucketsCount);

    _VERIFY(table, NULL);

    hashTableNode_t *node = hashTableGetBucketAndElement(table, key, NULL);
    
    return (node) ? getValueFromNode(table, node) : NULL;
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
        hashTableBucket_t *bucket = &table->buckets[bucketIdx];
        size += bucket->size;

        hashTableNode_t *node = bucket->elements;

        for (size_t idx = 0; idx < bucket->size; idx++) {
            const size_t keyLen = strlen((const char *)&node->key.MM);
            if (keyLen >= SMALL_STR_LEN) {
                errprintf("Long key found in buckets with short keys\n");
                return HT_NO_KEY;
            }

            hash_t hash = _HASH_FUNC(&node->key.MM);

            if (hash % table->bucketsCount != bucketIdx) {
                errprintf("Key %s with hash %ju must be in bucket %ju, but lays in bucket %zu\n",
                             (const char *)&node->key.MM,    hash,     hash % table->bucketsCount,     bucketIdx);
                return HT_WRONG_HASH;
            }

            #if defined(CMP_LEN_FIRST)
                if (keyLen != node->len) {
                    errprintf("Wrong len of key %s\n", (const char *)&node->key.MM);
                    return HT_NO_KEY;
                }
            #endif

            if (!getValueFromNode(table, node)) {
                errprintf("Found node without value in bucket %zu (valSize > 0)\n", bucketIdx);
                return HT_NO_VALUE;
            }

            node++;
        }

    }


    hashTableNode_t *node = table->longKeys.elements;
    for (size_t idx = 0; idx < table->longKeys.size; idx++) {
        const size_t keyLen = strlen(node->key.Ptr);
        if (keyLen < SMALL_STR_LEN) {
            errprintf("Short key found in buckets with long keys: %zu, %s\n", keyLen, node->key.Ptr);
            return HT_NO_KEY;
        }

        #if defined(CMP_LEN_FIRST)
            if (keyLen != node->len) {
                errprintf("Wrong len of key %s\n", (const char *)&node->key.MM);
                return HT_NO_KEY;
            }
        #endif

        if (!getValueFromNode(table, node)) {
            errprintf("Found node without value in bucket with long keys\n");
            return HT_NO_VALUE;
        }

        if (!node->key.Ptr) {
            errprintf("Node without key in longKeys array\n");
            return HT_NO_KEY;
        }

        node++;
    }

    size += table->longKeys.size;

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
              "\tbucketsCount  %zu\n"
              "\tlongKeysCount %zu\n"
              "\tvalSize       %zu\n"
              "\tsize          %zu\n",
              table, table->bucketsCount, table->longKeys.size, table->valSize, table->size);

    errprintf("Buckets[%p]:\n", table->buckets);
    for (size_t bucketIdx = 0; bucketIdx < table->bucketsCount; bucketIdx++) {

        hashTableNode_t *node = table->buckets[bucketIdx].elements;
        if (node) errprintf("\t#%zu \n", bucketIdx);

        for (size_t elemIdx = 0; elemIdx < table->buckets[bucketIdx].size; elemIdx++) {
            void *value = getValueFromNode(table, node);
            errprintf("\t\t\"%s\" -> [%p]", (const char *) &node->key.MM, value);
            HDBG(
                if (table->printElem ) {
                    table->printElem(value);
                }
            )
            errprintf("\n");
            node++;
        }

    }

    errprintf("LongKeys array[%p]: \n", &table->longKeys);
    for (size_t idx = 0; idx < table->longKeys.size; idx++) {
        hashTableNode_t *elem = table->longKeys.elements + idx;
        void *value = getValueFromNode(table, elem);

        errprintf("\t\t\"%s\" -> [%p]", elem->key.Ptr, value);
        HDBG(
            if (table->printElem ) {
                table->printElem(value);
            }
        )
        errprintf("\n");
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
        size_t bucketLen = table->buckets[idx].size;

        sumOfSquares += bucketLen * bucketLen;
        sum          += bucketLen;

        // Adding length of the list to corresponding bar in the chart
        bars[idx * BARS_COUNT / table->bucketsCount] += bucketLen;
    }

    float meanOfSquares = (float) sumOfSquares / (float) table->bucketsCount;
    float mean = (float) sum / (float) table->bucketsCount;

    float disp = sqrt(meanOfSquares - mean*mean);

    errprintf("Average elements in bucket: %.2f\n"
                    "Dispersion: %.2f\n", mean, disp);

    errprintf("=========== Distribution bar chart =========\n");
    for (int barIdx = 0; barIdx < BARS_COUNT; barIdx++) {
        int64_t filledChars = BARS_COUNT * BAR_LENGTH * bars[barIdx] / sum;
        fputc('|', stderr);
        while((filledChars--) > 0) fputc('#', stderr);
        fputc('\n', stderr);
    }
    errprintf("============================================\n");

    return HT_SUCCESS;
}

hashTableStatus_t hashTableDumpDistribution(hashTable_t *table, const char *fileName) {
    assert(table);
    assert(fileName);

    FILE *out = fopen(fileName, "w");
    if (!out) {
        errprintf("Failed to open file %s\n", fileName);
        _ERR_RET(HT_ERROR);
    }

    for (size_t idx = 0; idx < table->bucketsCount; idx++) {
        fprintf(out, "%zu\n", table->buckets[idx].size);
    }

    fprintf(out, "%zu\n", table->longKeys.size);

    fclose(out);

    return HT_SUCCESS;
}


#endif