CC=gcc
CFLAGS=-Wall -Wextra -O3

all: inkmonitor-server

inkmonitor-server: main.o options.o profiler.o screen.o transfer.o
	$(CC) $(CFLAGS) -o inkmonitor-server main.o options.o profiler.o screen.o transfer.o -lgiblib -lImlib2 -lX11

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
