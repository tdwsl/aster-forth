gcc aster.c main.c -o /usr/bin/aster
gcc -c aster.c -o aster.o
ar ruv /usr/lib/libaster.a aster.o
ranlib /usr/lib/libaster.a
rm aster.o
cp aster.h /usr/include
cp aster_boot.h /usr/include
