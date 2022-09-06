EXECUTABLE = proj2
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread 

.PHONY: all clean

all: clean $(EXECUTABLE)

$(EXECUTABLE):proj2.o
	$(CC) $(CFLAGS) -o $@ $^

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f $(EXECUTABLE) *.o
	rm -f $(EXECUTABLE) *.out
