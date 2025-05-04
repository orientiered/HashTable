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

    // fprintf(stderr, "Len = %ji bytes\n", result.length);

    char  *text      = (char*)   calloc( (size_t) result.length + 1, 1); // last byte serves as terminator
    char **words     = (char **) calloc((size_t) result.length, sizeof(char*));
    char  *wordsData = (char *)  aligned_alloc(KEY_ALIGNMENT, (size_t) result.length * SMALL_STR_LEN); 

    size_t bytesRead = fread(text, 1, (size_t) result.length, file);
    assert(bytesRead == (size_t) result.length);

    fclose(file);

    int64_t wordCount = 0;
    char *textPtr = text, *wordsPtr = wordsData;
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


        words[wordCount++] = wordsPtr;
        memcpy(wordsPtr, textPtr, wordLen);

        int wordsShift = KEY_ALIGNMENT * ((wordLen+1 + KEY_ALIGNMENT - 1) / KEY_ALIGNMENT);
        memset(wordsPtr + wordLen, 0, wordsShift-wordLen); 
        wordsPtr += wordsShift;

        textPtr += shift;

    }


    // fprintf(stderr, "Total words: %ji\n", wordCount);
    free(text);

    // shrinking allocated arrays
    words     = (char **) realloc(words, wordCount * sizeof(char*));
    // wordsData = (char *)  realloc(wordsData, (size_t)(wordsPtr - wordsData+1) );

    result.wordsCount = wordCount;
    result.data = wordsData;
    result.words = words;

    return result;
}