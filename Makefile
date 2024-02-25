
CC=gcc
ARGS=-O2

aster: aster.c makeboot aster.f aster.h
	./makeboot
	$(CC) -c $(ARGS) aster.c -DASTER_TERMIOS
	$(CC) $(ARGS) aster.o main.c -o aster

makeboot: makeboot.c
	$(CC) $(ARGS) makeboot.c -o makeboot

install: aster
	gcc aster.c main.c -o /usr/bin/aster
	cp aster /usr/bin/aster
	ar ruv /usr/lib/libaster.a aster.o
	ranlib /usr/lib/libaster.a
	cp aster.h /usr/include
	cp aster_boot.h /usr/include

uninstall:
	rm -f /usr/bin/aster /usr/lib/libaster.a /usr/include/aster.h /usr/include/sater_boot.h

clean:
	rm -f aster aster.o libaster.a makeboot aster_boot.h
