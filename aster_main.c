#include "aster.h"

#ifdef ASTER_NCURSES

#include <ncurses.h>
#include <stdlib.h>

void emit(int c)
{
    addch(c);
    refresh();
}

int key()
{
    return getch();
}

void aster_w_page()
{
    clear();
}

void aster_w_atxy()
{
    aster_sassert(2);
    move(aster_stack[aster_sp-1], aster_stack[aster_sp-2]);
    aster_sp -= 2;
    refresh();
}

void aster_w_form()
{
    aster_sp += 2;
    getmaxyx(stdscr, aster_stack[aster_sp-2], aster_stack[aster_sp-1]);
}

void bye()
{
    addstr("\n[press any key]");
    getch();
    scrollok(stdscr, 0);
    echo();
    endwin();
    exit(0);
}

#endif

int main(int argc, char **args)
{
    int i;
    aster_initArgs(argc, args);
#ifdef ASTER_NCURSES
    initscr();
    noecho();
    scrollok(stdscr, 1);
    aster_addC(aster_w_atxy, "AT-XY", 0);
    aster_addC(aster_w_page, "PAGE", 0);
    aster_addC(aster_w_form, "FORM", 0);
    aster_emit = emit;
    aster_key = key;
    aster_bye = bye;
    aster_flags |= ASTER_F_ECHO;
#endif
    aster_init();
    if(argc > 1) {
        for(i = 1; i < argc; i++)
            aster_runFile(args[i]);
    } else
        aster_runPrompt();
#ifdef ASTER_NCURSES
    bye();
#endif
    return 0;
}
