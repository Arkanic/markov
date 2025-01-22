#include <stdio.h>
#include <stdlib.h>

#include "markov.h"
#include "hashmap.h"

int main(void) {
    struct markov_chain *markov = markov_new();

    struct markov_word *word1 = _markov_m_word_create("hello");
    struct markov_word *word2 = _markov_m_word_create("world");
    _markov_m_word_occurence(word1, word2);

    struct markov_wordref *ref = (struct markov_wordref *)hm_get(word1->futures, "world");
    printf("%s, %d\n", ref->word->word, ref->occurences);

    char *txt = "hello there 1234";
    char *text = malloc(sizeof(char) * (strlen(txt) + 1));
    strcpy(text, txt);
    markov_train(markov, text);
    free(text);

    _markov_m_word_free(word1);
    _markov_m_word_free(word2);

    markov_free(markov);

    return 0;
}