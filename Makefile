
CC=gcc
# for windows may want to change to aster.exe
OUT=aster

# add -DASTER_TERMIOS/-DASTER_CONIO/-DASTER_WIN32 for console functionality
# (no support for win32 page/at-xy yet)
ARGS=-O2 -DASTER_TERMIOS

aster: aster.c makeboot aster.f aster.h
	./makeboot
	$(CC) -c $(ARGS) aster.c
	$(CC) $(ARGS) aster.o main.c -o $(OUT)

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
