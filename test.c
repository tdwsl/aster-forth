#include "aster.h"

int main()
{
    aster_init();
    aster_doToken("10");
    aster_doToken("20");
    aster_doToken("+");
    aster_doToken(".");
    aster_doToken("CR");
    aster_doToken("11");
    aster_doToken("1");
    aster_doToken("DO");
    aster_doToken("7");
    aster_doToken("I");
    aster_doToken("*");
    aster_doToken(".");
    aster_doToken("LOOP");
    aster_doToken("CR");
    *(void**)(aster_dict+aster_here) = 0;
    aster_pc = aster_old;
    aster_print(aster_pc);
    aster_run();
    return 0;
}