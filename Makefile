CC=gcc

%.o: %.c
	$(CC) -c $<

markov.a: markov.o hashmap.o ll.o
	ar rcs $@ $^

test: test.o markov.a
	gcc -o $@ $^
	./test

testmarkov: testmarkov.o markov.a
	gcc -o $@ $^
	./testmarkov

clean:
	rm -f *.o test markov.a

.PHONY: clean