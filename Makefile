CFLAGS= -Wall -Wextra -Werror

all: proxy daemon

proxy: bin/connect.o bin/dolos-proxy.o src/connect.h bin/stringutils.o
	gcc -o bin/dolos-proxy bin/connect.o bin/dolos-proxy.o src/connect.h bin/stringutils.o src/stringutils.h ${CFLAGS}

daemon: bin/dolosd.o bin/stringutils.o src/stringutils.h
	gcc -o bin/dolosd bin/dolosd.o bin/stringutils.o src/stringutils.h ${CFLAGS}

bin/dolosd.o: src/dolosd.c bin/stringutils.o
	mkdir -p bin
	gcc -c src/dolosd.c ${CFLAGS} -o bin/dolosd.o

bin/stringutils.o: src/stringutils.c
	mkdir -p bin
	gcc -c src/stringutils.c ${CFLAGS} -o bin/stringutils.o

bin/dolos-proxy.o: src/dolos-proxy.c
	mkdir -p bin
	gcc -c src/dolos-proxy.c ${CFLAGS} -o bin/dolos-proxy.o

bin/connect.o: src/connect.c
	mkdir -p bin
	gcc -c src/connect.c ${CFLAGS} -o bin/connect.o

clean:
	rm bin/*

