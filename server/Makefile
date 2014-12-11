CC=gcc
CFLAGS=-std=gnu99 -pedantic -Wall -O3

all: inkmonitor-server

inkmonitor-server: control.o exceptions.o ini_parser.o main.o options.o profiler.o screen.o shortcuts.o transfer.o
	$(CC) $(CFLAGS) -o inkmonitor-server control.o exceptions.o ini_parser.o main.o options.o profiler.o screen.o shortcuts.o transfer.o -lgiblib -lImlib2 -lpthread -lrt -lX11

control.o: control.c
	$(CC) $(CFLAGS) -c control.c

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

shortcuts.o: shortcuts.c
	$(CC) $(CFLAGS) -c shortcuts.c
	
transfer.o: transfer.c
	$(CC) $(CFLAGS) -c transfer.c

clean:
	rm -f *.o inkmonitor-server
