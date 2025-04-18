#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
// #include <random.h>


int64_t getFileLen(FILE *file) {
    fseek(file, 0, SEEK_END);
    int64_t len = ftell(file);
    fseek(file, 0, SEEK_SET);
    return len;
}

const int MAX_WORD_LEN = 32;
const int MIN_RNDWORD_LEN = 3;
const int MAX_RNDWORD_LEN = 14;
const int BUFFERING_SIZE = 1 << 18;

int generateRandomWord(char *word, int minLen, int maxLen) {
    assert(minLen > 0);
    assert(maxLen > minLen);

    int len = minLen + rand() % (maxLen - minLen);
    for (int idx = 0; idx < len; idx++) {
        word[idx] = 'a' + rand() % ('z' - 'a');
    }
    word[len] = '\0';
    return len;
}

typedef struct {
    char *data;
    int64_t length;
    char **words;
    int64_t wordsCount;
} text_t;

text_t readFileSplit(const char *fileName);

void writeTestsToFile(const char *fileName, text_t text, int tests, float foundPercent);


int main(int argc, const char *argv[]) {

    text_t text = readFileSplit(argv[1]);
    if (!text.data) {
        fprintf(stderr, "Failed to read file\n");
        return 1;
    }

    int totalRequests = 0; sscanf(argv[3], "%d", &totalRequests);
    float foundPercent = 0.; sscanf(argv[4], "%f", &foundPercent);

    writeTestsToFile(argv[2], text, totalRequests, foundPercent);

    free(text.words);
    free(text.data);

    return 0;
}

text_t readFileSplit(const char *fileName) {
    text_t result = {0};

    FILE *file = fopen(fileName, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", fileName);
        return result;
    }

    result.length = getFileLen(file);

    fprintf(stderr, "Len = %ji bytes\n", result.length);

    char *text = (char*) calloc(result.length + 1, 1); // last byte serves as terminator
    char **words = (char **) calloc(result.length, sizeof(char *));

    assert(fread(text, 1, result.length, file) == result.length);
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

void writeTestsToFile(const char *fileName, text_t text, int tests, float foundPercent) {
    FILE *output = fopen(fileName, "w");
    if (!output) {
        fprintf(stderr, "Failed to open %s for writing\n", fileName);
        return;
    }

    setvbuf(output, NULL, _IOFBF, BUFFERING_SIZE);

    for (int cnt = 0; cnt < tests; cnt++) {
        float normalRand = (float) rand() / RAND_MAX;
        bool wordFromText = normalRand <= foundPercent;

        if (wordFromText) {
            int idx = rand() % text.wordsCount;
            fputs(text.words[idx], output);
        } else {
            char word[MAX_WORD_LEN+1];
            generateRandomWord(word, MIN_RNDWORD_LEN, MAX_RNDWORD_LEN);
            fputs(word, output);
        }

        fputc('\n', output);
    }

    fclose(output);
    return;
}
