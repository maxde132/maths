CFLAGS := -I../incl -I.. -Wall -Wextra -Wpedantic -std=c23 -O3 -g
LDFLAGS := -O3 -g -fPIC
CC := gcc

SRC := $(wildcard *.c)
OUT := $(patsubst %.c,%.o,$(SRC))

all: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f $(OUT)
