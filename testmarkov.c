#include <stdio.h>
#include <stdlib.h>

#include "markov.h"
#include "hashmap.h"

int main(void) {
    struct markov_chain *markov = markov_new();

    struct markov_word *word1 = _markov_m_word_create("hello");
    struct markov_word *word2 = _markov_m_word_create("world");
    _markov_m_word_occurrence(word1, word2, 1);

    struct markov_wordref *ref = (struct markov_wordref *)hm_get(word1->futures, "world");
    printf("%s, %d\n", ref->word->word, ref->occurrences);

    char *txt = "hello there 1234";
    char *text = malloc(sizeof(char) * (strlen(txt) + 1));
    strcpy(text, txt);
    markov_train(markov, text);
    free(text);

    _markov_m_word_free(word1);
    _markov_m_word_free(word2);

    markov_free(markov);

    struct markov_word *words[] = {
        _markov_m_word_create("hello"),
        _markov_m_word_create("there"),
        _markov_m_word_create("friend"),
        _markov_m_word_create("enemy")
    };

    _markov_m_word_occurrence(words[0], words[1], 1);
    _markov_m_word_occurrence(words[1], words[2], 1);
    _markov_m_word_occurrence(words[1], words[3], 1);

    printf("hello: %s\n", words[0]->word);

    printf("hello next: %s\n", _markov_generate_getnext(words[0])->word->word);
    for(int i = 0; i < 10; i++) {
        printf("there next #%d: %s\n", i, _markov_generate_getnext(words[1])->word->word);
    }

    _markov_m_word_free(words[0]);
    _markov_m_word_free(words[1]);
    _markov_m_word_free(words[2]);
    _markov_m_word_free(words[3]);

    printf("BEGIN FULL MARKOV\n");
    markov = markov_new();
    char content[] = "hello there friend hello there enemy hello hello";
    char *dyn_content = (char *)malloc(sizeof(char) * (strlen(content) + 1));
    strcpy(dyn_content, content);
    markov_train(markov, dyn_content);

    char *result = markov_generate(markov, "hello", 100);
    printf("result: %s\n", result);
    free(result);
    free(dyn_content);

    markov_writefile(markov, "./markov.dat");

    struct markov_chain *new = markov_fromfile("./markov.dat");
    markov_writefile(new, "./new.dat");
    markov_free(new);

    markov_free(markov);

    return 0;
}