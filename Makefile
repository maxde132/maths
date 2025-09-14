OBJECTS := main.o expr.o parser.o eval.o config.o prompt.o map.o 
OBJ_FILES := $(patsubst %.o, obj/%.o, $(OBJECTS))

CC := gcc

EXEC := mml

CFLAGS := -Wall -Wextra -std=c23 -Iincl -I. -DNDEBUG -g
LDFLAGS := $(CFLAGS) -lm

.PHONY: clean build_all

all: build/$(EXEC)

build/$(EXEC): build obj Makefile $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(LDFLAGS) -o build/$(EXEC)

build/lib$(EXEC).a: build obj Makefile $(OBJ_FILES)
	ar rcs build/lib$(EXEC).a $(OBJ_FILES)

build/lib$(EXEC).so: build obj Makefile $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(LDFLAGS) -shared -o build/lib$(EXEC).so

build_all: build/$(EXEC) build/lib$(EXEC).a build/lib$(EXEC).so

obj/main.o: Makefile src/main.c incl/mml/expr.h incl/mml/token.h incl/mml/parser.h incl/mml/eval.h
	$(CC) src/main.c -c -o obj/main.o $(CFLAGS)

obj/expr.o: Makefile src/expr.c incl/mml/expr.h incl/mml/config.h
	$(CC) src/expr.c -c -o obj/expr.o $(CFLAGS)

obj/parser.o: Makefile src/parser.c incl/mml/parser.h incl/mml/token.h incl/mml/expr.h
	$(CC) src/parser.c -c -o obj/parser.o $(CFLAGS)

obj/eval.o: Makefile src/eval.c incl/mml/eval.h incl/mml/expr.h incl/mml/config.h src/eval_funcs_incl.c
	$(CC) src/eval.c -c -o obj/eval.o $(CFLAGS)

obj/config.o: Makefile src/config.c incl/mml/config.h incl/mml/token.h incl/mml/expr.h incl/mml/eval.h
	$(CC) src/config.c -c -o obj/config.o $(CFLAGS)

obj/prompt.o: Makefile src/prompt.c incl/mml/prompt.h incl/mml/eval.h incl/mml/parser.h
	$(CC) src/prompt.c -c -o obj/prompt.o $(CFLAGS)

obj/map.o: Makefile c-hashmap/map.c c-hashmap/map.h
	$(CC) c-hashmap/map.c -Ic-hashmap -c -o obj/map.o $(CFLAGS)

obj:
	mkdir obj
build:
	mkdir build

clean:
	rm -f build/* obj/*
