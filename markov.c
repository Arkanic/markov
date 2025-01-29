#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "hashmap.h"
#include "ll.h"
#include "rand.h"
#include "markov.h"

struct markov_word *_markov_m_word_create(char *word) {
    struct markov_word *mword = (struct markov_word *)malloc(sizeof(struct markov_word));
    mword->word = malloc(strlen(word) + 1);
    strcpy(mword->word, word);
    mword->wordlen = strlen(word);

    mword->totaloccurrences = 0;

    mword->futures = hm_create(12); // todo hashmap autoinc

    return mword;
}

void _markov_m_word_occurrence(struct markov_word *word, struct markov_word *new_word) {
    struct markov_wordref *reference = (struct markov_wordref *)hm_get(word->futures, new_word->word); // if this is NULL it wasn't present
    if(reference == NULL) {
        // now we need to make a new entry
        reference = (struct markov_wordref *)malloc(sizeof(struct markov_wordref));
        reference->occurrences = 0;
        reference->word = new_word;

        hm_insert(word->futures, new_word->word, reference);
    }

    reference->occurrences++;
    word->totaloccurrences++;
}

void _markov_m_word_free(struct markov_word *word) {
    unsigned long refs_num = word->futures->items;
    struct markov_wordref **refs = hm_freeall(word->futures);
    for(unsigned long i = 0; i < refs_num; i++) {
        struct markov_wordref *ref = refs[i];
        free(ref);
    }
    free(word->word);
    free(word);
}

struct markov_chain *markov_new(void) {
    struct markov_chain *markov = (struct markov_chain *)malloc(sizeof(struct markov_chain));
    markov->words = hm_create(16); // again need to autoinc

    return markov;
}

void markov_free(struct markov_chain *markov) {
    unsigned long words_num = markov->words->items;
    struct markov_word **words = hm_freeall(markov->words);
    for(unsigned long i = 0; i < words_num; i++) _markov_m_word_free(words[i]);
    free(markov);
}

// will consume text
void markov_train(struct markov_chain *markov, char *text) {
    // we want to split words by space, but without all the overhead of copying - just loop to space and replace with \0 temporarily
    unsigned int text_len = strlen(text) + 1;
    char previous[1024] = "";
    char *current = strtok(text, " ");
    while(current != NULL) {
        unsigned int current_len = strlen(current) + 1;
        current_len = current_len > 1024 ? 1024 : current_len;
        memcpy(previous, current, current_len);
        previous[1023] = '\0';

        current = strtok(NULL, " ");

        _markov_train_wordpair_handle(markov, previous, current);
    }
}

struct markov_word *_markov_get_or_insert(struct markov_chain *markov, char *word) {
    struct markov_word *obj = hm_get(markov->words, word);
    if(obj == NULL) {
        obj = _markov_m_word_create(word);
        hm_insert(markov->words, word, obj);
    }

    return obj;
}

// last can be NULL for end of file
void _markov_train_wordpair_handle(struct markov_chain *markov, char *first, char *last) {
    if(last == NULL) last = "";

    struct markov_word *word = _markov_get_or_insert(markov, first);
    struct markov_word *child = _markov_get_or_insert(markov, last);
    _markov_m_word_occurrence(word, child);
}

struct markov_wordref *_markov_generate_getnext(struct markov_word *word) {
    if(word->futures->items == 0) return NULL;

    long long rnum = rand_num(0, word->totaloccurrences);
    struct markov_wordref **futures = hm_values(word->futures);
    struct markov_wordref *future;
    for(int i = 0; i < word->futures->items; i++) {
        if(rnum < 0) break;
        future = futures[i];
        rnum -= future->occurrences;
    }
    free(futures);

    return future;
}

char *markov_generate(struct markov_chain *markov, char *first, unsigned long maxparticlelen) {
    char **output = (char **)malloc(sizeof(char *) * maxparticlelen);

    struct markov_word *current = hm_get(markov->words, first);
    if(current == NULL) return NULL;

    rand_init();

    unsigned long outlen;
    unsigned long long output_bufsize = 0;
    for(outlen = 0; outlen < maxparticlelen; outlen++) {
        if(!strcmp(current->word, "")) break;
        output[outlen] = current->word;
        output_bufsize += current->wordlen;

        current = _markov_generate_getnext(current)->word;
    }

    char *output_buf = (char *)malloc(sizeof(char) * (output_bufsize + outlen + 1));
    char *output_buf_index = output_buf;
    for(int i = 0; i < outlen; i++) {
        unsigned int len = strlen(output[i]);
        memcpy(output_buf_index, output[i], len);
        output_buf_index += len;
        *output_buf_index = ' ';
        output_buf_index++;
    }
    output_buf_index = '\0';

    return output_buf;
}