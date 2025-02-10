CC=gcc
OPTS=-Wall
RELEASE_OPTS=-O3 -s

all: markov

release: OPTS += $(RELEASE_OPTS)
release: markov

%.o: %.c
	$(CC) $(OPTS) -c $<

markov.a: markov.o hashmap.o ll.o rand.o
	ar rcs $@ $^

markov.so: markov.o hashmap.o ll.o rand.o
	gcc -shared $(RELEASE_OPTS) $(OPTS) -o $@ $^

markov: bin.o markov.a
	gcc $(OPTS) -o $@ $^

clean:
	rm -f *.o markov.a markov.so markov

.PHONY: clean
