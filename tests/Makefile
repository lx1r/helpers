#!/bin/make -f

.PHONY: all clean

CFLAGS = -g -I.. -Wall -Wfatal-errors
LDFLAGS = -lgh -L..

sources = $(wildcard *.c)
targets = $(sources:.c=)

all: $(targets)

%: %.c ../helpers.h ../helpers.c
	@$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
	@printf '  %-8s %-14s' "TEST" "$@"
	-@./$@ > $@.log
	-@diff $@.log $@.cmp > $@.diff && echo -e "OK"

clean:
	$(RM) $(targets) *.log *.diff
