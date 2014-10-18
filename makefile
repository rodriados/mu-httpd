CC = g++
CFLAGS = -Wall -pedantic -Ilib -O2

all: httpd

httpd: run.o httpd.cpp
	$(CC) $(CFLAGS) obj/run.o httpd.cpp -o httpd

run.o: src/run.cpp
	$(CC) $(CFLAGS) -c src/run.cpp -o obj/run.o

clean:
	rm -rf obj/* httpd
