#!/bin/make -f

.PHONY: all clean

CFLAGS=-I.. -Wall -Wfatal-errors
sources = $(wildcard *.c)
headers = $(wildcard *.h)
tests = $(sources:.c=)

all: README.md $(tests)

%: %.c $(headers)
	@gcc $(CFLAGS) $< -o $@
	@echo -n running $@...
	-@./$@ > $@.log
	-@diff $@.log $@.cmp > /dev/null && echo ok

clean:
	$(RM) *.o *.log $(tests)

README.md: helpers.h
	@echo gen $@
	@cat helpers.h | grep "^ \*" | \
	sed -r \
	-e "s/.*(@fn )(.*)/\n\`\2\`/" \
	-e "s/.*(@brief )(.*)/\2/" \
	-e "s/.*(@param )([a-z0-9_]*)(.*)/* \`\2\` - \3/" \
	-e "s/.*(@return )(.*)/\*\*Returns:\*\* \2/" \
	-e "s/^ \* (.*)/\1/" \
	-e "s/^ \*\//\n---/" \
	-e "s/^ \*(.*)//" > $@
