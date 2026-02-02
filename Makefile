#CHELES Giorgio, 312CB
CC=gcc
TARGETS=runic

# Manually define all targets.
build: $(TARGETS)

runic: main.c functions.c
	$(CC) -Wall -Wextra -std=c99 main.c functions.c snapshot.c -lm -o runic

clean:
	rm -f runic
