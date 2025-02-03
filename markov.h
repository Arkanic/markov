#include "hashmap.h"

/**
 * Representation of markov word. For internal use.
 */
struct markov_word {
    char *word;
    unsigned int wordlen;
    unsigned long long totaloccurrences;
    struct hm_map *futures; // hm_map<string, markov_wordref>
};

/**
 * Representation of markov word reference. For internal use.
 */
struct markov_wordref {
    struct markov_word *word;
    unsigned int occurrences;
};


/**
 * Markov chain object that contains the full tree, created with markov_new
 * @note Remember to free with markov_free after use.
 * @see markov_new()
 * @see markov_free()
 */
struct markov_chain {
    struct hm_map *words; // hm_map<string, markov_word>
};

/**
 * Create a new, empty markov chain
 * @returns Initialized empty markov chain. Needs to be `markov_free`d after done with use.
 * @see markov_free();
 */
struct markov_chain *markov_new(void);

/**
 * Cleanly erase an old markov chain object from memory.
 * @param markov Chain to be freed
 */
void markov_free(struct markov_chain *markov);

/**
 * Train a markov chain on given text. Text is mangled in process.
 * @param text Text to be processed. Must not be a static pointer to progmem.
 */
void markov_train(struct markov_chain *markov, char *text);

/**
 * Return a "first word" to start the chain off with.
 * @returns Word. This pointer is not a copy, so do not modify or free it.
 */
char *markov_getfirst(struct markov_chain *markov);

/**
 * Generate a chain of text from a pre-trained markov chain
 * @param first First word to kick off the chain. This word must be known to the chain in order to be successful. If NULL will pick own first word based on model.
 * @param maxparticlelen Maximum length of text to produce before stopping.
 */
char *markov_generate(struct markov_chain *markov, char *first, unsigned long maxparticlelen);

/**
 * Write a markov chain to a file. Note that this does not free the `markov_chain` object, and it is still perfectly usable after being written to a file.
 * @param outpath Path of file to be written to. Standard practice is to end the file with the ".mkd" file extension. Any preexisting file will be overwritten.
 * @see markov_fromfile()
 */
void markov_writefile(struct markov_chain *markov, char *outpath);

/**
 * Create a markov chain from a file by reading a .mkd file previously written by `markov_writefile()`.
 * @returns new markov_chain object. This can still be trained further, and saved again. Consider it the same as creating a chain with `markov_new()`.
 * @see markov_writefile()
 */
struct markov_chain *markov_fromfile(char *inpath);