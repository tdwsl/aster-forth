#include "aster.h"
#include "aster_boot.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **args) {
    int i;
    char *s;

    if(argc >= 2) {
        s = args[1];
        argc--;
        for(i = 1; i < argc; i++)
            args[i] = args[i+1];

        aster_init(argc, args);
        aster_runString(aster_boot);

        if(!aster_runFile(s)) { printf("failed to open %s\n", s); return 1; }
        for(i = 1; !aster_usedArgs && i < argc; i++)
            aster_runFile(args[i]);
    } else {
        aster_init(argc, args);
        aster_runString(aster_boot);
    }

    printf("Aster FORTH - tdwsl 2023. Type 'bye' to exit.\n");
    aster_runStdin();
    return 0;
}
