CC=gcc
OPTS=

%.o: %.c
	$(CC) $(OPTS) -c $<

markov.a: markov.o hashmap.o ll.o rand.o
	ar rcs $@ $^

test: test.o markov.a
	gcc $(OPTS) -o $@ $^
	./test

testmarkov: testmarkov.o markov.a
	gcc $(OPTS) -o $@ $^

clean:
	rm -f *.o test markov.a

.PHONY: clean