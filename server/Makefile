CC=gcc
CFLAGS=-std=gnu99 -pedantic -Wall -O3

all: inkmonitor-server

inkmonitor-server: exceptions.o ini_parser.o main.o options.o profiler.o screen.o transfer.o
	$(CC) $(CFLAGS) -o inkmonitor-server exceptions.o ini_parser.o main.o options.o profiler.o screen.o transfer.o -lgiblib -lImlib2 -lX11

exceptions.o: ../common/exceptions.c
	$(CC) $(CFLAGS) -c ../common/exceptions.c

ini_parser.o: ../common/ini_parser.c
	$(CC) $(CFLAGS) -c ../common/ini_parser.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

options.o: options.c
	$(CC) $(CFLAGS) -c options.c

profiler.o: profiler.c
	$(CC) $(CFLAGS) -c profiler.c

screen.o: screen.c
	$(CC) $(CFLAGS) -c screen.c
	
transfer.o: transfer.c
	$(CC) $(CFLAGS) -c transfer.c

clean:
	rm -f *.o inkmonitor-server
