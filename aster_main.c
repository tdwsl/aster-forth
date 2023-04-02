#include "aster.h"

#ifdef ASTER_NCURSES

#include <ncurses.h>

void emit(int c)
{
    addch(c);
}

int key()
{
    return getch();
}

#else

#include <stdio.h>

void emit(int c)
{
    printf("%c", c);
}

int key()
{
    return fgetc(stdin);
}

#endif

int main(int argc, char **args)
{
    int i;
#ifdef ASTER_NCURSES
    initscr();
    noecho();
#endif
    aster_emit = emit;
    aster_key = key;
    aster_init();
    if(argc > 1) {
        for(i = 1; i < argc; i++)
            aster_runFile(args[i]);
    } else
        aster_runPrompt();
#ifdef ASTER_NCURSES
    echo();
    endwin();
#endif
    return 0;
}
