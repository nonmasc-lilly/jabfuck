
BINDIR ?= /usr/local/bin

all: comp.c
	gcc comp.c -o jabf -std=c89

install: all
	cp -r jabf /usr/local/bin

test: all
	./jabf cat.bf -o cat.asm
	nasm cat.asm -o cat.o -felf64
	ld cat.o -o cat

clean:
	rm -rf *.o *.asm
