CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g $(LIBS)
INCLUDE = -Iinclude

SRC = $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJ = $(SRC:.c=.o)
BUILD = build

all: clear dirs clean run

intercept: clean
	intercept-build --append make build

dirs:
	mkdir -p $(BUILD) src tests

run: build
	$(BUILD)/build

build: $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BUILD)/$@ $?

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	rm -Rf $(OBJ)

clear:
	clear
