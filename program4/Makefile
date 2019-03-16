SRC_DIR=src
INC_DIR=include
OBJ_DIR=obj
BIN_DIR=bin

SRC=$(wildcard $(SRC_DIR)/*.c)
INC=$(wildcard $(INC_DIR)/*.h)
OBJ=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

CFLAGS=-std=c89 -Wall -g

_progs=keygen
progs=$(patsubst %, $(BIN_DIR)/%, $(_progs))

all: $(progs)

$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	gcc $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC)
	gcc $(CFLAGS) -c $< -o $@ -I$(INC_DIR)

.PHONY: clean

clean:
	rm -f $(OBJ_DIR)/* $(BIN_DIR)/*
