#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <perfTester.h>
#include <hashTable.h>

/* ========================== Functions to parse files ==================== */
int64_t getFileLen(FILE *file) {
    fseek(file, 0, SEEK_END);
    int64_t len = ftell(file);
    fseek(file, 0, SEEK_SET);
    return len;
}

void textDtor(text_t *text) {
    free(text->data);  text->data  = NULL;
    free(text->words); text->words = NULL;
    text->wordsCount = text->length = 0;
}


text_t readFileSplitAligned(const char *fileName) {
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

    // fprintf(stderr, "Len = %ji bytes\n", result.length);

    char  *text      = (char*)   calloc( (size_t) result.length + 1, 1); // last byte serves as terminator
    miniString_t *words     = (miniString_t *) calloc((size_t) result.length, sizeof(miniString_t));
    char  *wordsData = (char *)  aligned_alloc(KEY_ALIGNMENT, (size_t) result.length * SMALL_STR_LEN);

    assert(text);
    assert(words);
    assert(wordsData);

    size_t bytesRead = fread(text, 1, (size_t) result.length, file);
    assert(bytesRead == (size_t) result.length);

    fclose(file);

    int64_t wordCount = 0;
    char *textPtr = text, *wordsPtr = wordsData;
    int shift = 0;
    sscanf(textPtr, "%*[^a-zA-Z]%n", &shift);
    textPtr += shift;

    while (true) {
        size_t wordLen = 0;
        while (*(textPtr + wordLen) && !isspace(*(textPtr+wordLen)) ) wordLen++;
        shift = wordLen;
        while(*(textPtr + shift) && isspace(*(textPtr + shift)) ) shift++;

        if (wordLen == 0)
            break;


        words[wordCount].data = wordsPtr;
        words[wordCount++].len = wordLen;

        memcpy(wordsPtr, textPtr, wordLen);

        int wordsShift = KEY_ALIGNMENT * ((wordLen+1 + KEY_ALIGNMENT - 1) / KEY_ALIGNMENT);
        memset(wordsPtr + wordLen, 0, wordsShift-wordLen);
        wordsPtr += wordsShift;

        textPtr += shift;

    }


    // fprintf(stderr, "Total words: %ji\n", wordCount);
    free(text);

    // shrinking allocated arrays
    words     = (miniString_t *) realloc(words, wordCount * sizeof(miniString_t));
    assert(words);

    // wordsData = (char *)  realloc(wordsData, (size_t)(wordsPtr - wordsData+1) );
    result.wordsCount = wordCount;
    result.data = wordsData;
    result.words = words;

    return result;
}

text_t readFileSplitUnaligned(const char *fileName) {
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
    miniString_t *words = (miniString_t *) calloc( (size_t) result.length, sizeof(miniString_t));

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

        words[wordCount].data = textPtr;
        words[wordCount++].len = wordLen;

        *(textPtr + wordLen) = '\0';

        textPtr += shift;

    }

    // fprintf(stderr, "Total words: %ji\n", wordCount);
    result.wordsCount = wordCount;
    result.data = text;
    result.words = words;

    return result;
}
