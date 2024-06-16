CC = gcc
CFLAGS = -O2 -Wall -Wextra -Wno-unused-parameter -lX11 -lXi


all: xinputmon

clean:
	rm -f xinputmon

xinputmon: xinputmon.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all clean
