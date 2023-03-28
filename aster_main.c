#include "aster.h"
#include <stdio.h>

int main(int argc, char **args)
{
    int i;
    aster_init();
    if(argc > 1) {
        for(i = 1; i < argc; i++)
            aster_runFile(args[i]);
    } else
        aster_runPrompt();
    return 0;
}
