OBJECTS := main.o expr.o parser.o eval.o config.o prompt.o map.o 
OBJ_FILES := $(patsubst %.o, obj/%.o, $(OBJECTS))

CC := gcc

EXEC := mml

FPIC_FLAG := 
CFLAGS := -Wall -Wextra -std=c23 -Iincl -I. -DNDEBUG -g
LDFLAGS := $(CFLAGS) -lm

.PHONY: cleanobjs clean static_lib shared_lib

all: build/$(EXEC)

build/$(EXEC): build obj Makefile $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(LDFLAGS) -o build/$(EXEC)

build/lib$(EXEC).a: build obj Makefile $(OBJ_FILES)
	ar rcs build/lib$(EXEC).a $(OBJ_FILES)

build/lib$(EXEC).so: FPIC_FLAG=-fPIC
build/lib$(EXEC).so: build obj Makefile $(OBJ_FILES) 
	$(CC) $(OBJ_FILES) $(LDFLAGS) -shared -o build/lib$(EXEC).so

obj/main.o: Makefile src/main.c incl/mml/expr.h incl/mml/token.h incl/mml/parser.h incl/mml/eval.h incl/dvec/dvec.h
	$(CC) src/main.c -c -o obj/main.o $(CFLAGS) $(FPIC_FLAG)

obj/expr.o: Makefile src/expr.c incl/mml/expr.h incl/mml/config.h incl/dvec/dvec.h
	$(CC) src/expr.c -c -o obj/expr.o $(CFLAGS) $(FPIC_FLAG)

obj/parser.o: Makefile src/parser.c incl/mml/parser.h incl/mml/token.h incl/mml/expr.h incl/dvec/dvec.h
	$(CC) src/parser.c -c -o obj/parser.o $(CFLAGS) $(FPIC_FLAG)

obj/eval.o: Makefile src/eval.c incl/mml/eval.h incl/mml/expr.h incl/mml/config.h src/eval_funcs_incl.c incl/dvec/dvec.h
	$(CC) src/eval.c -c -o obj/eval.o $(CFLAGS) $(FPIC_FLAG)

obj/config.o: Makefile src/config.c incl/mml/config.h incl/mml/token.h incl/mml/expr.h incl/mml/eval.h
	$(CC) src/config.c -c -o obj/config.o $(CFLAGS) $(FPIC_FLAG)

obj/prompt.o: Makefile src/prompt.c incl/mml/prompt.h incl/mml/eval.h incl/mml/parser.h incl/dvec/dvec.h
	$(CC) src/prompt.c -c -o obj/prompt.o $(CFLAGS) $(FPIC_FLAG)

obj/map.o: Makefile c-hashmap/map.c c-hashmap/map.h
	$(CC) c-hashmap/map.c -Ic-hashmap -c -o obj/map.o $(CFLAGS) $(FPIC_FLAG)

obj:
	mkdir obj
build:
	mkdir build

static_lib: cleanobjs build/lib$(EXEC).a
shared_lib: cleanobjs build/lib$(EXEC).so

cleanobjs:
	rm -f obj/*
clean: cleanobjs
	rm -f build/*
