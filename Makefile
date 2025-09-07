CC := gcc

OBJECTS := expr.o parser.o eval.o main.o

EXEC = mml

CFLAGS := -Wall -Wextra -Wpedantic -g
LDFLAGS := $(CFLAGS) -lm

.PHONY: clean

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS)

expr.o: expr.c expr.h
	$(CC) expr.c -c -o expr.o $(CFLAGS)

parser.o: parser.c parser.h token.h expr.h
	$(CC) parser.c -c -o parser.o $(CFLAGS)

eval.o: eval.c eval.h expr.h
	$(CC) eval.c -c -o eval.o $(CFLAGS)

main.o: main.c expr.h token.h
	$(CC) main.c -c -o main.o $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJECTS)
