CFLAGS= -Wall -Wextra -Werror
SERVICE_PATH_OPENRC = /etc/init.d/dolosd.d/

all: proxy daemon

valgrind: CFLAGS += -g
valgrind: all

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

# TODO: make install-runit install-openrc install-systemd 
install-openrc: all services/*
	cp bin/dolosd /usr/bin/dolosd
	cp bin/dolos-proxy /usr/bin/dolos-proxy
	mkdir -p ${SERVICE_PATH_OPENRC}
	cp services/dolosd-openrc.sh /etc/init.d/dolosd
	chown root:root /etc/init.d/dolosd
	chmod +x /etc/init.d/dolosd
	cp services/prestart.sh ${SERVICE_PATH_OPENRC}
	chown root:root ${SERVICE_PATH_OPENRC}prestart.sh
	chmod +x ${SERVICE_PATH_OPENRC}prestart.sh
	cp services/poststop.sh ${SERVICE_PATH_OPENRC}
	chown root:root ${SERVICE_PATH_OPENRC}poststop.sh
	chmod +x ${SERVICE_PATH_OPENRC}poststop.sh

clean:
	rm bin/*

