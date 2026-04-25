CFLAGS= -Wall -Wextra -Werror

all: proxy daemon
	gcc bin/connect.o src/connect.h src/dolos-proxy.c ${CFLAGS} -o bin/dolos-proxy

proxy: bin/connect.o bin/dolos-proxy.o
	gcc -o bin/dolos-proxy bin/connect.o bin/dolos-proxy.o ${CFLAGS}

daemon: bin/dolosd.o
	gcc -o bin/dolosd bin/dolosd.o ${CFLAGS}

bin/dolosd.o: src/dolosd.c
	mkdir -p bin
	gcc -c src/dolosd.c ${CFLAGS} -o bin/dolosd.o

bin/dolos-proxy.o: src/dolos-proxy.c
	mkdir -p bin
	gcc -c src/dolos-proxy.c ${CFLAGS} -o bin/dolos-proxy.o

bin/connect.o: src/connect.c
	mkdir -p bin
	gcc -c src/connect.c ${CFLAGS} -o bin/connect.o

clean:
	rm bin/*

