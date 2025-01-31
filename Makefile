CC=gcc
OPTS=-Wall
RELEASE_OPTS=-O3

all: markov

release: OPTS += $(RELEASE_OPTS)
release: markov markov.so

%.o: %.c
	$(CC) $(OPTS) -c $<

markov.a: markov.o hashmap.o ll.o rand.o
	ar rcs $@ $^

markov.so: markov.a
	gcc -shared $(OPTS) -o $@ $^

markov: bin.o markov.a
	gcc $(OPTS) -o $@ $^

clean:
	rm -f *.o markov.a markov

.PHONY: clean
