# Compiler
CC		= gcc

default:
	mkdir -p bin
	$(CC) *.h server.c -o ./bin/server
	$(CC) *.h client.c -o ./bin/client
