CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -O2
SRC     := $(wildcard src/*.c)
HDR     := $(wildcard src/*.h)
BIN     := lenguajito

.PHONY: all test test-hito1 test-hito2 test-hito3 test-hito4 clean

all: $(BIN)

$(BIN): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $@ $(SRC)

test: $(BIN)
	python3 tests/run.py

test-hito1: $(BIN)
	python3 tests/run.py --hito1

test-hito2: $(BIN)
	python3 tests/run.py --hito2

test-hito3: $(BIN)
	python3 tests/run.py --hito3

test-hito4: $(BIN)
	python3 tests/run.py --hito4

clean:
	rm -f $(BIN)
