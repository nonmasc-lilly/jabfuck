
BINDIR ?= /usr/local/bin

all: comp.c
	gcc comp.c -o jabf -std=c89

install: all
	cp -r jabf /usr/local/bin
