#include <stdio.h>
#include <ctype.h>

int main(int argc, const char *argv[]) {

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");

    setvbuf(input, NULL, _IOFBF, 8192);
    setvbuf(input, NULL, _IOFBF, 8192);

    int curChar = 0;
    bool spaces = true;
    while ((curChar = fgetc(input)) != EOF) {
        bool alpha = isalpha(curChar);

        if (!alpha && !spaces)
            fputc('\n', output);

        if (alpha)
            fputc(tolower(curChar), output);

        spaces = !alpha;
    }

    fclose(input);
    fclose(output);

    return 0;
}
