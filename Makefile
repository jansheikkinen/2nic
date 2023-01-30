CC = gcc
CFLAGS = -Wall -Wextra -Wformat=2 -Wshadow \
				 -Wwrite-strings -Wstrict-prototypes -g $(LIBS)
ifeq ($(CC),gcc)
	CFLAGS += -Wjump-misses-init -Wlogical-op
endif
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
