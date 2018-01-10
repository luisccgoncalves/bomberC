# Compiler
CC		= gcc

default:
	mkdir -p bin
	$(CC) *.h server.c -o ./bin/server -lpthread -lncurses
	$(CC) *.h client.c -o ./bin/client -lpthread -lncurses
	cp default.lvl ./bin/default.lvl

