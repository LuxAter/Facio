SHELL=/usr/bin/bash

CC=clang

all: facio

facio: main.c output.c output.h file.h file.c lexer.h lexer.c token.h
	$(CC) main.c output.c file.c lexer.c -lbsd -lm -o facio 
