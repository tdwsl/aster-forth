gcc -c aster.c aster_dict.c
gcc *.o aster_main.c -o aster $1
ar ruv libaster.a *.o
ranlib libaster.a
