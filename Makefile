CC=gcc
CFLAGS=-Wall -g -I./shared
SFLAGS=-I./server/include
.DEFAULT_GOAL=all

DCOUNT=5
TCOUNT=2

CLICOUNT=2
DLEVEL=1

VSIZE=10
FNAME=./data/dataset
MQNAME=mqueue

all: server-build client-build

server-build: 
	$(CC) ./server/*.c ./shared/*.c -o ./bin/serverk $(CFLAGS) $(SFLAGS)

server-run:
	./bin/serverk -d $(DCOUNT) -t $(TCOUNT) -s $(VSIZE) -m $(MQNAME) -f $(FNAME)

server-leak:
	valgrind --leak-check=yes ./bin/serverk -d $(DCOUNT) -t $(TCOUNT) -s $(VSIZE) -m $(MQNAME) -f $(FNAME)

client-build:
	$(CC) ./client/*.c ./shared/*.c -o ./bin/clientk $(CFLAGS)

client-run:
	./clientk -f $(FNAME) -m $(MQNAME) -s $(VSIZE) -n $(CLICOUNT) -d $(DLEVEL)

client-leak:
	valgrind --leak-check=yes ./clientk -f $(FNAME) -m $(MQNAME) -s $(VSIZE) -n $(CLICOUNT) -d $(DLEVEL)

clean:
	rm -f serverk clientk ./data/*