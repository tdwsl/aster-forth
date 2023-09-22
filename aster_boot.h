/* generated with makeboot.f */

#ifndef ASTER_BOOT_H
#define ASTER_BOOT_H

static char *aster_boot =
"\n"
": [then] ; immediate\n"
"\n"
": begin r> here >r >r ; immediate compile-only\n"
": again r> r> jmp, >r ; immediate compile-only\n"
": until r> r> jz, >r ; immediate compile-only\n"
": if r> 0 jz, here >r >r ; immediate compile-only\n"
": then r> here r> cell - ! >r ; immediate compile-only\n"
": else r> 0 jmp, postpone then here >r >r ; immediate compile-only\n"
": while r> postpone if >r ; immediate compile-only\n"
": repeat r> r> postpone again >r postpone then >r ; immediate compile-only\n"
"\n"
": ['] ' ; immediate compile-only\n"
"\n"
": hex 16 base ! ;\n"
": decimal 10 base ! ;\n"
"\n"
": 0<> 0= 0= ;\n"
": = - 0= ;\n"
": <> - 0= 0= ;\n"
"hex\n"
": 0>= 80000000 and 0= ;\n"
": 0<  80000000 and 0= 0= ;\n"
": 0>  1- 80000000 and 0= ;\n"
": 0<= 1- 80000000 and 0= 0= ;\n"
": >= - 80000000 and 0= ;\n"
": <  - 80000000 and 0= 0= ;\n"
": >  - 1- 80000000 and 0= ;\n"
": <= - 1- 80000000 and 0= 0= ;\n"
"decimal\n"
"\n"
": invert -1 xor ;\n"
": negate -1 xor 1+ ;\n"
"\n"
": / /mod nip ;\n"
": mod /mod drop ;\n"
"\n"
": +! dup >r @ + r> ! ;\n"
"\n"
": 2>r r> -rot swap >r >r >r ;\n"
": 2r> r> r> r> swap rot >r ;\n"
"\n"
": ?dup dup if dup then ;\n"
"\n"
": 2drop drop drop ;\n"
": 2dup over over ;\n"
": 2over 3 pick 3 pick ;\n"
": 2swap >r -rot r> -rot ;\n"
": 2nip >r >r 2drop r> r> ;\n"
": 2tuck 2swap 2over ;\n"
"\n"
": max 2dup > if drop else nip then ;\n"
": min 2dup < if drop else nip then ;\n"
"\n"
": [ 0 status ! ; immediate\n"
": ] 1 status ! ;\n"
": ]l ] postpone literal ;\n"
": compile? status @ ;\n"
"\n"
": , here ! cell allot ;\n"
": c, here c! 1 allot ;\n"
"\n"
": cell+ cell + ;\n"
": cells cell * ;\n"
"\n"
": does>\n"
"  postpone last postpone dup funsz cell+ 2* postpone literal postpone +\n"
"  postpone swap postpone lit!\n"
"  0 postpone literal here\n"
"  postpone last funsz cell+ postpone literal postpone +\n"
"  postpone jmp! postpone exit\n"
"  cell - here swap !\n"
"; immediate compile-only\n"
"\n"
": create : 0 postpone literal here postpone ; cell - here swap ! ;\n"
": constant : postpone literal postpone ; ;\n"
": variable create cell allot ;\n"
"\n"
"' constant alias value\n"
": to ' funsz + compile? if postpone literal postpone ! else ! then ; immediate\n"
"\n"
"create strbuf 24000 allot\n"
"variable strbufp\n"
"strbuf strbufp !\n"
"\n"
": ( begin parsec dup 41 = swap 0= or until ; immediate\n"
": \\ begin parsec 0= until ; immediate\n"
"\n"
": parse-name ( -- a u )\n"
"  strbufp @ begin\n"
"    parsec dup >r over c! 1+\n"
"  r> 32 <= until\n"
"  1- strbufp @ tuck - ;\n"
"\n"
": save-mem ( u -- )\n"
"  strbufp +! ;\n"
"\n"
": parse-until ( c -- a u )\n"
"  >r strbufp @ begin\n"
"    parsec dup >r over c! 1+\n"
"  r> dup r@ = swap 0= or until\n"
"  r> drop 1- strbufp @ tuck - ;\n"
"\n"
": type ( a u -- )\n"
"  begin dup 0> while over c@ emit >r 1+ r> 1- repeat 2drop ;\n"
"\n"
": char parse-name drop c@ ;\n"
": [char] char postpone literal ; immediate compile-only\n"
"\n"
": 2literal swap postpone literal postpone literal ; immediate compile-only\n"
"\n"
": .\" [char] \" parse-until\n"
"  compile? if dup save-mem postpone 2literal postpone type\n"
"  else type then ; immediate\n"
"\n"
": s\" [char] \" parse-until dup save-mem\n"
"  compile? if postpone 2literal then ; immediate\n"
"\n"
": cr 10 emit ;\n"
": space 32 emit ;\n"
"\n"
": spaces begin dup 0> while 1- space repeat drop ;\n"
"\n"
"create nbuf 160 allot\n"
"\n"
": .digit ( u -- )\n"
"  dup 10 < if [char] 0\n"
"  else [ char a 10 - ]l\n"
"  then + emit ;\n"
"\n"
": >nbuf ( n -- u )\n"
"  dup 0< if [char] - emit negate then\n"
"  nbuf >r\n"
"  begin\n"
"    base @ /mod\n"
"    swap r@ c! r> 1+ >r\n"
"  dup 0= until drop\n"
"  r> nbuf - ;\n"
"\n"
": .nbuf ( u -- )\n"
"  nbuf + begin 1- dup c@ .digit dup nbuf = until drop ;\n"
"\n"
": (.) >nbuf .nbuf ;\n"
"\n"
": . (.) space ;\n"
"\n"
": .r swap >nbuf swap over - spaces .nbuf ;\n"
"\n"
": .s ( -- )\n"
"  .\" stack(\" depth (.) .\" ): \"\n"
"  depth begin dup while dup >r 1- pick . r> 1- repeat drop\n"
"  cr ;\n"
"\n"
": strlen ( a -- a u )\n"
"  dup begin dup c@ while 1+ repeat over - ;\n"
"\n"
": arg ( u -- a u )\n"
"  dup 0< over argc @ >= or if drop 0 0 exit then\n"
"  cells (args) + @ strlen ;\n"
"\n"
": do postpone >r 0 postpone literal postpone !\n"
"  r> 0 >r 0 >r here >r -1 >r >r ; immediate compile-only\n"
"\n"
": ?do postpone 2dup postpone <= postpone if postpone 2drop\n"
"  0 jmp, here postpone then\n"
"  postpone >r 0 postpone literal postpone !\n"
"  r> swap >r 0 >r here >r -1 >r >r ; immediate compile-only\n"
"\n"
": unloop r> r> drop >r ; compile-only\n"
"\n"
": (end-loop) r>\n"
"  r> begin ?dup while\n"
"    here r> cell - !\n"
"  1- repeat\n"
"  postpone unloop\n"
"  r> ?dup if cell - here swap ! then >r ;\n"
"\n"
": loop r> r> drop\n"
"  postpone r> postpone 1+ postpone dup postpone >r\n"
"  0 postpone literal  here cell -  r@ funsz - cell - !\n"
"  postpone >=  r> jz,\n"
"  (end-loop) >r ; immediate compile-only\n"
"\n"
": +loop r> r> drop\n"
"  postpone r> postpone dup postpone -rot postpone + postpone dup postpone >r\n"
"  0 postpone literal  here cell -  r@ funsz - cell - !\n"
"  postpone over postpone < postpone -rot\n"
"  postpone >= postpone <>\n"
"  r> jz,\n"
"  (end-loop) >r ; immediate compile-only\n"
"\n"
": leave r>\n"
"  0 begin 1+ r> swap over -1 = until\n"
"  0 jmp, r> r> 1+ here >r >r >r\n"
"  begin swap >r 1- dup 0= until drop >r ; immediate compile-only\n"
"\n"
"' r@ alias i compile-only\n"
"\n"
": j r> r> r@ swap >r >r ; compile-only\n"
"\n"
": key cin begin cin 10 = until ;\n"
"\n"
": accept ( a u -- u )\n"
"  dup >r 0 ?do\n"
"    cin dup 10 = over -1 = or if\n"
"      2drop i unloop r> drop exit\n"
"    then over i + c!\n"
"  loop\n"
"  drop r> ;\n"
"\n"
": throw if .\" throw\" cr -1 error then ;\n"
"\n"
": bin 3 + ;\n"
"\n"
"' open-file alias create-file\n"
"\n"
": read-file ( a u f -- u ior )\n"
"  dup valid-file? 0= if 2drop drop 0 -1 exit then\n"
"  -rot dup >r 0 ?do\n"
"    over fgetc\n"
"    dup -1 = if\n"
"      2drop drop\n"
"      i unloop r> drop 0 exit\n"
"    then\n"
"    over i + c!\n"
"  loop 2drop r> 0 ;\n"
"\n"
": read-line ( a u f -- u flag ior )\n"
"  dup valid-file? 0= if 2drop drop 0 -1 exit then\n"
"  -rot dup >r 0 ?do\n"
"    over fgetc\n"
"    dup -1 = if\n"
"      2drop drop\n"
"      i unloop r> drop dup if -1 else 0 then 0 exit\n"
"    then\n"
"    dup 10 = if\n"
"      2drop drop\n"
"      i unloop r> drop -1 0 exit\n"
"    then\n"
"    over i + c!\n"
"  loop 2drop r> -1 0 ;\n"
"\n"
": write-file ( a u f -- ior )\n"
"  dup valid-file? 0= if 2drop drop -1 exit then\n"
"  -rot 0 ?do\n"
"    2dup i + c@ swap fputc\n"
"  loop 2drop 0 ;\n"
"\n"
"here 10 c,\n"
"\n"
": write-line ( a u f -- ior )\n"
"  dup >r write-file\n"
"  ?dup 0= if literal 1 r> write-file else r> drop then ;\n"
"\n"
;

#endif

