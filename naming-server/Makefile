SRC=$(wildcard src/*.c) 

br: build run
build :
	gcc -g $(SRC) -fsanitize=address -o bin/naming-server

run:
	./bin/naming-server
