CC=gcc
CFLAGS=-Wall -O3

all: inkmonitor-server

inkmonitor-server: main.o ini_parser.o options.o profiler.o screen.o transfer.o
	$(CC) $(CFLAGS) -o inkmonitor-server ini_parser.o main.o options.o profiler.o screen.o transfer.o -lgiblib -lImlib2 -lX11

ini_parser.o: ../common/ini_parser.c
	$(CC) $(CFLAGS) -c ../common/ini_parser.c

main.o: main.c main.h options.h profiler.h screen.h profiler.h
	$(CC) $(CFLAGS) -c main.c

options.o: options.c main.h options.h
	$(CC) $(CFLAGS) -c options.c

profiler.o: profiler.c profiler.h
	$(CC) $(CFLAGS) -c profiler.c

screen.o: screen.c screen.h
	$(CC) $(CFLAGS) -c screen.c
	
transfer.o: transfer.c main.h options.h profiler.h transfer.h
	$(CC) $(CFLAGS) -c transfer.c

clean:
	rm -f *.o inkmonitor-server
