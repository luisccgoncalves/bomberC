# Compiler
CC		= gcc

default:
	mkdir -p bin
	$(CC) server.c -o ./bin/server -lpthread -lncurses
	$(CC) client.c -o ./bin/client -lpthread -lncurses
	$(CC) gmech.c -o ./bin/gmech -lpthread -lncurses
	cp default.lvl ./bin/default.lvl

