OBJECTS := expr.o parser.o eval.o main.o
OBJ_FILES := $(patsubst %.o, obj/%.o, $(OBJECTS))

CC := gcc

EXEC := mml

CFLAGS := -Wall -Wextra -Wpedantic -g
LDFLAGS := $(CFLAGS) -lm

.PHONY: clean

$(EXEC): Makefile $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(EXEC) $(LDFLAGS)

obj/expr.o: expr.c expr.h
	$(CC) expr.c -c -o obj/expr.o $(CFLAGS)

obj/parser.o: parser.c parser.h token.h expr.h
	$(CC) parser.c -c -o obj/parser.o $(CFLAGS)

obj/eval.o: eval.c eval.h expr.h
	$(CC) eval.c -c -o obj/eval.o $(CFLAGS)

obj/main.o: main.c expr.h token.h
	$(CC) main.c -c -o obj/main.o $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJ_FILES)
