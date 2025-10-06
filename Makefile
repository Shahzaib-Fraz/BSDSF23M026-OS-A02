CC = gcc
CFLAGS = -Wall -Wextra

V1 = src/ls-v1.0.0.c
V2 = src/ls-v1.1.0.c

all: ls-v1.0.0 ls-v1.1.0

ls-v1.0.0: $(V1)
	$(CC) $(CFLAGS) -o ls-v1.0.0 $(V1)

ls-v1.1.0: $(V2)
	$(CC) $(CFLAGS) -o ls-v1.1.0 $(V2)

clean:
	rm -f ls-v1.0.0 ls-v1.1.0
