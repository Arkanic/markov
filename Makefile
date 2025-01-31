CC=gcc
OPTS=

all: markov

%.o: %.c
	$(CC) $(OPTS) -c $<

markov.a: markov.o hashmap.o ll.o rand.o
	ar rcs $@ $^

markov: bin.o markov.a
	gcc $(OPTS) -o $@ $^

test: test.o markov.a
	gcc $(OPTS) -o $@ $^
	./test

testmarkov: testmarkov.o markov.a
	gcc $(OPTS) -o $@ $^

clean:
	rm -f *.o test markov.a

.PHONY: clean