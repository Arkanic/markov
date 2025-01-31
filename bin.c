// cool cli interface for markov
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "markov.h"

void print_help(void) {
    printf("markov new <name> - create new\nmarkov train <name> <file> - train name with file content\nmarkov run <name> <starting word> - run model name with starting word\nnote: no args error handling so don't mess up\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    if(argc < 2) print_help();

    char *aarg = argv[1];
    if(!strcmp(aarg, "new")) {
        struct markov_chain *chain = markov_new();
        markov_writefile(chain, argv[2]);
        markov_free(chain);
    } else if(!strcmp(aarg, "train")) {
        struct markov_chain *chain = markov_fromfile(argv[2]);

        FILE *fp = fopen(argv[3], "r");
        fseek(fp, 0L, SEEK_END);
        long end = ftell(fp);
        rewind(fp);
        char *text = (char *)malloc(sizeof(char) * (end + 1));
        fread(text, sizeof(char), end, fp);
        text[end] = '\0';
        fclose(fp);

        markov_train(chain, text);

        free(text);
        markov_writefile(chain, argv[2]);
        markov_free(chain);
    } else if(!strcmp(aarg, "run")) {
        struct markov_chain *chain = markov_fromfile(argv[2]);
        char *result = markov_generate(chain, argv[3], 1000);
        if(result == NULL) {
            printf("failed to generate\n");
            exit(2);
        }
        printf("%s\n", result);
        free(result);
        markov_free(chain);
    } else {
        print_help();
    }

    return 0;
}