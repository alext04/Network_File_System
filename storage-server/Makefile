SRC=$(wildcard src/*.c) 

SS ?= 1
CLIENT ?= 5432
NAMING ?= 5555
SERVER ?= 6000
br: build run
build :
	gcc -g $(SRC) -fsanitize=address -o bin/storage-server

run:
	./bin/storage-server $(SS) $(CLIENT) $(NAMING) $(SERVER)
