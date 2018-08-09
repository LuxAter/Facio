CC=clang

all: facio

.PHONY: facio
facio: main.c output.c output.h file.h file.c lexer.h lexer.c token.h ast.c ast.h util.h util.c parser.h parser.c targets.c targets.h
	$(CC) main.c output.c file.c lexer.c ast.c util.c parser.c targets.c -lbsd -lm -o facio 
