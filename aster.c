/* aster forth - tdwsl 2023 */

#include "aster.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct aster_word {
    char *s;
    char flags;
    int a;
    void (*f)(void);
    int end;
};

unsigned char aster_dict[ASTER_DICTSZ];
int aster_stack[256];
int aster_rstack[256];
int aster_backtrace[256];
unsigned char aster_sp=0, aster_rsp=0, aster_btsp=0;
int aster_here = 0;
int aster_pc;
char aster_buf[ASTER_BUFSZ];
struct aster_word aster_words[ASTER_MAXWORDS];
int aster_nwords = 0;
FILE *aster_fp;
char aster_nameBuf[ASTER_NAMEBUFSZ];
char *aster_nameBufP = aster_nameBuf;
char *aster_string;
const char *aster_fileModes[] = { "r", "w", "rw", "rb", "wb", "rwb", };
FILE *aster_files[ASTER_NFILES];
int aster_filen = 0;
unsigned char aster_error = 0;
unsigned char aster_waitThen = 0;
unsigned char aster_trace = 0;

const char *aster_sSU = "stack underflow !\n";
const char *aster_sSO = "stack overflow !\n";
const char *aster_sRU = "return stack underflow !\n";
const char *aster_sRO = "return stack overflow !\n";
const char *aster_sOB = "invalid memory address\n";

void aster_getNext(char *buf, int max) {
    int i;

    for(i = 0;;) {
        buf[i] = *aster_string;
        if(*aster_string) aster_string++;
        if(buf[i] == 0) return;
        if(buf[i] <= 32) {
            if(i) { buf[i] = 0; return; }
        } else if(++i >= max-1) { buf[i] = 0; return; }
    }
}

int aster_streq(char *s1, char *s2) {
    char c1, c2;

    do {
        c1 = *s1;
        c2 = *s2;
        if(c1 >= 'a' && c1 <= 'z') c1 += 'A'-'a';
        if(c2 >= 'a' && c2 <= 'z') c2 += 'A'-'a';
        if(c1 != c2) return 0;
        s1++; s2++;
    } while(c1);

    return 1;
}

void aster_printIns(int addr);

void aster_printAddr(int addr);

void aster_runAddr(int pc) {
    void (*fun)(void);

    aster_pc = pc;
    aster_rstack[aster_rsp++] = 0;

    if(aster_trace)
        aster_printAddr(aster_pc);

    do {
        if(aster_trace)
            aster_printIns(aster_pc);

        fun = *(void (**)(void))&aster_dict[aster_pc];
        aster_pc += ASTER_FUNSZ;
        fun();
    } while((aster_pc != 0) & (!aster_error));
}

struct aster_word *aster_findWord(char *s) {
    int i;

    for(i = aster_nwords-1; i >= 0; i--)
        if(aster_streq(s, aster_words[i].s))
            return &aster_words[i];

    return 0;
}

/*** DICTIONARY ***/

void aster_f_lit() {
    aster_stack[aster_sp++] = *(int*)&aster_dict[aster_pc];
    aster_pc += ASTER_INTSZ;
    aster_soassert(1);
}

void aster_f_jmp() {
    aster_pc = *(int*)&aster_dict[aster_pc];
}

void aster_f_jz() {
    if(aster_stack[--aster_sp]) aster_pc += ASTER_INTSZ;
    else aster_pc = *(int*)&aster_dict[aster_pc];
}

void aster_f_call() {
    aster_rstack[aster_rsp++] = aster_pc + ASTER_INTSZ;
    aster_pc = *(int*)&aster_dict[aster_pc];
    aster_backtrace[aster_btsp++] = aster_pc;

    if(aster_trace) {
        printf("call ");
        aster_printAddr(aster_pc);
    }
}

void aster_f_ret() {
    aster_pc = aster_rstack[--aster_rsp];
    aster_btsp--;

    if(aster_trace) {
        printf("ret ");
        aster_printAddr(aster_backtrace[aster_btsp]);
    }
}

void aster_f_rph() {
    aster_sassert(1);
    aster_rstack[aster_rsp++] = aster_stack[--aster_sp];
    aster_roassert(1);
}

void aster_f_rpl() {
    aster_rassert(1);
    aster_stack[aster_sp++] = aster_rstack[--aster_rsp];
    aster_soassert(1);
}

void aster_f_rat() {
    aster_rassert(1);
    aster_stack[aster_sp++] = aster_rstack[aster_rsp-1];
    aster_soassert(1);
}

void aster_f_add() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] += aster_stack[aster_sp];
}

void aster_f_sub() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] -= aster_stack[aster_sp];
}

void aster_f_and() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] &= aster_stack[aster_sp];
}

void aster_f_or() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] |= aster_stack[aster_sp];
}

void aster_f_xor() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] ^= aster_stack[aster_sp];
}

void aster_f_mul() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] *= aster_stack[aster_sp];
}

void aster_f_div() {
    int t;
    aster_sassert(2);
    t = aster_stack[aster_sp-2] / aster_stack[aster_sp-1];
    aster_stack[aster_sp-2] %= aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = t;
}

void aster_f_inc() {
    aster_sassert(1);
    aster_stack[aster_sp-1]++;
}

void aster_f_dec() {
    aster_sassert(1);
    aster_stack[aster_sp-1]--;
}

void aster_f_shr() {
    aster_sassert(1);
    aster_stack[aster_sp-1] >>= 1;
}

void aster_f_shl() {
    aster_sassert(1);
    aster_stack[aster_sp-1] <<= 1;
}

void aster_f_zeq() {
    aster_sassert(1);
    aster_stack[aster_sp-1] = aster_stack[aster_sp-1] ? 0 : -1;
}

void aster_f_inv() {
    aster_sassert(1);
    aster_stack[aster_sp-1] = ~aster_stack[aster_sp-1];
}

void aster_f_swap() {
    int t;
    aster_sassert(2);
    t = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = t;
}

void aster_f_dup() {
    aster_sassert(1);
    aster_stack[aster_sp] = aster_stack[aster_sp-1];
    aster_sp++;
    aster_soassert(1);
}

void aster_f_over() {
    aster_sassert(2);
    aster_stack[aster_sp] = aster_stack[aster_sp-2];
    aster_sp++;
    aster_soassert(1);
}

void aster_f_nip() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] = aster_stack[aster_sp];
}

void aster_f_tuck() {
    aster_sassert(2);
    aster_stack[aster_sp] = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp];
    aster_sp++;
    aster_soassert(1);
}

void aster_f_drop() {
    aster_sassert(1);
    aster_sp--;
}

void aster_f_rot() {
    int t;
    aster_sassert(3);
    t = aster_stack[aster_sp-3];
    aster_stack[aster_sp-3] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = t;
    aster_soassert(1);
}

void aster_f_mrot() {
    int t;
    aster_sassert(3);
    t = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-3];
    aster_stack[aster_sp-3] = t;
    aster_soassert(1);
}

void aster_f_depth() {
    aster_stack[aster_sp] = aster_sp;
    aster_sp++;
    aster_soassert(1);
}

void aster_f_pick() {
    aster_sassert(aster_stack[aster_sp-1]+1);
    aster_stack[aster_sp-1] =
      aster_stack[aster_sp-2-aster_stack[aster_sp-1]];
    aster_soassert(1);
}

void aster_f_sw() {
    aster_sassert(2);
    aster_bassert(aster_stack[aster_sp-1]);
    *(int*)&aster_dict[aster_stack[aster_sp-1]] = aster_stack[aster_sp-2];
    aster_sp -= 2;
}

void aster_f_lw() {
    aster_sassert(1);
    aster_bassert(aster_stack[aster_sp-1]);
    aster_stack[aster_sp-1] = *(int*)&aster_dict[aster_stack[aster_sp-1]];
}

void aster_f_sb() {
    aster_sassert(2);
    aster_bassert(aster_stack[aster_sp-1]);
    aster_dict[aster_stack[aster_sp-1]] = aster_stack[aster_sp-2];
    aster_sp -= 2;
}

void aster_f_lb() {
    aster_sassert(1);
    aster_bassert(aster_stack[aster_sp-1]);
    aster_stack[aster_sp-1] = aster_dict[aster_stack[aster_sp-1]];
}

void aster_f_here() {
    aster_stack[aster_sp++] = aster_here;
    aster_soassert(1);
}

void aster_f_allot() {
    aster_sassert(1);
    aster_here += aster_stack[--aster_sp];
}

void aster_f_execute() {
    aster_sassert(1);
    aster_sp--;
    if(aster_stack[aster_sp] < 0) aster_words[~aster_stack[aster_sp]].f();
    else aster_runAddr(aster_stack[aster_sp]);
}

struct aster_word *aster_getNextWord() {
    struct aster_word *w;

    aster_getNext(aster_buf, ASTER_BUFSZ);
    w = aster_findWord(aster_buf);
    if(!w) { printf("%s ?\n", aster_buf); aster_error = 1; }
    return w;
}

void aster_f_tick() {
    struct aster_word *w;

    w = aster_getNextWord();
    if(!w) return;
    if(w->flags & ASTER_FUNCTION) aster_stack[aster_sp] = ~(w-aster_words);
    else aster_stack[aster_sp] = w->a;
    aster_sp++;
    aster_soassert(1);
}

void aster_f_compile() {
    if(*(int*)&aster_dict[ASTER_STATUS]) {
        aster_sassert(1);
        aster_sp--;
        if(aster_stack[aster_sp] < 0) {
            *(void (**)(void))&aster_dict[aster_here] =
              aster_words[~aster_stack[aster_sp]].f;
            aster_here += ASTER_FUNSZ;
        } else {
            *(void (**)(void))&aster_dict[aster_here] = aster_f_call;
            aster_here += ASTER_FUNSZ;
            *(int*)&aster_dict[aster_here] = aster_stack[aster_sp];
            aster_here += ASTER_INTSZ;
        }
    } else aster_f_execute();
}

void aster_f_postpone() {
    struct aster_word *w;

    w = aster_getNextWord();
    if(!w) return;
    if(w->flags & ASTER_IMMEDIATE) {
        if(w->flags & ASTER_FUNCTION) {
            *(void (**)(void))&aster_dict[aster_here] = w->f;
            aster_here += ASTER_FUNSZ;
        } else {
            *(void (**)(void))&aster_dict[aster_here] = aster_f_call;
            aster_here += ASTER_FUNSZ;
            *(int*)&aster_dict[aster_here] = w->a;
            aster_here += ASTER_INTSZ;
        }
    } else {
        *(void (**)(void))&aster_dict[aster_here] = aster_f_lit;
        aster_here += ASTER_FUNSZ;
        if(w->flags & ASTER_FUNCTION)
            *(int*)&aster_dict[aster_here] = ~(w-aster_words);
        else
            *(int*)&aster_dict[aster_here] = w->a;
        aster_here += ASTER_INTSZ;
        *(void (**)(void))&aster_dict[aster_here] = aster_f_compile;
        aster_here += ASTER_FUNSZ;
    }
}

struct aster_word *aster_findWordAddr(int a) {
    int i;

    for(i = aster_nwords-1; i >= 0; i--)
        if(!(aster_words[i].flags & ASTER_FUNCTION) && aster_words[i].a == a)
            return &aster_words[i];
    return 0;
}

void aster_f_alias() {
    char buf[ASTER_BUFSZ];
    struct aster_word *w;

    aster_sassert(1);
    aster_getNext(aster_nameBufP,
      ASTER_NAMEBUFSZ-(int)(aster_nameBufP-aster_nameBuf)-1);
    aster_words[aster_nwords].s = aster_nameBufP;
    aster_nameBufP += strlen(aster_nameBufP)+1;
    aster_sp--;
    if(aster_stack[aster_sp] < 0) {
        aster_words[aster_nwords].f = aster_words[~aster_stack[aster_sp]].f;
        aster_words[aster_nwords].flags = ASTER_FUNCTION;
    } else {
        aster_words[aster_nwords].a = aster_stack[aster_sp];
        aster_words[aster_nwords].flags = 0;
        if(w = aster_findWordAddr(aster_stack[aster_sp]))
            aster_words[aster_nwords].end = w->end;
    }
    aster_nwords++;
}

void aster_f_literal() {
    aster_sassert(1);
    if(*(int*)&aster_dict[ASTER_STATUS]) {
        *(void (**)(void))&aster_dict[aster_here] = aster_f_lit;
        aster_here += ASTER_FUNSZ;
        *(int*)&aster_dict[aster_here] = aster_stack[--aster_sp];
        aster_here += ASTER_INTSZ;
    }
}

void aster_f_jmpc() {
    aster_sassert(1);
    *(void (**)(void))&aster_dict[aster_here] = aster_f_jmp;
    aster_here += ASTER_FUNSZ;
    *(int*)&aster_dict[aster_here] = aster_stack[--aster_sp];
    aster_here += ASTER_INTSZ;
}

void aster_f_jzc() {
    aster_sassert(1);
    *(void (**)(void))&aster_dict[aster_here] = aster_f_jz;
    aster_here += ASTER_FUNSZ;
    *(int*)&aster_dict[aster_here] = aster_stack[--aster_sp];
    aster_here += ASTER_INTSZ;
}

void aster_f_jmps() {
    aster_sassert(2);
    *(void (**)(void))&aster_dict[aster_stack[aster_sp-1]] = aster_f_jmp;
    *(int*)&aster_dict[aster_stack[aster_sp-1]+ASTER_FUNSZ] =
      aster_stack[aster_sp-2];
    aster_sp -= 2;
}

void aster_f_lits() {
    aster_sassert(2);
    *(void (**)(void))&aster_dict[aster_stack[aster_sp-1]] = aster_f_lit;
    *(int*)&aster_dict[aster_stack[aster_sp-1]+ASTER_FUNSZ] =
      aster_stack[aster_sp-2];
    aster_sp -= 2;
}

void aster_f_recurse() {
    *(void (**)(void))&aster_dict[aster_here] = aster_f_call;
    aster_here += ASTER_FUNSZ;
    *(int*)&aster_dict[aster_here] = aster_words[aster_nwords].a;
    aster_here += ASTER_INTSZ;
}

void aster_f_exit() {
    *(void (**)(void))&aster_dict[aster_here] = aster_f_ret;
    aster_here += ASTER_FUNSZ;
}

void aster_f_parsec() {
    aster_stack[aster_sp++] = *aster_string;
    if(*aster_string) aster_string++;
    aster_soassert(1);
}

void aster_f_emit() {
    aster_sassert(1);
    fputc(aster_stack[--aster_sp], stdout);
}

void aster_f_cin() {
    aster_stack[aster_sp++] = fgetc(stdin);
    aster_soassert(1);
}

void aster_f_colon() {
    *(int*)&aster_dict[ASTER_STATUS] = 1;
    aster_getNext(aster_buf, ASTER_BUFSZ);

    aster_words[aster_nwords].s     = aster_nameBufP;
    aster_words[aster_nwords].flags = 0;
    aster_words[aster_nwords].a     = aster_here;

    strcpy(aster_nameBufP, aster_buf);
    aster_nameBufP += strlen(aster_nameBufP)+1;
}

void aster_f_noname() {
    *(int*)&aster_dict[ASTER_STATUS] = 1;
    aster_words[aster_nwords].s     = 0;
    aster_words[aster_nwords].flags = 0;
    aster_words[aster_nwords].a     = aster_here;
}

void aster_f_semi() {
    *(int*)&aster_dict[ASTER_STATUS] = 0;
    *(void (**)(void))&aster_dict[aster_here] = aster_f_ret;
    aster_here += ASTER_FUNSZ;
    if(aster_words[aster_nwords].s)
        aster_words[aster_nwords++].end = aster_here;
    else {
        aster_stack[aster_sp++] = aster_words[aster_nwords].a;
        aster_soassert(1);
    }
}

void aster_f_last() {
    aster_stack[aster_sp++] = aster_words[aster_nwords-1].a;
    aster_soassert(1);
}

void aster_f_immediate() {
    aster_words[aster_nwords-1].flags |= ASTER_IMMEDIATE;
}

void aster_f_compileonly() {
    aster_words[aster_nwords-1].flags |= ASTER_COMPILEONLY;
}

void aster_printAddr(int addr) {
    int i;
    for(i = aster_nwords-1; i >= 0; i--)
        if(!(aster_words[i].flags & ASTER_FUNCTION)
          && aster_words[i].a == addr) {
            printf("%s\n", aster_words[i].s);
            return;
        }
    printf("$%.8X\n", addr);
}

void aster_printFunction(void (*fun)(void)) {
    int i;
    for(i = aster_nwords-1; i >= 0; i--)
        if((aster_words[i].flags & ASTER_FUNCTION)
          && aster_words[i].f == fun) {
            printf("%s\n", aster_words[i].s);
            return;
        }
    printf("function $%X\n", fun);
}

void aster_printIns(int addr) {
    void (*fun)(void);

    printf("%.8X ", addr);
    fun = *(void (**)(void))&aster_dict[addr];

    if(fun == aster_f_lit)
        printf("lit $%X\n", *(int*)&aster_dict[addr+ASTER_FUNSZ]);
    else if(fun == aster_f_call)
        aster_printAddr(*(int*)&aster_dict[addr+ASTER_FUNSZ]);
    else if(fun == aster_f_jmp)
        printf("jmp %.8X\n", *(int*)&aster_dict[addr+ASTER_FUNSZ]);
    else if(fun == aster_f_jz)
        printf("jz %.8X\n", *(int*)&aster_dict[addr+ASTER_FUNSZ]);
    else if(fun == aster_f_ret)
        printf("ret\n");
    else
        aster_printFunction(*(void (**)(void))&aster_dict[addr]);
}

void aster_f_see() {
    struct aster_word *w;
    int i;
    void (*fun)(void);

    w = aster_getNextWord();
    if(!w) return;
    printf("%s", w->s);
    if(w->flags & ASTER_IMMEDIATE) printf(" (immediate)");
    if(w->flags & ASTER_COMPILEONLY) printf(" (compile-only)");
    printf("\n");

    if(w->flags & ASTER_FUNCTION) {
        printf("function $%.X\n", w->f);
        return;
    }

    for(i = w->a; i < w->end; i += ASTER_FUNSZ) {
        aster_printIns(i);
        fun = *(void (**)(void))&aster_dict[i];
        if(fun == aster_f_jmp || fun == aster_f_jz
          || fun == aster_f_call || fun == aster_f_lit)
            i += ASTER_INTSZ;
    }
}

void aster_f_words() {
    int i, x;

    x = 0;
    for(i = aster_nwords-1; i >= 0; i--) {
        x += strlen(aster_words[i].s)+1;
        if(x >= 80) { printf("\n"); x = strlen(aster_words[i].s)+1; }
        printf("%s ", aster_words[i].s);
    }
    printf("\n");
}

void aster_f_include() {
    aster_getNext(aster_buf, ASTER_BUFSZ);
    aster_runFile(aster_buf);
}

void aster_f_openFile() {
    aster_sassert(3);
    memcpy(aster_buf, &aster_dict[aster_stack[aster_sp-3]],
      aster_stack[aster_sp-2]);
    aster_buf[aster_stack[aster_sp-2]] = 0;
    aster_files[aster_filen] =
      fopen(aster_buf, aster_fileModes[aster_stack[aster_sp-1]]);
    aster_sp -= 3;
    if(!aster_files[aster_filen]) { aster_stack[aster_sp++] = -1; return; }

    aster_stack[aster_sp++] = aster_filen;
    aster_filen = (aster_filen+1)%ASTER_NFILES;
    aster_stack[aster_sp++] = 0;
    aster_soassert(2);
}

void aster_f_closeFile() {
    aster_sassert(1);
    fclose(aster_files[aster_stack[aster_sp-1]]);
    aster_files[aster_stack[aster_sp-1]] = 0;
    aster_stack[aster_sp-1] = 0;
}

void aster_f_validFile() {
    aster_sassert(1);
    if(aster_stack[aster_sp-1] < 0 || aster_stack[aster_sp-1] >= ASTER_NFILES)
        aster_stack[aster_sp-1] = -1;
    else
        aster_stack[aster_sp-1] =
          (aster_files[aster_stack[aster_sp-1]] != 0)*-1;
}

void aster_f_fgetc() {
    aster_sassert(1);
    aster_stack[aster_sp-1] = fgetc(aster_files[aster_stack[aster_sp-1]]);
}

void aster_f_fputc() {
    aster_sassert(2);
    aster_sp -= 2;
    fputc(aster_stack[aster_sp], aster_files[aster_stack[aster_sp+1]]);
}

void aster_f_if1() {
    aster_sassert(1);
    if(!aster_stack[--aster_sp]) aster_waitThen = 1;
}

void aster_f_error() {
    aster_sassert(1);
    aster_error = aster_stack[--aster_sp];
}

void aster_f_trace() {
    aster_sassert(1);
    aster_trace = aster_stack[--aster_sp] != 0;
}

void aster_f_bye() {
    exit(0);
}

/*** END DICTIONARY ***/

void aster_addC(void (*fun)(void), const char *name, char flags) {
    aster_words[aster_nwords].s     = (char*)name;
    aster_words[aster_nwords].flags = flags|ASTER_FUNCTION;
    aster_words[aster_nwords].a     = 0;
    aster_words[aster_nwords++].f   = fun;
}

void aster_addConstant(int v, const char *name) {
    aster_words[aster_nwords].s     = (char*)name;
    aster_words[aster_nwords].flags = 0;
    aster_words[aster_nwords].a     = aster_here;
    aster_words[aster_nwords++].f   = 0;

    *(void (**)(void))&aster_dict[aster_here] = aster_f_lit;
    aster_here += ASTER_FUNSZ;
    *(int*)&aster_dict[aster_here] = v;
    aster_here += ASTER_INTSZ;
    *(void (**)(void))&aster_dict[aster_here] = aster_f_ret;
    aster_here += ASTER_FUNSZ;
    aster_words[aster_nwords-1].end = aster_here;
}

void aster_init(int argc, char **args) {
    int i;

    aster_sp = 0;
    aster_rsp = 0;
    aster_here = ASTER_START+argc*ASTER_INTSZ;
    aster_nwords = 0;
    aster_error = 0;
    aster_waitThen = 0;
    *(int*)&aster_dict[ASTER_BASE] = 10;
    *(int*)&aster_dict[ASTER_STATUS] = 0;
    *(int*)&aster_dict[ASTER_ARGC] = argc;

    for(i = 0; i < argc; i++) {
        *(int*)&aster_dict[ASTER_START+i*ASTER_INTSZ] = aster_here;
        strcpy(&aster_dict[aster_here], args[i]);
        aster_here += strlen(args[i])+1;
    }

    aster_addC(aster_f_rph, ">r", ASTER_COMPILEONLY);
    aster_addC(aster_f_rpl, "r>", ASTER_COMPILEONLY);
    aster_addC(aster_f_rat, "r@", ASTER_COMPILEONLY);
    aster_addC(aster_f_add, "+",   0);
    aster_addC(aster_f_sub, "-",   0);
    aster_addC(aster_f_and, "and", 0);
    aster_addC(aster_f_or,  "or",  0);
    aster_addC(aster_f_xor, "xor", 0);
    aster_addC(aster_f_mul, "*",   0);
    aster_addC(aster_f_div, "/mod", 0);
    aster_addC(aster_f_inc, "1+", 0);
    aster_addC(aster_f_dec, "1-", 0);
    aster_addC(aster_f_shr, "2/", 0);
    aster_addC(aster_f_shl, "2*", 0);
    aster_addC(aster_f_zeq, "0=", 0);
    aster_addC(aster_f_inv, "invert", 0);
    aster_addC(aster_f_swap, "swap", 0);
    aster_addC(aster_f_dup,  "dup",  0);
    aster_addC(aster_f_over, "over", 0);
    aster_addC(aster_f_nip,  "nip",  0);
    aster_addC(aster_f_tuck, "tuck", 0);
    aster_addC(aster_f_drop, "drop", 0);
    aster_addC(aster_f_rot,  "rot",  0);
    aster_addC(aster_f_mrot, "-rot", 0);
    aster_addC(aster_f_depth, "depth", 0);
    aster_addC(aster_f_pick, "pick", 0);
    aster_addC(aster_f_sw, "!", 0);
    aster_addC(aster_f_lw, "@", 0);
    aster_addC(aster_f_sb, "c!", 0);
    aster_addC(aster_f_lb, "c@", 0);
    aster_addC(aster_f_here, "here", 0);
    aster_addC(aster_f_allot, "allot", 0);
    aster_addC(aster_f_execute, "execute", 0);
    aster_addC(aster_f_tick, "'", 0);
    aster_addC(aster_f_compile, "compile,", ASTER_COMPILEONLY);
    aster_addC(aster_f_postpone,"postpone",ASTER_COMPILEONLY|ASTER_IMMEDIATE);
    aster_addC(aster_f_alias, "alias", 0);
    aster_addC(aster_f_literal, "literal", ASTER_COMPILEONLY|ASTER_IMMEDIATE);
    aster_addC(aster_f_jmpc, "jmp,", ASTER_COMPILEONLY);
    aster_addC(aster_f_jzc,  "jz,",  ASTER_COMPILEONLY);
    aster_addC(aster_f_jmps, "jmp!", 0);
    aster_addC(aster_f_lits, "lit!", 0);
    aster_addC(aster_f_recurse, "recurse", ASTER_COMPILEONLY|ASTER_IMMEDIATE);
    aster_addC(aster_f_exit, "exit", ASTER_COMPILEONLY|ASTER_IMMEDIATE);
    aster_addC(aster_f_parsec, "parseC", 0);
    aster_addC(aster_f_emit, "emit", 0);
    aster_addC(aster_f_cin, "cin", 0);
    aster_addC(aster_f_colon,  ":", 0);
    aster_addC(aster_f_noname, ":NONAME", 0);
    aster_addC(aster_f_semi,   ";", ASTER_COMPILEONLY|ASTER_IMMEDIATE);
    aster_addC(aster_f_last,   "last", 0);
    aster_addC(aster_f_immediate, "immediate", 0);
    aster_addC(aster_f_compileonly, "compile-only", 0);
    aster_addC(aster_f_see, "see", 0);
    aster_addC(aster_f_words, "words", 0);
    aster_addC(aster_f_include, "include", 0);
    aster_addC(aster_f_openFile, "open-file", 0);
    aster_addC(aster_f_closeFile, "close-file", 0);
    aster_addC(aster_f_validFile, "valid-file?", 0);
    aster_addC(aster_f_fputc, "fputc", 0);
    aster_addC(aster_f_fgetc, "fgetc", 0);
    aster_addC(aster_f_if1, "[if]", 0);
    aster_addC(aster_f_error, "error", 0);
    aster_addC(aster_f_trace, "trace!", 0);
    aster_addC(aster_f_bye, "bye", 0);

    aster_addConstant(ASTER_BASE,   "base");
    aster_addConstant(ASTER_STATUS, "status");
    aster_addConstant(ASTER_INTSZ,  "cell");
    aster_addConstant(ASTER_FUNSZ,  "funsz");
    aster_addConstant(ASTER_ARGC,   "argc");
    aster_addConstant(ASTER_START,  "(args)");
    aster_addConstant(0, "r/o");
    aster_addConstant(1, "w/o");
    aster_addConstant(2, "r/w");
}

int aster_hex(char *s, int *n, char neg) {
    if(*s == '-') { if(neg) return 0; s++; neg = 1; }
    if(*s == 0) return 0;

    do {
        (*n) <<= 4;
        if(*s >= '0' && *s <= '9') *n |= *s - '0';
        else if(*s >= 'a' && *s <= 'f') *n |= *s - 'a' + 10;
        else if(*s >= 'A' && *s <= 'F') *n |= *s - 'A' + 10;
        else return 0;
    } while(*(++s));

    if(neg) *n *= -1;
    return 1;
}

int aster_bin(char *s, int *n, char neg) {
    if(*s == '-') { if(neg) return 0; s++; neg = 1; }
    if(*s == 0) return 0;

    do {
        *n <<= 1;
        if(*s == '1') *n |= 1;
        else if(*s != '0') return 0;
    } while(*(++s));

    if(neg) *n *= -1;
    return 1;
}

int aster_den(char *s, int *n, char neg) {
    if(*s == '-') { if(neg) return 0; s++; neg = 1; }
    if(*s == 0) return 0;

    do {
        *n *= 10;
        if(*s >= '0' && *s <= '9') *n += *s - '0';
        else return 0;
    } while(*(++s));

    if(neg) *n *= -1;
    return 1;
}

int aster_number(char *s, int *n) {
    char neg;
    int base;

    if(*s == '-') { s++; neg = 1; }
    else neg = 0;

    *n = 0;

    if(*s == 0) return 0;
    if(*s == '$') return aster_hex(s+1, n, neg);
    if(*s == '%') return aster_bin(s+1, n, neg);
    if(*s == '#') return aster_den(s+1, n, neg);

    base = *(int*)&aster_dict[ASTER_BASE];

    if(base <= 10) {
        do {
            *n *= base;
            if(*s >= '0' && *s < '0'+base) *n += *s - '0';
            else return 0;
        } while(*(++s));
    } else {
        do {
            *n *= base;
            if(*s >= '0' && *s <= '9') *n += *s - '0';
            else if(*s >= 'a' && *s < 'a'+base-10) *n += *s - 'a' + 10;
            else if(*s >= 'A' && *s < 'A'+base-10) *n += *s - 'A' + 10;
            else return 0;
        } while(*(++s));
    }

    if(neg) *n *= -1;
    return 1;
}

void aster_run() {
    int n;
    struct aster_word *w;

    while(!aster_error) {
        aster_getNext(aster_buf, ASTER_BUFSZ);
        if(!(*aster_buf)) return;

        if(aster_waitThen) {
            if(aster_streq(aster_buf, "[then]")) aster_waitThen = 0;
            continue;
        }

        if(w = aster_findWord(aster_buf)) {
            if(*(int*)&aster_dict[ASTER_STATUS]) {
                if(w->flags & ASTER_IMMEDIATE) {
                    if(w->flags & ASTER_FUNCTION) w->f();
                    else aster_runAddr(w->a);
                } else if(w->flags & ASTER_FUNCTION) {
                    *(void(**)(void))&aster_dict[aster_here] = w->f;
                    aster_here += ASTER_FUNSZ;
                } else {
                    *(void(**)(void))&aster_dict[aster_here] = aster_f_call;
                    aster_here += ASTER_FUNSZ;
                    *(int*)&aster_dict[aster_here] = w->a;
                    aster_here += ASTER_INTSZ;
                }
            } else if(w->flags & ASTER_COMPILEONLY) {
                printf("%s is compile only\n", w->s);
                aster_error = 1;
            } else if(w->flags & ASTER_FUNCTION) {
                w->f();
            } else {
                aster_runAddr(w->a);
            }
        } else if(aster_number(aster_buf, &n)) {
            if(*(int*)&aster_dict[ASTER_STATUS]) {
                *(void(**)(void))&aster_dict[aster_here] = aster_f_lit;
                aster_here += ASTER_FUNSZ;
                *(int*)&aster_dict[aster_here] = n;
                aster_here += ASTER_INTSZ;
            } else {
                aster_stack[aster_sp++] = n;
            }
        } else {
            printf("%s ?\n", aster_buf);
            aster_error = 1;
        }
    }
}

void aster_runFile(const char *filename) {
    char buf[ASTER_LINEBUFSZ];
    char *p;
    FILE *fp;

    fp = fopen(filename, "r");
    if(!fp) {
        printf("failed to open %s\n", filename);
        aster_error = 1;
        return;
    }

    p = buf;
    while(!feof(fp)) {
        *p = fgetc(fp);
        if(*p == EOF || *p == '\n' || p-buf >= ASTER_LINEBUFSZ-2) {
            if(p != buf) { *p = 0; aster_runString(buf); p = buf; }
        } else p++;
    }

    fclose(fp);
}

void aster_runStdin() {
    char buf[ASTER_LINEBUFSZ];
    char *p;

    printf("  ok\n");
    p = buf;
    for(;;) {
        if(aster_error) {
            printf("error\n");
            aster_sp = 0;
            aster_rsp = 0;
            if(*(int*)&aster_dict[ASTER_STATUS]) {
                aster_here = aster_words[aster_nwords].a;
                *(int*)&aster_dict[ASTER_STATUS] = 0;
            }
            aster_error = 0;
        }

        *p = fgetc(stdin);
        if(*p == EOF) {
            if(p != buf) { *p = 0; aster_runString(buf); }
            return;
        } else if(*p == '\n' || p-buf >= ASTER_LINEBUFSZ-2) {
            if(p != buf) { *p = 0; aster_runString(buf); }
            if(aster_error);
            else if(*(int*)&aster_dict[ASTER_STATUS]) printf("  compiled\n");
            else printf("  ok\n");
            p = buf;
        } else p++;
    }
}

void aster_runString(char *s) {
    char buf[ASTER_BUFSZ];
    char *p;

    if(p = strchr(s, '\n')) {
        strncpy(buf, s, ASTER_BUFSZ);
        buf[ASTER_BUFSZ-1] = 0;
        buf[p-s] = 0;
        aster_string = buf;
        aster_run();
        aster_runString(p+1);
    } else {
        aster_string = s;
        aster_run();
    }
}

