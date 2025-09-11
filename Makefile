OBJECTS := expr.o parser.o eval.o main.o config.o map.o
OBJ_FILES := $(patsubst %.o, obj/%.o, $(OBJECTS))

CC := gcc

EXEC := mml

CFLAGS := -Wall -Wextra -std=c23 -g
LDFLAGS := $(CFLAGS) -lm

.PHONY: clean

all: $(EXEC)

$(EXEC): obj Makefile $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(EXEC) $(LDFLAGS)

lib$(EXEC).so: obj Makefile $(OBJ_FILES) forbidden_realm/python_interface.c
	$(CC) $(OBJ_FILES) -o lib$(EXEC).so $(LDFLAGS) -I. forbidden_realm/python_interface.c -shared

obj/expr.o: Makefile expr.c expr.h
	$(CC) expr.c -c -o obj/expr.o $(CFLAGS)

obj/parser.o: Makefile parser.c parser.h token.h expr.h
	$(CC) parser.c -c -o obj/parser.o $(CFLAGS)

obj/eval.o: Makefile eval.c eval.h expr.h eval_funcs_incl.c
	$(CC) eval.c -c -o obj/eval.o $(CFLAGS)

obj/main.o: Makefile main.c expr.h token.h
	$(CC) main.c -c -o obj/main.o $(CFLAGS)

obj/config.o: Makefile config.c config.h token.h
	$(CC) config.c -c -o obj/config.o $(CFLAGS)

obj/map.o: Makefile c-hashmap/map.c c-hashmap/map.h
	$(CC) c-hashmap/map.c -Ic-hashmap -c -o obj/map.o $(CFLAGS)

obj:
	mkdir obj

clean:
	rm -f $(EXEC) $(OBJ_FILES)
