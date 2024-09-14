
: [then] ; immediate

: , here ! cell allot ;
: c, here c! 1 allot ;

: jmp, jmp , , ;
: jz, jz , , ;

: compile-only ; 0 ,

: [ compile-only 0 status ! ; immediate
: ] 1 status ! ;
: compile? status @ ;

: bstack 0 ;
here ' bstack cell + !
64 cell * allot
: bsp 0 ;
here ' bsp cell + !
cell allot
bstack bsp !
: >b bsp @ ! cell bsp +! ;
: b> [ cell invert 1+ lit , , ] bsp +! bsp @ @ ;

: ['] compile-only ' lit , , ; immediate

: begin compile-only here >b ; immediate
: again compile-only b> jmp, ; immediate
: until compile-only b> jz, ; immediate
: if compile-only 0 jz, here >b ; immediate
: then compile-only here b> cell - ! ; immediate
: else compile-only 0 jmp, here b> cell - ! here >b ; immediate
: compile, compile? if , else execute then ;
: lit, compile? if lit , , then ;
: postpone ' dup immediate? if compile, else lit, ['] compile, compile, then ;
  immediate
' if alias while immediate
: repeat compile-only b> postpone again >b postpone then ; immediate
: for compile-only postpone >r here >b ; immediate
: (next) r> r> 1- dup >r swap >r 0< ;
: rdrop r> r> drop >r ;
: next compile-only postpone (next) b> jz, postpone rdrop ; immediate

: literal lit, ; immediate
: 2literal compile? if swap then lit, lit, ; immediate

: ]l ] postpone literal ;

: exit compile-only ret , ; immediate
: recurse compile-only this , ; immediate

: hex 16 base ! ;
: decimal 10 base ! ;

: = - 0= ;
: <> - 0<> ;
: 0> 1- 0>= ;
: 0<= 1- 0< ;
: >= - 0>= ;
: <  - 0< ;
: >  - 1- 0>= ;
: <= - 1- 0< ;

: negate invert 1+ ;
: abs dup 0< if negate then ;

: / /mod nip ;
: mod /mod drop ;

: 2>r r> -rot swap >r >r >r ;
: 2r> r> r> r> swap rot >r ;

: ?dup dup if dup then ;

: 2drop drop drop ;
: 2dup over over ;
: 2over 3 pick 3 pick ;
: 2swap >r -rot r> -rot ;
: 2nip >r >r 2drop r> r> ;
: 2tuck 2swap 2over ;

: max 2dup > if drop else nip then ;
: min 2dup < if drop else nip then ;

: cell+ cell + ;
: cells cell * ;

: 2! tuck ! cell+ ! ;
: 2@ dup cell+ @ swap @ ;
: 2, , , ;

: ( begin parsec dup 41 = swap 10 <= or until ; immediate
: \ begin parsec 10 <= until ; immediate

: >body 4 cells + ;

: does> r> jmp last 2 cells + ! last 3 cells + ! ;

: create : 0 postpone literal here postpone ; cell dup allot - here swap ! ;
: variable create cell allot ;
: 2variable create cell cell+ allot ;
: constant : postpone literal ['] literal compile,
  postpone ; postpone immediate ;
: 2constant : postpone 2literal ['] 2literal compile,
  postpone ; postpone immediate ;
: value : postpone literal postpone ; ;
: to ' cell+ postpone literal postpone ! ; immediate

: defer create does> execute ;
: is ' cell+ postpone literal postpone ! ; immediate

: (is!) jmp over ! cell+ ! ;
: is! ' postpone literal postpone (is!) ; immediate

variable struct-sz
: begin-structure 0 struct-sz ! create 0 , last >body does> @ ;
: +field create struct-sz @ , struct-sz +! does> @ + ;
: field: cell +field ;
: 2field: 2 cells +field ;
: cfield: 1 +field ;
: end-structure struct-sz @ swap ! ;

-1 constant true
0 constant false

: fill ( a u c -- )
  >r begin dup while over r@ swap c! 1- >r 1+ r> repeat r> drop 2drop ;

: move ( a a u -- )
  >r 2dup < if
    begin r@ while r> 1- >r over r@ + c@ over r@ + c! repeat
  else
    begin r@ while over c@ over c! r> 1- >r
    1+ >r 1+ r> repeat
  then r> drop 2drop ;

: erase ( a u -- )
  >r begin r@ while 0 over c! 1+ r> 1- >r repeat r> 2drop ;

heap0 value heap
: hallot negate heap + to heap ;

: heap-save ( a u -- )
  dup hallot
  heap swap move ;

: parse-name ( -- a u )
  here begin
    parsec dup >r over c! 1+
  r> 32 <= until
  1- here tuck - ;

: parse-until ( c -- a u )
  >r here begin
    parsec dup >r over c! 1+
  r> dup r@ = swap 0= or until
  r> drop 1- here tuck - ;

: str, ( a u -- a u )
  dup >r heap-save heap r> ;

: word ( c -- a )
  >r here 1+ begin
  dup here 1+ = parsec dup r@ <> rot or over 10 > and while
    dup r@ <> if over c! 1+ else drop then
  repeat
  r> 2drop
  here - 1- here c!
  here ;

: type ( a u -- )
  begin dup 0> while over c@ emit >r 1+ r> 1- repeat 2drop ;

: char parse-name drop c@ ;
: [char] compile-only char postpone literal ; immediate

: ." [char] " parse-until compile? if str, postpone 2literal then
  postpone type ; immediate

: s" [char] " parse-until str, postpone 2literal ; immediate

: c" 1 allot [char] " parse-until -1 allot
  nip here 2dup c! swap 1+ str, drop postpone literal ; immediate

: count dup c@ >r 1+ r> ;

: cr 10 emit ;
: space 32 emit ;

: spaces begin dup 0> while 1- space repeat drop ;

create pic 48 allot
here constant picend
picend value picp

: hold picp 1- dup to picp c! ;
: holds begin dup while over c@ hold 1- >r 1+ r> repeat 2drop ;

: digit
  abs dup 10 >= if [ char a 10 - ]l
  else [char] 0 then + ;

: d>s dup 0< ;
: <# picend to picp ;
: # >r base @ /mod swap digit hold r> ;
: #> nip if [char] - hold then picp picend over - ;
: #s begin # over 0= until ;

: (.) d>s <# #s #> ;

: . (.) type space ;

: .r swap (.) rot over - spaces type ;

: .s ( -- )
  ." stack(" depth (.) type ." ): "
  depth begin dup while dup >r 1- pick . r> 1- repeat drop
  cr ;

: ? @ . ;

: marker here create , does> @ marker! ;

: strlen ( a -- a u )
  dup begin dup c@ while 1+ repeat over - ;

: arg ( u -- a u )
  dup 0< over argc @ >= or if drop 0 0 exit then
  access-args
  cells (args) + @ strlen ;

: do compile-only postpone 2>r
  0 >b 0 >b here >b -1 >b ; immediate

: ?do compile-only postpone 2dup postpone <= postpone if postpone 2drop
  0 jmp, here postpone then
  postpone 2>r
  >b 0 >b here >b -1 >b ; immediate

: unloop r> 2r> 2drop >r ;

: (end-loop)
  b> begin ?dup while
    here b> cell - !
  1- repeat
  postpone unloop
  b> ?dup if cell - here swap ! then ;

: (loop) r> 2r> 1+ 2dup 2>r <= swap >r ;

: loop compile-only b> drop
  postpone (loop) b> jz,
  (end-loop) ; immediate

: (+loop) r> swap 2r> ( n i2 i1 -- )
  2dup 4 pick + 2dup 2>r
  <= -rot <= <> nip
  swap >r ;

: +loop compile-only b> drop
  postpone (+loop) b> jz,
  (end-loop) ; immediate

: leave compile-only
  0 begin 1+ b> swap over -1 = until
  0 jmp, b> b> 1+ here >b >b >b
  begin swap >b 1- dup 0= until drop ; immediate

' r@ alias i

: j r> r> r> r@ -rot >r >r swap >r ;

:noname
  compile? 0= if
    ." word is compile only" cr -1 throw then ; is! compile-only

:noname
  compile-only
  bsp @ ['] bsp >= if
    ." branch stack overflow" cr bstack bsp ! -1 throw
  else bsp @ ! cell bsp +! then ; is! >b

:noname
  compile-only
  bsp @ bstack <= if
    ." branch stack underflow" cr bstack bsp ! -1 throw
  else [ cell negate ]l bsp +! bsp @ @ then ; is! b>

create cstack 64 cells allot
cstack 64 cells + constant cstacktop
cstack value csp

: >c compile-only csp
  dup cstacktop >= if ." compile stack overflow" cr
  cstack to csp -1 throw then
  ! csp cell+ to csp ;

: c> compile-only csp cell - dup
  dup cstack < if ." compile stack underflow" cr
  cstack to csp -1 throw then
  to csp @ ;

: case compile-only 0 >c ; immediate

: of compile-only
  postpone over postpone = r> postpone if >r postpone drop
  c> 1+ >c ; immediate

: endof compile-only r> postpone else >r ; immediate

: endcase compile-only postpone drop
  r> c> begin ?dup while 1- postpone then repeat >r ; immediate

: str= ( a u a u -- )
  rot over <> if 2drop drop 0 exit then
  0 ?do over i + c@ over i + c@ <> if 2drop unloop 0 exit then loop
  2drop -1 ;

: exists? 32 word find nip ;

exists? key 0= [if]

: key cin begin cin 10 = until ;

: accept ( a u -- u )
  dup >r 0 ?do
    cin dup 10 = over -1 = or if
      2drop i unloop r> drop exit
    then over i + c!
  loop
  drop r> ;

[else]

: accept ( a u -- u )
  >r 0 >r begin
    key dup 10 = over -1 = or if
      2drop r> r> drop exit
    then
    dup 127 = if drop i if 8 emit 32 emit 8 emit r> 1- >r then
    else r> r@ over >r < if
      dup emit over i + c! r> 1+ >r then then
  again ;

[then]

\ file handling words

: bin 3 + ;

' open-file alias create-file

: read-file ( a u f -- u ior )
  dup valid-file? 0= if 2drop drop 0 -1 exit then
  -rot dup >r 0 ?do
    over fgetc
    dup -1 = if
      2drop drop
      i unloop r> drop 0 exit
    then
    over i + c!
  loop 2drop r> 0 ;

: read-line ( a u f -- u flag ior )
  dup valid-file? 0= if 2drop drop 0 -1 exit then
  -rot dup >r 0 ?do
    over fgetc
    dup -1 = if
      2drop drop
      i unloop r> drop dup if -1 else 0 then 0 exit
    then
    dup 10 = if
      2drop drop
      i unloop r> drop -1 0 exit
    then
    over i + c!
  loop 2drop r> -1 0 ;

: write-file ( a u f -- ior )
  dup valid-file? 0= if 2drop drop -1 exit then
  -rot 0 ?do
    2dup i + c@ swap fputc
  loop 2drop 0 ;

here 10 c,

: write-line ( a u f -- ior )
  dup >r write-file
  ?dup 0= if literal 1 r> write-file else r> drop then ;

168 constant pad-size
create pad pad-size allot

\ redefine looping to allow multiple whiles

: begin compile-only here >c ; immediate
: again compile-only c> jmp, ; immediate
: until compile-only c> jz, ; immediate

: repeat compile-only c> jmp, here b> cell - ! ; immediate

\ ansi escape

exists? page 0= [if]

: esc[ 27 emit [char] [ emit ;
: page esc[ ." 2J" esc[ ." H" ;
: at-xy esc[ ." H" ?dup if esc[ (.) type ." B" then
  ?dup if esc[ (.) type ." C" then ;

[then]

