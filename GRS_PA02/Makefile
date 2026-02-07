#  MT25017

CC = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
GNUFLAGS = -D_GNU_SOURCE

TARGETS = \
    A1_server A1_client \
    A2_server A2_client \
    A3_server A3_client

all: $(TARGETS)

A1_server: MT25017_PartA1_Server.c
	$(CC) $(CFLAGS) $< -o $@

A1_client: MT25017_PartA1_Client.c
	$(CC) $(CFLAGS) $< -o $@

A2_server: MT25017_PartA2_Server.c
	$(CC) $(CFLAGS) $< -o $@

A2_client: MT25017_PartA2_Client.c
	$(CC) $(CFLAGS) $< -o $@

A3_server: MT25017_PartA3_Server.c
	$(CC) $(CFLAGS) $< -o $@

A3_client: MT25017_PartA3_Client.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

.PHONY: all clean