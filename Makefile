SOURCE := $(filter-out %_incl.c,$(wildcard src/*.c))
LIB_SRC := $(filter-out %_incl.c,$(wildcard lib/*.c))

OBJECTS := $(patsubst src/%.c,obj/%.o,$(SOURCE)) $(patsubst lib/%.c,lib/%.o,$(LIB_SRC)) obj/map.o

CC := gcc

EXEC := mml

#NO_DEBUG := -DNDEBUG

FPIC_FLAG := 
CFLAGS := -Wall -Wextra -std=c23 -Iincl -I. $(NO_DEBUG) -O3 -g
LDFLAGS := $(CFLAGS) -lm

.PHONY: cleanobjs clean static_lib shared_lib print_done

all: build obj \
	print_building_func_libs build_func_libs print_done_libs \
	print_building_objects $(OBJECTS) print_done_obj \
	print_building_exe build/$(EXEC) print_done_exe

build/$(EXEC): Makefile $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o build/$(EXEC)

obj/main.o: Makefile src/main.c incl/mml/expr.h incl/mml/token.h incl/mml/parser.h incl/mml/eval.h cvi/dvec/dvec.h
	$(CC) src/main.c -c -o obj/main.o $(CFLAGS) $(FPIC_FLAG)

obj/expr.o: Makefile src/expr.c incl/mml/expr.h incl/mml/config.h cvi/dvec/dvec.h
	$(CC) src/expr.c -c -o obj/expr.o $(CFLAGS) $(FPIC_FLAG)

obj/parser.o: Makefile src/parser.c incl/mml/parser.h incl/mml/token.h incl/mml/expr.h incl/mml/config.h cvi/dvec/dvec.h
	$(CC) src/parser.c -c -o obj/parser.o $(CFLAGS) $(FPIC_FLAG)

obj/eval.o: Makefile src/eval.c incl/mml/eval.h incl/mml/expr.h incl/mml/config.h cvi/dvec/dvec.h
	$(CC) src/eval.c -c -o obj/eval.o $(CFLAGS) $(FPIC_FLAG)

obj/config.o: Makefile src/config.c incl/mml/config.h incl/mml/token.h incl/mml/expr.h incl/mml/eval.h
	$(CC) src/config.c -c -o obj/config.o $(CFLAGS) $(FPIC_FLAG)

obj/prompt.o: Makefile src/prompt.c incl/mml/prompt.h incl/mml/eval.h incl/mml/parser.h cvi/dvec/dvec.h incl/mml/expr.h
	$(CC) src/prompt.c -c -o obj/prompt.o $(CFLAGS) $(FPIC_FLAG)

obj/map.o: Makefile c-hashmap/map.c c-hashmap/map.h
	$(CC) c-hashmap/map.c -Ic-hashmap -c -o obj/map.o $(CFLAGS) $(FPIC_FLAG)


# printing
.PHONY: print_building_exe
print_building_exe:
	@echo --- Building executable from object files \(object files found in 'obj' dir\) ---

.PHONY: print_building_objects
print_building_objects:
	@echo --- Building object files \(source found in 'src' dir\) ---

.PHONY: print_building_func_libs
print_building_func_libs:
	@echo --- Building function libraries \(found in 'lib' dir\) ---

.PHONY: build_func_libs
build_func_libs:
	$(MAKE) -C lib

.PHONY: build_func_libs_shared
build_func_libs_shared:
	$(MAKE) -C lib SHARED=-fPIC

print_done_%:
	@echo --- DONE ---


# directory makers
obj:
	mkdir obj
build:
	mkdir build


# library targets
static_lib: cleanobjs build/lib$(EXEC).a
shared_lib: cleanobjs build/lib$(EXEC).so

build/lib$(EXEC).a: build obj Makefile build_func_libs $(OBJECTS)
	ar rcs build/lib$(EXEC).a $(OBJECTS)

build/lib$(EXEC).so: FPIC_FLAG=-fPIC
build/lib$(EXEC).so: build obj Makefile build_func_libs_shared $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -shared -o build/lib$(EXEC).so


# clean targets
cleanobjs:
	rm -f obj/*
	$(MAKE) -C lib clean
clean: cleanobjs
	rm -f build/*
