all: inkmonitor-server

inkmonitor-server: main.o options.o screen.o transfer.o
	gcc -Wall -O3 -o inkmonitor-server main.o options.o screen.o transfer.o -lgiblib -lImlib2 -lX11

main.o: main.c
	gcc -Wall -O3 -c main.c

options.o: options.c
	gcc -Wall -O3 -c options.c

screen.o: screen.c
	gcc -Wall -O3 -c screen.c
	
transfer.o: transfer.c
	gcc -Wall -O3 -c transfer.c

clean:
	rm -f *.o inkmonitor-server
