#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

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

void _markov_m_word_occurrence(struct markov_word *word, struct markov_word *new_word, unsigned int count) {
    struct markov_wordref *reference = (struct markov_wordref *)hm_get(word->futures, new_word->word); // if this is NULL it wasn't present
    if(reference == NULL) {
        // now we need to make a new entry
        reference = (struct markov_wordref *)malloc(sizeof(struct markov_wordref));
        reference->occurrences = 0;
        reference->word = new_word;

        hm_insert(word->futures, new_word->word, reference);
    }

    reference->occurrences += count;
    word->totaloccurrences += count;
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
    _markov_m_word_occurrence(word, child, 1);
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

/*
# file format:
----
## markov word
word (%s with space instead of null terminator)
<futures>
## markov future
word (lu to line in file containing word, first line is line 0)
occurences (du)
*/
// will overwrite any current file
void markov_writefile(struct markov_chain *markov, char *outpath) {
    struct markov_word **words = hm_values(markov->words);
    // min size required to fit all elements
    unsigned int pow = (sizeof(unsigned int) * 8) - __builtin_clzll(markov->words->items);
    struct hm_map *word_pos_map = hm_create(pow);
    for(unsigned long i = 0; i < markov->words->items; i++) {
        struct markov_word *word = words[i];
        hm_insert(word_pos_map, word->word, i);
    }

    FILE *fp = fopen(outpath, "w");

    // all words are now in map, start appending
    for(int i = 0; i < markov->words->items; i++) {
        struct markov_word *word = words[i];
        fwrite(word->word, word->wordlen, 1, fp);
        fputc(' ', fp);

        struct markov_wordref **futures = hm_values(word->futures);
        for(int j = 0; j < word->futures->items; j++) {
            struct markov_wordref *future = futures[j];
            unsigned long _futurepos = (unsigned long)hm_get(word_pos_map, future->word->word);
            uint32_t futurepos = _futurepos;
            uint32_t occurrences = future->occurrences;
            fwrite(&futurepos, sizeof(uint32_t), 1, fp);
            fwrite(&occurrences, sizeof(uint32_t), 1, fp);
        }
        fputc('\n', fp);
        free(futures);
    }

    fclose(fp);

    free(words);
    free(hm_freeall(word_pos_map));
}

struct temp_markov_word {
    char *word;
    struct ll_list *futures; // ll_list<&temp_markov_wordref>
};

struct temp_markov_wordref {
    unsigned long future;
    unsigned int occurrences;
};

struct markov_chain *markov_fromfile(char *inpath) {
    FILE *fp = fopen(inpath, "r");
    struct markov_chain *chain = markov_new();

    struct ll_list *temp_markov_words = ll_create();

    unsigned short loop = 1;
    char word[256];
    while(loop) {
        // get next word
        int c = fgetc(fp);
        ungetc(c, fp);
        if(c == ' ') word[0] = '\0';
        else fscanf(fp, "%255s", word);
        fgetc(fp); // delete space
        
        struct temp_markov_word *temp_word = (struct temp_markov_word *)malloc(sizeof(struct temp_markov_word));
        ll_push(temp_markov_words, temp_word);
        temp_word->word = (char *)malloc(sizeof(char) * (strlen(word) + 1));
        strcpy(temp_word->word, word);
        temp_word->futures = ll_create();

        struct markov_word *mword = _markov_m_word_create(temp_word->word);
        hm_insert(chain->words, temp_word->word, mword);

        c = fgetc(fp);
        ungetc(c, fp);
        while(c != '\n') {
            struct temp_markov_wordref *ref = (struct temp_markov_wordref *)malloc(sizeof(struct temp_markov_wordref));
            uint32_t future;
            uint32_t occurrences;
            fread(&future, sizeof(uint32_t), 1, fp);
            fread(&occurrences, sizeof(uint32_t), 1, fp);
            ref->future = future;
            ref->occurrences = occurrences;

            ll_push(temp_word->futures, ref);

            c = fgetc(fp);
            ungetc(c, fp);
        }

        fgetc(fp); // delete newline

        // now check for EOF
        c = fgetc(fp);
        if(c == EOF) loop = 0;
        ungetc(c, fp);
    }

    unsigned int tempwordlen = ll_length(temp_markov_words);
    struct temp_markov_word **temp_words = ll_freeall(temp_markov_words);

    for(unsigned int i = 0; i < tempwordlen; i++) {
        struct temp_markov_word *temp_word = temp_words[i];

        struct markov_word *word = hm_get(chain->words, temp_word->word);

        unsigned int tempwordrefslen = ll_length(temp_word->futures);
        for(int j = 0; j < tempwordrefslen; j++) {
            struct temp_markov_wordref *temp_wordref = ll_get(temp_word->futures, j);

            struct temp_markov_word *temp_future = temp_words[temp_wordref->future];
            struct markov_word *future = hm_get(chain->words, temp_future->word);

            _markov_m_word_occurrence(word, future, temp_wordref->occurrences);

            free(temp_wordref);
        }
    }

    for(unsigned int i = 0; i < tempwordlen; i++) {
        struct temp_markov_word *temp_word = temp_words[i];
        free(temp_word->word);
        free(temp_word);
    }

    free(temp_words);

    struct markov_word **words = hm_values(chain->words);
    for(int i = 0; i < chain->words->items; i++) {
        _markov_m_word_debug_print(words[i]);
    }
    free(words);

    return chain;
}

void _markov_m_word_debug_print(struct markov_word *word) {
    printf("word \"%s\":\n wordlen: %u\n totaloccurences: %llu\n futures:\n", word->word, word->wordlen, word->totaloccurrences);
    struct markov_wordref **refs = hm_values(word->futures);
    for(int i = 0; i < word->futures->items; i++) {
        _markov_m_wordref_debug_print(refs[i]);
    }
    free(refs);
}

void _markov_m_wordref_debug_print(struct markov_wordref *ref) {
    printf("  ref \"%s\": %u occurrences\n", ref->word->word, ref->occurrences);
}