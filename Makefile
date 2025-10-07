# Makefile

CC = gcc
CFLAGS = -Wall -Wextra
SRC = src/ls-v1.0.0.c
OUT = bin/ls

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OUT)

