CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2

all: som_das_seis

som_das_seis: main.c
	$(CC) $(CFLAGS) main.c -o som_das_seis

clean:
	rm -f som_das_seis som_das_seis.exe
