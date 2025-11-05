CFLAGS=-std=c11 -g -static
CC=gcc
SRCS=$(wildcard src/*.c src/lib/*.c)
OBJS=$(patsubst src/%.c,build/%.o,$(SRCS))

all: build/9cc

build/9cc: $(OBJS) | build
		$(CC) -o $@ $(OBJS) $(LDFLAGS)

build:
	mkdir -p build

build/%.o: src/%.c | build
		mkdir -p $(dir $@)
		$(CC) $(CFLAGS) -c $< -o $@

test: build/9cc
		./build/9cc ./test/test.c > build/tmp.s
		$(CC) -o build/tmp build/tmp.s build/lib/*.o
		./build/tmp

clean:
		rm -rf build *~ *.o tmp*

.PHONY: test clean
