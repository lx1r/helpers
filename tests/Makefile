#!/bin/make -f

.PHONY: all clean

CFLAGS = -I.. -Wall -Wfatal-errors
LDFLAGS = -lgh -L..

sources = $(wildcard *.c)
targets = $(sources:.c=)
outputs = $(sources:.c=.log)

all: $(targets)

%: %.c ../helpers.h ../helpers.c
	@gcc $(CFLAGS) $< -o $@ $(LDFLAGS)
	@printf '  %-8s %-14s ' "TEST" "$@"
	-@./$@ > $@.log
	-@diff $@.log $@.cmp > /dev/null && echo -e "OK"

clean:
	rm -f $(targets) $(outputs)
