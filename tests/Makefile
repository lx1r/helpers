#!/bin/make -f

.PHONY: all clean

CFLAGS=-I.. -Wall -Wfatal-errors

sources = $(wildcard *.c)
targets = $(sources:.c=)
outputs = $(sources:.c=.log)

all: $(targets)

%: %.c ../helpers.h
	@gcc $(CFLAGS) $< -o $@
	@printf '  %-8s %-14s ' "TEST" "$@"
	-@./$@ > $@.log
	-@diff $@.log $@.cmp > /dev/null && echo -e "OK"

clean:
	rm -f $(targets) $(outputs)
