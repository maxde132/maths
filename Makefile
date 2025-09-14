OBJECTS := expr.o parser.o eval.o main.o config.o map.o
OBJ_FILES := $(patsubst %.o, obj/%.o, $(OBJECTS))

CC := gcc

EXEC := mml

CFLAGS := -Wall -Wextra -std=c23 -Iincl -I. -DNDEBUG -g
LDFLAGS := $(CFLAGS) -lm

.PHONY: clean

all: $(EXEC)

$(EXEC): obj Makefile $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(EXEC) $(LDFLAGS)

lib$(EXEC).so: obj Makefile $(OBJ_FILES) forbidden_realm/python_interface.c
	$(CC) $(OBJ_FILES) -o lib$(EXEC).so $(LDFLAGS) -I. forbidden_realm/python_interface.c -shared

obj/expr.o: Makefile src/expr.c incl/expr.h
	$(CC) src/expr.c -c -o obj/expr.o $(CFLAGS)

obj/parser.o: Makefile src/parser.c incl/parser.h incl/token.h incl/expr.h
	$(CC) src/parser.c -c -o obj/parser.o $(CFLAGS)

obj/eval.o: Makefile src/eval.c incl/eval.h incl/expr.h incl/config.h src/eval_funcs_incl.c
	$(CC) src/eval.c -c -o obj/eval.o $(CFLAGS)

obj/main.o: Makefile src/main.c incl/expr.h incl/token.h
	$(CC) src/main.c -c -o obj/main.o $(CFLAGS)

obj/config.o: Makefile src/config.c incl/config.h incl/token.h
	$(CC) src/config.c -c -o obj/config.o $(CFLAGS)

obj/map.o: Makefile c-hashmap/map.c c-hashmap/map.h
	$(CC) c-hashmap/map.c -Ic-hashmap -c -o obj/map.o $(CFLAGS)

obj:
	mkdir obj

clean:
	rm -f $(EXEC) obj/*
