SRC=$(wildcard src/*.c) 

U_NAME ?= Mayaank

br: build run
build :
	gcc -g $(SRC) -fsanitize=address -o bin/client-server

run:
	./bin/client-server $(U_NAME)