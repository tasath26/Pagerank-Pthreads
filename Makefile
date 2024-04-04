CC = gcc
CFLAGS = -pthread -g -o

SRC = pagerank.c

OUT = pagerank

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(OUT) $(SRC)
	@echo "Usage: ./ask1_4579 <filename> <num of threads>"

clean:
	rm -f $(OUT)
