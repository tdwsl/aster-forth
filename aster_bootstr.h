#ifndef ASTER_BOOTSTR_H
#define ASTER_BOOTSTR_H

const char *aster_bootstr = ""
": ['] ' ; immediate "
": [char] char postpone literal ; immediate "
": cr 10 emit ; "
": space 32 emit ; "
": cells cell * ; "
": cell+ cell + ; "
": type 0 do dup c@ emit 1+ loop drop ; "
": s\" [char] \" parse dup save-string "
"  swap postpone literal postpone literal ; immediate "
": .\" [char] \" parse dup save-string "
"  swap postpone literal postpone literal postpone type ; immediate "
": .( [char] ) parse dup save-string "
"  swap postpone literal postpone literal postpone type ; immediate "
"";

#endif
