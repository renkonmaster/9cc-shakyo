CFLAGS=-std=c11 -g -static
#SRCS=$(wildcard *.c)
SRCS=codegen.c parce.c token.c main.c
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
		./test.sh

clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test clean
