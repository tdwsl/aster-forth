\ generate aster_boot.h

argc @ 1 = [if]
  ." usage: makeboot.f <file1,file2,...>" cr
  bye
[then]

s" aster_boot.h" w/o create-file throw constant fd-out

s" /* generated with makeboot.f */" fd-out write-line throw
s" " fd-out write-line
s" #ifndef ASTER_BOOT_H" fd-out write-line throw
s" #define ASTER_BOOT_H" fd-out write-line throw
s" " fd-out write-line throw
s" static char *aster_boot =" fd-out write-line throw

78 constant bufsz
create buf bufsz allot

-1 value first-(
-1 value first-\

variable col
1 col !
78 constant max-col

: charstr create c, does> 1 ;
char " charstr quo
32     charstr spc

quo fd-out write-file throw

: write-word ( a u -- )
  dup 1+ col +!
  col @ max-col >= if
    quo fd-out write-line throw
    quo fd-out write-file throw
    dup 1+ 1+ col !
  then
  fd-out write-file throw
  spc fd-out write-file throw ;

: parse-next ( f -- a u )
  >r 0 begin dup buf + 1 r@ read-file throw while
    dup buf + c@ ( u c -- )
    dup 32 <= if
      drop dup 0<> if r> drop buf swap exit then
    else
      dup [char] \ = over [char] " = or if
        over buf + [char] \ swap c! swap 1+ swap
      then
      over buf + c! 1+
      dup bufsz 1- >= if r> drop buf swap exit then
    then
  repeat
  r> drop buf swap ;

: add-word ( a u f -- )
  >r

  2dup s" (" str= if
    first-( if 0 to first-(
    else
      2drop begin r@ parse-next dup >r s" )" str= r> 0= or until
      r> drop exit
    then
  then

  2dup s" \\" str= if
    first-\ if 0 to first-\
    else
      2drop begin buf 1 r@ read-file throw 0= buf c@ 32 <= or until
      r> drop exit
    then
  then

  \ 2dup type space
  write-word r> drop ;

: add-file ( a u -- )
  r/o open-file throw >r

  begin r@ parse-next ?dup while
    r@ add-word
  repeat drop cr

  r> close-file throw ;

:noname
  1 begin dup argc @ <> while
    dup arg add-file 1+
  repeat drop
; execute

quo fd-out write-line throw
s" ;" fd-out write-line throw
s" " fd-out write-line throw
s" #endif" fd-out write-line throw
s" " fd-out write-line throw

fd-out close-file throw

bye

