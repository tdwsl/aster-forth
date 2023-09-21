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

200 constant bufsz
create buf bufsz allot

create q char " c,
create b char \ c,
create sq char \ c, char n c, char " c,

: write-quote ( a u -- )
  \ [char] " emit 2dup type [char] " emit cr
  q 1 fd-out write-file throw

  begin dup while
    over c@ dup [char] " = swap [char] \ = or if
      b 1 fd-out write-file throw
    then
    over 1 fd-out write-file throw
  1- >r 1+ r> repeat 2drop

  sq 3 fd-out write-line throw ;

: add-file ( a u -- )
  r/o open-file throw >r

  begin buf bufsz r@ read-line throw while
    buf swap write-quote
  repeat drop

  r> close-file throw ;

:noname
  1 begin dup argc @ <> while
    dup arg add-file 1+
  repeat drop
; execute

s" ;" fd-out write-line throw
s" " fd-out write-line throw
s" #endif" fd-out write-line throw
s" " fd-out write-line throw

fd-out close-file throw

bye

