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

int main(int argc, const char *argv[]) {

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");

    if (!input || !output) {
        fprintf(stderr, "failed to open %s or %s\n", argv[1], argv[2]);
        return 1;
    }

    int totalRequests = 0; sscanf(argv[3], "%d", &totalRequests);
    float foundPercent = 0.; sscanf(argv[4], "%f", &foundPercent);

    setvbuf(input, NULL, _IOFBF, 65536);
    setvbuf(output, NULL, _IOFBF, 65536);

    int64_t textLen = getFileLen(input);

    fprintf(stderr, "Len = %ji bytes\n", textLen);

    char *text = (char*) calloc(textLen+1, 1);
    char **words = (char **) calloc(textLen, sizeof(char *));

    assert(fread(text, 1, textLen, input) == textLen);
    fclose(input);

    int64_t wordCount = 0;
    char *textPtr = text;
    int shift = 0;
    sscanf(textPtr, "%*[^a-zA-Z]%n", &shift);
    textPtr += shift;
    while (true) {
        int wordLen = 0;
        sscanf(textPtr, "%*s%n %n", &wordLen, &shift);
        if (wordLen == 0)
            break;

        words[wordCount++] = textPtr;
        *(textPtr + wordLen) = '\0';

        textPtr += shift;

    }

    fprintf(stderr, "Total words: %ji\n", wordCount);

    for (int cnt = 0; cnt < totalRequests; cnt++) {
        float normalRand = (float) rand() / RAND_MAX;
        bool wordFromText = normalRand <= foundPercent;

        if (wordFromText) {
            int idx = rand() % wordCount;
            fprintf(output, "%s\n", words[idx]);
        } else {
            char word[MAX_WORD_LEN+1];
            generateRandomWord(word, 3, 14);
            fprintf(output, "%s\n", word);
        }
    }

    fclose(output);

    return 0;
}
