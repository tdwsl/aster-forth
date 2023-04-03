#include "aster.h"

#ifdef ASTER_NCURSES

#include <ncurses.h>

void emit(int c)
{
    if((c&0xff) == c)
        if(c >= ' ' || c == '\t' || c == '\n' || c == '\b') {
            addch(c);
            refresh();
        }
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
    aster_initArgs(argc, args);
#ifdef ASTER_NCURSES
    initscr();
    noecho();
    scrollok(stdscr, 1);
    aster_addC(aster_w_atxy, "AT-XY", 0);
    aster_addC(aster_w_page, "PAGE", 0);
    aster_addC(aster_w_form, "FORM", 0);
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
    addstr("press any key");
    getch();
    scrollok(stdscr, 0);
    echo();
    endwin();
#endif
    return 0;
}
