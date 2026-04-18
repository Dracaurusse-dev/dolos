CFLAGS= -Wall -Wextra -Werror

all: bin/connect.o src/connect.h src/main.c
	gcc bin/connect.o src/connect.h src/main.c ${CFLAGS} -o bin/dolos

bin/connect.o: src/connect.c
	gcc -c src/connect.c ${CFLAGS} -o bin/connect.o

clean:
	rm bin/*

