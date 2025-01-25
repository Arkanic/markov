#include "hashmap.h"

struct markov_word {
    char *word;
    unsigned int wordlen;
    unsigned long long totaloccurrences;
    struct hm_map *futures; // hm_map<string, markov_wordref>
};

struct markov_wordref {
    struct markov_word *word;
    unsigned int occurrences;
};

struct markov_chain {
    struct hm_map *words; // hm_map<string, markov_word>
};

struct markov_chain *markov_new(void);
void markov_free(struct markov_chain *markov);
void markov_train(struct markov_chain *markov, char *text);
char *markov_generate(struct markov_chain *markov, char *first, unsigned long maxparticlelen);

// dev functions for testing
struct markov_word *_markov_m_word_create(char *word);
void _markov_m_word_occurrence(struct markov_word *word, struct markov_word *new_word);
void _markov_m_word_free(struct markov_word *word);
struct markov_wordref *_markov_generate_getnext(struct markov_word *word);