
: [then] ; immediate

: begin r> here >r >r ; immediate compile-only
: again r> r> jmp, >r ; immediate compile-only
: until r> r> jz, >r ; immediate compile-only
: if r> 0 jz, here >r >r ; immediate compile-only
: then r> here r> cell - ! >r ; immediate compile-only
: else r> 0 jmp, postpone then here >r >r ; immediate compile-only
: while r> postpone if >r ; immediate compile-only
: repeat r> r> postpone again >r postpone then >r ; immediate compile-only

: ['] ' postpone literal ; immediate compile-only

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

: invert -1 xor ;
: negate -1 xor 1+ ;

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

: [ 0 status ! ; immediate
: ] 1 status ! ;
: ]l ] postpone literal ;
: compile? status @ ;

: , here ! cell allot ;
: c, here c! 1 allot ;

: cell+ cell + ;
: cells cell * ;

: 2! tuck ! cell+ ! ;
: 2@ dup cell+ @ swap @ ;

: ( begin parsec dup 41 = swap 0= or until ; immediate
: \ begin parsec 0= until ; immediate

: (does) ( u -- )
  last dup funsz cell+ 2* + swap lit!
  last funsz cell+ + jmp! r> drop ;

: does>
  here funsz cell+ 2* + postpone literal postpone (does) ;
immediate compile-only

: create : 0 postpone literal here postpone ; cell dup allot - here swap ! ;
: constant : postpone literal postpone ; ;
: variable create cell allot ;

' constant alias value
: to ' funsz + postpone literal postpone ! ; immediate

create strbuf 24000 allot
variable strbufp
strbuf strbufp !

: parse-name ( -- a u )
  strbufp @ begin
    parsec dup >r over c! 1+
  r> 32 <= until
  1- strbufp @ tuck - ;

: save-mem ( u -- )
  strbufp +! ;

: parse-until ( c -- a u )
  >r strbufp @ begin
    parsec dup >r over c! 1+
  r> dup r@ = swap 0= or until
  r> drop 1- strbufp @ tuck - ;

: word ( c -- a )
  >r here 1+ begin parsec dup r@ <> over 0<> and while
    over c! 1+
  repeat
  r> 2drop
  here - 1- here c!
  here ;

: type ( a u -- )
  begin dup 0> while over c@ emit >r 1+ r> 1- repeat 2drop ;

: char parse-name drop c@ ;
: [char] char postpone literal ; immediate compile-only

: 2literal compile? if swap then
  postpone literal postpone literal ; immediate compile-only

: ." [char] " parse-until
  compile? if dup save-mem then postpone 2literal postpone type ; immediate

: s" [char] " parse-until dup save-mem postpone 2literal ; immediate

: c" 1 strbufp +!
  [char] " parse-until dup save-mem over 1- c! 1- postpone literal ; immediate

: count dup c@ >r 1+ r> ;

: cr 10 emit ;
: space 32 emit ;

: spaces begin dup 0> while 1- space repeat drop ;

create nbuf 160 allot

: .digit ( u -- )
  dup 10 < if [char] 0
  else [ char a 10 - ]l
  then + emit ;

: >nbuf ( n -- u )
  dup 0< if [char] - emit negate then
  nbuf >r
  begin
    base @ /mod
    swap r@ c! r> 1+ >r
  dup 0= until drop
  r> nbuf - ;

: .nbuf ( u -- )
  nbuf + begin 1- dup c@ .digit dup nbuf = until drop ;

: (.) >nbuf .nbuf ;

: . (.) space ;

: .r swap >nbuf swap over - spaces .nbuf ;

: .s ( -- )
  ." stack(" depth (.) ." ): "
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

: do postpone 2>r
  r> 0 >r 0 >r here >r -1 >r >r ; immediate compile-only

: ?do postpone 2dup postpone <= postpone if postpone 2drop
  0 jmp, here postpone then
  postpone 2>r
  r> swap >r 0 >r here >r -1 >r >r ; immediate compile-only

: unloop r> 2r> 2drop >r ; compile-only

: (end-loop) r>
  r> begin ?dup while
    here r> cell - !
  1- repeat
  postpone unloop
  r> ?dup if cell - here swap ! then >r ;

: loop r> r> drop
  postpone 2r> postpone 1+ postpone 2dup postpone 2>r
  postpone <=  r> jz,
  (end-loop) >r ; immediate compile-only

: +loop r> r> drop
  postpone 2r> ( n i2 i1 -- )
  postpone 2dup 4 postpone literal postpone pick
  postpone + postpone 2dup postpone 2>r
  postpone <= postpone -rot postpone <= postpone <> postpone nip
  r> jz,
  (end-loop) >r ; immediate compile-only

: leave r>
  0 begin 1+ r> swap over -1 = until
  0 jmp, r> r> 1+ here >r >r >r
  begin swap >r 1- dup 0= until drop >r ; immediate compile-only

' r@ alias i compile-only

: j r> r> r> r@ -rot >r >r swap >r ; compile-only

: fill ( a u c -- )
  >r begin dup while over r@ swap c! 1- >r 1+ r> repeat r> drop 2drop ;

: move ( a a u -- )
  >r begin r@ while over c@ over c! r> 1- >r
  1+ >r 1+ r> repeat r> drop 2drop ;

: erase ( a u -- )
  >r begin r@ while 0 over c! 1+ r> 1- >r repeat r> 2drop ;

: str= ( a u a u -- )
  rot over <> if 2drop drop 0 exit then
  0 ?do over i + c@ over i + c@ <> if 2drop unloop 0 exit then loop
  2drop -1 ;

: key cin begin cin 10 = until ;

: accept ( a u -- u )
  dup >r 0 ?do
    cin dup 10 = over -1 = or if
      2drop i unloop r> drop exit
    then over i + c!
  loop
  drop r> ;

: throw if ." throw" cr -1 error then ;

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

