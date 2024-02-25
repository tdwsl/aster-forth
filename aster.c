/* aster forth - tdwsl 2023 */

#include "aster.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef ASTER_TERMIOS
#include <termios.h>

void aster_f_getch() {
    struct termios term, old;
    tcgetattr(0, &old);
    memcpy(&term, &old, sizeof(struct termios));
    term.c_lflag &= ~ICANON;
    term.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &term);

    aster_stack[aster_sp++] = getchar();

    tcsetattr(0, TCSANOW, &old);
}
#endif

struct aster_word {
    char *s;
    char flags;
    int a;
    int end;
};

unsigned char aster_dict[ASTER_DICTSZ];
int aster_stack[256];
int aster_rstack[256];
int aster_cstack[256];
unsigned char aster_sp=0, aster_rsp=0;
int aster_here = 0;
int aster_pc, aster_ppc;
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
unsigned char aster_usedArgs = 0;
int aster_lastFun;
aster_fun aster_functions[ASTER_MAXFUNS];
int aster_nfunctions = 0;

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

void aster_printIns(int addr);

int aster_printAddr(int addr);

void aster_printStack() {
    int i;
    printf("DS: ");
    for(i = 0; i < aster_sp; i++)
        printf("%x ", aster_stack[i]);
    printf("\n");
}

void aster_printRstack() {
    int i;
    printf("RS: ");
    for(i = 0; i < aster_rsp; i++)
        printf("%x ", aster_rstack[i]);
    printf("\n");
}

void aster_runAddr(int pc) {
    int a;

    if(pc < 0) {
        pc = ~pc;
        if(pc >= aster_nfunctions) {
            strcpy(aster_buf, "invalid function\n"); aster_error = 1;
        } else {
            aster_lastFun = ~pc;
            aster_functions[pc]();
        }
        return;
    }

    aster_pc = pc;
    aster_rstack[aster_rsp++] = 0;

    do {
        aster_ppc = aster_pc;
        aster_bassert(aster_pc);
        a = *(int*)&aster_dict[aster_pc];
        aster_pc += ASTER_INTSZ;
        if(a < 0) {
            a = ~a;
            if(a >= aster_nfunctions) {
                strcpy(aster_buf, "invalid function\n"); aster_error = 1;
            } else aster_functions[a]();
        } else {
            aster_rstack[aster_rsp++] = aster_pc;
            aster_pc = a;
        }
    } while((aster_pc != 0) & (!aster_error));

    if(aster_rsp != 0 && !aster_error) {
        sprintf(aster_buf, "%s", aster_sOB); aster_error = 1;
    }
}

struct aster_word *aster_findWord(char *s) {
    int i;

    for(i = aster_nwords-1; i >= 0; i--)
        if(!strcasecmp(s, aster_words[i].s))
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
    aster_sassert(1);
    if(aster_stack[--aster_sp]) aster_pc += ASTER_INTSZ;
    else aster_pc = *(int*)&aster_dict[aster_pc];
}

void aster_f_call() {
    aster_rstack[aster_rsp++] = aster_pc + ASTER_INTSZ;
    aster_pc = *(int*)&aster_dict[aster_pc];
}

void aster_f_ret() {
    aster_pc = aster_rstack[--aster_rsp];
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
    if(aster_stack[aster_sp-1]) {
        t = aster_stack[aster_sp-2] / aster_stack[aster_sp-1];
        aster_stack[aster_sp-2] %= aster_stack[aster_sp-1];
        aster_stack[aster_sp-1] = t;
    } else {
        sprintf(aster_buf, "divide by zero\n");
        aster_error = 1;
    }
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

void aster_f_rshift() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] >>= aster_stack[aster_sp];
}

void aster_f_lshift() {
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] <<= aster_stack[aster_sp];
}

void aster_f_zeq() {
    aster_sassert(1);
    aster_stack[aster_sp-1] = aster_stack[aster_sp-1] ? 0 : -1;
}

void aster_f_zne() {
    aster_sassert(1);
    aster_stack[aster_sp-1] = aster_stack[aster_sp-1] ? -1 : 0;
}

void aster_f_zge() {
    aster_sassert(1);
    aster_stack[aster_sp-1] = (aster_stack[aster_sp-1] < 0) ? 0 : -1;
}

void aster_f_zlt() {
    aster_sassert(1);
    aster_stack[aster_sp-1] = (aster_stack[aster_sp-1] < 0) ? -1 : 0;
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

void aster_f_roll() {
    int i, r, t;
    aster_sassert(1);
    r = aster_stack[--aster_sp];
    if(r <= 0) return;
    aster_sassert(r+1);
    t = aster_stack[aster_sp-r-1];
    for(i = aster_sp-r-1; i < aster_sp-1; i++)
        aster_stack[i] = aster_stack[i+1];
    aster_stack[aster_sp-1] = t;
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

void aster_f_addsw() {
    aster_sassert(2);
    aster_bassert(aster_stack[aster_sp-1]);
    *(int*)&aster_dict[aster_stack[aster_sp-1]] += aster_stack[aster_sp-2];
    aster_sp -= 2;
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
    if(aster_stack[aster_sp] < 0) aster_functions[~aster_stack[aster_sp]]();
    else if(aster_rsp) {
        aster_rstack[aster_rsp++] = aster_pc;
        aster_pc = aster_stack[aster_sp];
    } else aster_runAddr(aster_stack[aster_sp]);
}

struct aster_word *aster_getNextWord() {
    struct aster_word *w;

    aster_getNext(aster_buf, ASTER_BUFSZ);
    w = aster_findWord(aster_buf);
    if(!w) { strcat(aster_buf, " ?\n"); aster_error = 1; }
    return w;
}

void aster_f_tick() {
    struct aster_word *w;

    w = aster_getNextWord();
    if(!w) return;
    aster_stack[aster_sp] = w->a;
    aster_sp++;
    aster_soassert(1);
}

struct aster_word *aster_findWordAddr(int a) {
    int i;

    for(i = aster_nwords-1; i >= 0; i--)
        if(aster_words[i].a == a)
            return &aster_words[i];
    return 0;
}

void aster_f_isImmediate() {
    struct aster_word *w;
    w = aster_findWordAddr(aster_stack[aster_sp-1]);
    if(!w) aster_stack[aster_sp-1] = 0;
    aster_stack[aster_sp-1] = (w->flags & ASTER_IMMEDIATE) ? -1 : 0;
}

void aster_f_find() {
    char buf[256];
    struct aster_word *w;
    strncpy(buf, &aster_dict[aster_stack[aster_sp-1]+1],
            (unsigned)aster_dict[aster_stack[aster_sp-1]]);
    if(w = aster_findWord(buf)) {
        aster_stack[aster_sp-1] = w->a;
        aster_stack[aster_sp++] = (w->flags&ASTER_IMMEDIATE)?1:-1;
    } else aster_stack[aster_sp++] = 0;
}

void aster_f_alias() {
    struct aster_word *w;

    aster_sassert(1);
    aster_getNext(aster_nameBufP,
      ASTER_NAMEBUFSZ-(int)(aster_nameBufP-aster_nameBuf)-1);
    aster_words[aster_nwords].s = aster_nameBufP;
    aster_nameBufP += strlen(aster_nameBufP)+1;
    aster_sp--;
    aster_words[aster_nwords].a = aster_stack[aster_sp];
    aster_words[aster_nwords].flags = 0;
    w = aster_findWordAddr(aster_stack[aster_sp]);
    aster_words[aster_nwords].end = w ? w->end : 0;
    aster_nwords++;
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
    *(int*)&aster_dict[aster_here] = ~ASTER_RET;
    aster_here += ASTER_INTSZ;
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

void aster_f_this() {
    aster_stack[aster_sp++] = aster_words[aster_nwords].a;
    aster_soassert(1);
}

void aster_f_immediate() {
    aster_words[aster_nwords-1].flags |= ASTER_IMMEDIATE;
}

int aster_printAddr(int addr) {
    int i;
    for(i = aster_nwords-1; i >= 0; i--)
        if(aster_words[i].a == addr)
            return printf("%s ", aster_words[i].s);
    return printf("$%.8X ", addr);
}

int aster_printInsAddr(int addr) {
    return printf("%.8X ", addr);
}

int aster_printIns0(int addr) {
    int i;

    i = *(int*)&aster_dict[addr];
    if(i == ~ASTER_LIT)
        return printf("#%d ", *(int*)&aster_dict[addr+ASTER_INTSZ]);
    else if(i == ~ASTER_JMP)
        return printf("JMP %.8X ", *(int*)&aster_dict[addr+ASTER_INTSZ]);
    else if(i == ~ASTER_JZ)
        return printf("JZ %.8X ", *(int*)&aster_dict[addr+ASTER_INTSZ]);
    else if(i == ~ASTER_RET)
        return printf("RET ");
    else
        return aster_printAddr(*(int*)&aster_dict[addr]);
}

void aster_printIns(int addr) {
    aster_printInsAddr(addr);
    aster_printIns0(addr);
    printf("\n");
}

int aster_intIn(int *a, int l, int n) {
    int i;
    for(i = 0; i < l; i++)
        if(n == a[i]) return 1;
    return 0;
}

void aster_f_see() {
    struct aster_word *w;
    int i, n;
    int br[200];
    int nbr = 0, x = 0;

    w = aster_getNextWord();
    if(!w) return;
    printf("%s", w->s);
    if(w->flags & ASTER_IMMEDIATE) printf(" (immediate)");
    printf("\n");

    if(w->a < 0) {
        printf("function $%.X\n", (unsigned)(size_t)aster_functions[~w->a]);
        return;
    }

    for(i = w->a; i < w->end; i += ASTER_INTSZ) {
        n = *(int*)&aster_dict[i];
        if(n == ~ASTER_JMP || n == ~ASTER_JZ)
            br[nbr++] = *(int*)&aster_dict[i+ASTER_INTSZ];
        if(n == ~ASTER_JMP || n == ~ASTER_JZ || n == ~ASTER_LIT)
            i += ASTER_INTSZ;
    }

    aster_printInsAddr(w->a);
    for(i = w->a; i < w->end; i += ASTER_INTSZ) {
        if(aster_intIn(br, nbr, i) && x) {
            printf("\n");
	    aster_printInsAddr(i);
            x = 0;
        }
        x += aster_printIns0(i);
        n = *(int*)&aster_dict[i];
        if(n == ~ASTER_JMP || n == ~ASTER_JZ || n == ~ASTER_LIT)
            i += ASTER_INTSZ;
        if(x >= 60 || n == ~ASTER_JMP || n == ~ASTER_JZ) {
            printf("\n");
	    aster_printInsAddr(i+ASTER_INTSZ);
            x = 0;
        }
    }
    printf("\n");
}

void aster_f_words() {
    int i, x;

    x = 0;
    for(i = 0; i < aster_nwords; i++) {
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

void aster_f_accessArgs() {
    aster_usedArgs = -1;
}

void aster_f_marker() {
    int i;
    aster_sassert(1);
    aster_here = aster_stack[--aster_sp];
    for(i = 0; i < aster_nwords; i++)
        if(aster_words[i].a >= aster_here) {
            aster_nwords = i;
            aster_nameBufP = aster_words[i].s;
        }
}

void aster_f_error() {
    aster_sassert(1);
    *aster_buf = 0;
    aster_error = aster_stack[--aster_sp];
}

void aster_resetStacks();

void aster_f_catch() {
    int rsp;
    aster_sassert(1);
    rsp = aster_rsp;
    aster_execute(aster_stack[--aster_sp]);
    if(aster_error) {
        aster_resetStacks(); aster_rsp = rsp; aster_stack[aster_sp++] = -1;
    } else aster_stack[aster_sp++] = 0;
}

int aster_number(char *s, int *n);

void aster_f_number() {
    char *buf;
    aster_sassert(2);
    buf = alloca(aster_stack[aster_sp-1]+1);
    memcpy(buf, &aster_dict[aster_stack[aster_sp-2]], aster_stack[aster_sp-1]);
    buf[aster_stack[aster_sp-1]] = 0;
    if(aster_number(buf, &aster_stack[aster_sp-2]))
        aster_stack[aster_sp-1] = -1;
    else {
        aster_sp--;
        aster_stack[aster_sp-1] = 0;
    }
}

void aster_f_evaluate() {
    char buf[1024];
    aster_sassert(2);
    strncpy(buf,
        &aster_dict[aster_stack[aster_sp-2]], aster_stack[aster_sp-1]);
    buf[aster_stack[aster_sp-1]] = 0;
    aster_runString(buf);
}

void aster_f_time() {
    time_t t;
    t = time(0);
    aster_stack[aster_sp++] = t;
    aster_stack[aster_sp++] = t>>(sizeof(int)*8);
    aster_soassert(2);
}

void aster_f_bye() {
    exit(0);
}

/*** END DICTIONARY ***/

void aster_addC(void (*fun)(void), const char *name, char flags) {
    aster_words[aster_nwords].s     = (char*)name;
    aster_words[aster_nwords].flags = flags;
    aster_words[aster_nwords].a     = ~aster_nfunctions;
    aster_nwords++;
    aster_functions[aster_nfunctions++] = fun;
}

void aster_addConstant(int v, const char *name) {
    aster_words[aster_nwords].s     = (char*)name;
    aster_words[aster_nwords].flags = 0;
    aster_words[aster_nwords].a     = aster_here;
    aster_nwords++;

    *(int*)&aster_dict[aster_here] = ~ASTER_LIT;
    aster_here += ASTER_INTSZ;
    *(int*)&aster_dict[aster_here] = v;
    aster_here += ASTER_INTSZ;
    *(int*)&aster_dict[aster_here] = ~ASTER_RET;
    aster_here += ASTER_INTSZ;
    aster_words[aster_nwords-1].end = aster_here;
}

void aster_addWord(int a, const char *name, int flags) {
    aster_words[aster_nwords].s     = (char*)name;
    aster_words[aster_nwords].flags = flags;
    aster_words[aster_nwords].a     = a;
    aster_nwords++;
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
    aster_functions[ASTER_JMP] = aster_f_jmp;
    aster_functions[ASTER_JZ] = aster_f_jz;
    aster_functions[ASTER_LIT] = aster_f_lit;
    aster_functions[ASTER_RET] = aster_f_ret;
    aster_nfunctions = ASTER_USER;

    aster_addConstant(~ASTER_JMP, "jmp");
    aster_addConstant(~ASTER_JZ, "jz");
    aster_addConstant(~ASTER_LIT, "lit");
    aster_addConstant(~ASTER_RET, "ret");

    for(i = 0; i < argc; i++) {
        *(int*)&aster_dict[ASTER_START+i*ASTER_INTSZ] = aster_here;
        strcpy((char*)&aster_dict[aster_here], args[i]);
        aster_here += strlen(args[i])+1;
    }

    aster_addC(aster_f_rph, ">r", 0);
    aster_addC(aster_f_rpl, "r>", 0);
    aster_addC(aster_f_rat, "r@", 0);
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
    aster_addC(aster_f_rshift, "rshift", 0);
    aster_addC(aster_f_lshift, "lshift", 0);
    aster_addC(aster_f_zeq, "0=", 0);
    aster_addC(aster_f_zne, "0<>", 0);
    aster_addC(aster_f_zge, "0>=", 0);
    aster_addC(aster_f_zlt, "0<", 0);
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
    aster_addC(aster_f_roll, "roll", 0);
    aster_addC(aster_f_sw, "!", 0);
    aster_addC(aster_f_lw, "@", 0);
    aster_addC(aster_f_addsw, "+!", 0);
    aster_addC(aster_f_sb, "c!", 0);
    aster_addC(aster_f_lb, "c@", 0);
    aster_addC(aster_f_here, "here", 0);
    aster_addC(aster_f_allot, "allot", 0);
    aster_addC(aster_f_execute, "execute", 0);
    aster_addC(aster_f_tick, "'", 0);
    aster_addC(aster_f_isImmediate, "immediate?", 0);
    aster_addC(aster_f_find, "find", 0);
    aster_addC(aster_f_alias, "alias", 0);
    aster_addC(aster_f_parsec, "parseC", 0);
    aster_addC(aster_f_emit, "emit", 0);
    aster_addC(aster_f_cin, "cin", 0);
    aster_addC(aster_f_colon,  ":", 0);
    aster_addC(aster_f_noname, ":noname", 0);
    aster_addC(aster_f_semi,   ";", ASTER_IMMEDIATE);
    aster_addC(aster_f_last,   "last", 0);
    aster_addC(aster_f_this,   "this", 0);
    aster_addC(aster_f_immediate, "immediate", 0);
    aster_addC(aster_f_see, "see", 0);
    aster_addC(aster_f_words, "words", 0);
    aster_addC(aster_f_include, "include", 0);
    aster_addC(aster_f_openFile, "open-file", 0);
    aster_addC(aster_f_closeFile, "close-file", 0);
    aster_addC(aster_f_validFile, "valid-file?", 0);
    aster_addC(aster_f_fputc, "fputc", 0);
    aster_addC(aster_f_fgetc, "fgetc", 0);
    aster_addC(aster_f_if1, "[if]", 0);
    aster_addC(aster_f_accessArgs, "access-args", 0);
    aster_addC(aster_f_marker, "marker!", 0);
    aster_addC(aster_f_error, "error", 0);
    aster_addC(aster_f_catch, "catch", 0);
    aster_addC(aster_f_number, "number", 0);
    aster_addC(aster_f_evaluate, "evaluate", 0);
    aster_addC(aster_f_time, "time", 0);
    aster_addC(aster_f_bye, "bye", 0);

#ifdef ASTER_TERMIOS
    aster_addC(aster_f_getch, "key", 0);
#endif

    aster_addConstant(ASTER_BASE,   "base");
    aster_addConstant(ASTER_STATUS, "status");
    aster_addConstant(ASTER_INTSZ,  "cell");
    aster_addConstant(ASTER_ARGC,   "argc");
    aster_addConstant(ASTER_START,  "(args)");
    aster_addConstant(ASTER_DICTSZ, "heap0");
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

void aster_printError();

void aster_run() {
    int n;
    struct aster_word *w;

    while(!aster_error) {
        aster_ppc = 0;
        aster_lastFun = 0;

        aster_getNext(aster_buf, ASTER_BUFSZ);
        if(!(*aster_buf)) return;

        if(aster_waitThen) {
            if(!strcasecmp(aster_buf, "[then]")) aster_waitThen = 0;
            continue;
        }

        if((w = aster_findWord(aster_buf))) {
            if(*(int*)&aster_dict[ASTER_STATUS]) {
                if(w->flags & ASTER_IMMEDIATE) {
                    aster_runAddr(w->a);
                } else {
                    *(int*)&aster_dict[aster_here] = w->a;
                    aster_here += ASTER_INTSZ;
                }
            } else {
                aster_runAddr(w->a);
            }
        } else if(aster_number(aster_buf, &n)) {
            if(*(int*)&aster_dict[ASTER_STATUS]) {
                *(int*)&aster_dict[aster_here] = ~ASTER_LIT;
                aster_here += ASTER_INTSZ;
                *(int*)&aster_dict[aster_here] = n;
                aster_here += ASTER_INTSZ;
            } else {
                aster_stack[aster_sp++] = n;
            }
        } else {
            strcat(aster_buf, " ?\n");
            aster_error = 1;
        }
    }

    if(aster_error) aster_printError();
}

void aster_runFile(const char *filename) {
    char buf[ASTER_LINEBUFSZ];
    char *p;
    FILE *fp;

    fp = fopen(filename, "r");
    if(!fp) {
        sprintf(aster_buf, "failed to open %s\n", filename);
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

struct aster_word *aster_addrWord(int a) {
    int i;
    struct aster_word *w;
    for(i = 0; i < aster_nwords; i++) {
        w = &aster_words[i];
        if(!(w->flags & ASTER_FUNCTION) && a >= w->a && a < w->end)
            return w;
    }
    return 0;
}

void aster_printBacktrace() {
    int i;
    struct aster_word *w;
    if(!(aster_lastFun || aster_ppc)) return;
    if(aster_rsp) aster_rstack[aster_rsp++] = aster_pc;
    printf("backtrace:\n");
    if(aster_lastFun) aster_printAddr(aster_lastFun);
    else aster_printIns0(aster_ppc);
    printf("\n");
    for(i = aster_rsp-1; i > 0; i--) {
        w = aster_addrWord(aster_rstack[i]);
        if(w) printf("%s\n", w->s);
        else printf("$%.8x\n", aster_rstack[i]);
    }
}

void aster_printError() {
    printf("error: %s", aster_buf);
    aster_printBacktrace();
}

void aster_resetStacks() {
    aster_sp = 0;
    aster_rsp = 0;
    if(*(int*)&aster_dict[ASTER_STATUS]) {
        aster_here = aster_words[aster_nwords].a;
        *(int*)&aster_dict[ASTER_STATUS] = 0;
    }
    aster_error = 0;
}

void aster_runStdin() {
    char buf[ASTER_LINEBUFSZ];
    char *p;

    printf("  ok\n");
    p = buf;
    for(;;) {
        if(aster_error) aster_resetStacks();
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
    aster_string = s;
    aster_run();
}

