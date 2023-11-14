CC=gcc
CFLAGS=-Wall -g
.DEFAULT_GOAL=build

DCOUNT=5
TCOUNT=2

CLICOUNT=2
DLEVEL=1

VSIZE=5
FNAME=./data/dataset
MQNAME=mqueue

server-build: 
	$(CC) ./server/*.c -o ./bin/serverk $(CFLAGS)

server-run:
	./bin/serverk -d $(DCOUNT) -t $(TCOUNT) -s $(VSIZE) -m $(MQNAME) -f $(FNAME)

server-leak:
	valgrind --leak-check=yes ./bin/serverk -d $(DCOUNT) -t $(TCOUNT) -s $(VSIZE) -m $(MQNAME) -f $(FNAME)

client-build:
	$(CC) clientk_args.c clientk.c -o clientk $(CFLAGS)

client-run:
	./clientk -f $(FNAME) -m $(MQNAME) -s $(VSIZE) -n $(CLICOUNT) -d $(DLEVEL)

client-leak:
	valgrind --leak-check=yes ./clientk -f $(FNAME) -m $(MQNAME) -s $(VSIZE) -n $(CLICOUNT) -d $(DLEVEL)

clean:
	rm -f serverk clientk