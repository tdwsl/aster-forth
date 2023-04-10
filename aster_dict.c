#include "aster.h"
#include "aster_bootstr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ASTER_NCURSES
#include <ncurses.h>
#endif

FILE *aster_files[200];

void aster_serr()
{
    aster_printf("stack underflow\n");
    aster_err();
}

void aster_rerr()
{
    aster_printf("return stack underflow\n");
    aster_err();
}

void aster_jmp()
{
    aster_pc = *(int*)(aster_dict+aster_pc);
}

void aster_call()
{
    aster_rstack[aster_rsp++] = aster_pc + sizeof(int)/sizeof(char);
    aster_pc = *(int*)(aster_dict+aster_pc);
}

void aster_ret()
{
    aster_rassert(1);
    aster_pc = aster_rstack[--aster_rsp];
}

void aster_jz()
{
    aster_sassert(1);
    if(!aster_stack[--aster_sp])
        aster_pc = *(int*)(aster_dict+aster_pc);
    else
        aster_pc += sizeof(int)/sizeof(char);
}

void aster_push()
{
    aster_stack[aster_sp++] = *(int*)(aster_dict+aster_pc);
    aster_pc += sizeof(int)/sizeof(char);
}

void aster_w_rph()
{
    aster_sassert(1);
    aster_rstack[aster_rsp++] = aster_stack[--aster_sp];
}

void aster_w_rpl()
{
    aster_rassert(1);
    aster_stack[aster_sp++] = aster_rstack[--aster_rsp];
}

void aster_w_rdrop()
{
    aster_rassert(1);
    aster_rsp--;
}

void aster_w_rget()
{
    aster_rassert(1);
    aster_stack[aster_sp++] = aster_rstack[aster_rsp-1];
}

void aster_w_dup()
{
    aster_sassert(1);
    aster_stack[aster_sp] = aster_stack[aster_sp-1];
    aster_sp++;
}

void aster_w_qdup()
{
    aster_sassert(1);
    if(aster_stack[aster_sp-1]) {
        aster_stack[aster_sp] = aster_stack[aster_sp-1];
        aster_sp++;
    }
}

void aster_w_drop()
{
    aster_sassert(1);
    aster_sp--;
}

void aster_w_over()
{
    aster_sassert(2);
    aster_stack[aster_sp] = aster_stack[aster_sp-2];
    aster_sp++;
}

void aster_w_swap()
{
    int i;
    aster_sassert(2);
    i = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = i;
}

void aster_w_nip()
{
    aster_sassert(2);
    aster_sp--;
    aster_stack[aster_sp-1] = aster_stack[aster_sp];
}

void aster_w_tuck()
{
    aster_sassert(2);
    aster_stack[aster_sp] = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp];
    aster_sp++;
}

void aster_w_rot()
{
    int i;
    aster_sassert(3);
    i = aster_stack[aster_sp-3];
    aster_stack[aster_sp-3] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = i;
}

void aster_w_mrot()
{
    int i;
    aster_sassert(3);
    i = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-3];
    aster_stack[aster_sp-3] = i;
}

void aster_w_pick()
{
    aster_sassert(aster_stack[aster_sp-1]+1);
    aster_stack[aster_sp-1] =
        aster_stack[aster_sp-2-aster_stack[aster_sp-1]];
}

void aster_w_depth()
{
    aster_stack[aster_sp] = aster_sp;
    aster_sp++;
}

void aster_w_2dup()
{
    aster_sassert(2);
    aster_stack[aster_sp] = aster_stack[aster_sp-2];
    aster_stack[aster_sp+1] = aster_stack[aster_sp-1];
    aster_sp += 2;
}

void aster_w_2drop()
{
    aster_sassert(2);
    aster_sp -= 2;
}

void aster_w_2over()
{
    aster_sassert(4);
    aster_stack[aster_sp] = aster_stack[aster_sp-4];
    aster_stack[aster_sp+1] = aster_stack[aster_sp-3];
    aster_sp += 2;
}

void aster_w_2swap()
{
    int i1, i2;
    aster_sassert(4);
    i1 = aster_stack[aster_sp-2];
    i2 = aster_stack[aster_sp-1];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-4];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-3];
    aster_stack[aster_sp-4] = i1;
    aster_stack[aster_sp-3] = i2;
}

void aster_w_2nip()
{
    aster_sassert(4);
    aster_stack[aster_sp-4] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-3] = aster_stack[aster_sp-1];
    aster_sp -= 2;
}

void aster_w_2tuck()
{
    aster_sassert(4);
    aster_stack[aster_sp] = aster_stack[aster_sp-2];
    aster_stack[aster_sp+1] = aster_stack[aster_sp-1];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-4];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-3];
    aster_stack[aster_sp-4] = aster_stack[aster_sp];
    aster_stack[aster_sp-3] = aster_stack[aster_sp+1];
    aster_sp += 2;
}

void aster_w_add()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] += aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_sub()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] -= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_mul()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] *= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_div()
{
    aster_sassert(2);
    if(!aster_stack[aster_sp-1]) {
        aster_printf("divide by zero\n");
        aster_err();
        return;
    }
    aster_stack[aster_sp-2] /= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_mod()
{
    aster_sassert(2);
    if(!aster_stack[aster_sp-1]) {
        aster_printf("divide by zero\n");
        aster_err();
        return;
    }
    aster_stack[aster_sp-2] %= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_divmod()
{
    int d;
    aster_sassert(2);
    if(!aster_stack[aster_sp-1]) {
        aster_printf("divide by zero\n");
        aster_err();
        return;
    }
    d = aster_stack[aster_sp-2] / aster_stack[aster_sp-1];
    aster_stack[aster_sp-2] %= aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = d;
}

void aster_w_inc()
{
    aster_sassert(1);
    aster_stack[aster_sp-1]++;
}

void aster_w_dec()
{
    aster_sassert(1);
    aster_stack[aster_sp-1]--;
}

void aster_w_shr()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] >>= 1;
}

void aster_w_shl()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] <<= 1;
}

void aster_w_and()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] &= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_or()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] |= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_xor()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] ^= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_invert()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] = ~aster_stack[aster_sp-1];
}

void aster_w_abs()
{
    aster_sassert(1);
    if(aster_stack[aster_sp-1] < 0) aster_stack[aster_sp-1] *= -1;
}

void aster_w_equ()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] =
        (aster_stack[aster_sp-2] == aster_stack[aster_sp-1])*-1;
    aster_sp--;
}

void aster_w_gre()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] =
        (aster_stack[aster_sp-2] > aster_stack[aster_sp-1])*-1;
    aster_sp--;
}

void aster_w_les()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] =
        (aster_stack[aster_sp-2] < aster_stack[aster_sp-1])*-1;
    aster_sp--;
}

void aster_w_greequ()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] =
        (aster_stack[aster_sp-2] >= aster_stack[aster_sp-1])*-1;
    aster_sp--;
}

void aster_w_lesequ()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] =
        (aster_stack[aster_sp-2] <= aster_stack[aster_sp-1])*-1;
    aster_sp--;
}

void aster_w_neq()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] =
        (aster_stack[aster_sp-2] != aster_stack[aster_sp-1])*-1;
    aster_sp--;
}

void aster_w_zequ()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] = (aster_stack[aster_sp-1] == 0)*-1;
}

void aster_w_zgre()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] = (aster_stack[aster_sp-1] > 0)*-1;
}

void aster_w_zles()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] = (aster_stack[aster_sp-1] < 0)*-1;
}

void aster_w_zgreequ()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] = (aster_stack[aster_sp-1] >= 0)*-1;
}

void aster_w_zlesequ()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] = (aster_stack[aster_sp-1] <= 0)*-1;
}

void aster_w_zneq()
{
    aster_sassert(1);
    aster_stack[aster_sp-1] = (aster_stack[aster_sp-1] != 0)*-1;
}

void aster_w_prnum()
{
    aster_sassert(1);
    aster_printf("%d ", aster_stack[--aster_sp]);
}

void aster_w_emit()
{
    aster_sassert(1);
    aster_printf("%c", aster_stack[--aster_sp]);
}

void aster_w_words()
{
    int i, x;
    x = 0;
    for(i = aster_nwords-1; i >= 0; i--)
    {
        x += strlen(aster_words[i].name)+1;
        if(x >= 64) { aster_printf("\n"); x = strlen(aster_words[i].name)+1; }
        aster_printf("%s ", aster_words[i].name);
    }
    aster_printf("\n");
}

void aster_w_prstack()
{
    int i;
    aster_printf("<%d> ", aster_sp);
    for(i = 0; i < aster_sp; i++)
        aster_printf("%d ", aster_stack[i]);
    aster_printf("\n");
}

void aster_w_bye()
{
#ifdef ASTER_NCURSES
    addstr("press any key");
    getch();
    scrollok(stdscr, 0);
    echo();
    endwin();
#endif
    exit(0);
}

void aster_w_immediate()
{
    aster_words[aster_nwords-1].flag |= ASTER_IMMEDIATE;
}

void aster_w_backslash()
{
    char c;
    while((c = aster_nextChar()) && c != '\n');
}

void aster_w_lbrace()
{
    char c;
    while((c = aster_nextChar()) && c != '\n' && c != ')');
}

void aster_getNextLower(char *s1)
{
    char c;
    char *s;
    while((c = aster_nextChar()) <= ' ' && c);
    if(!c) {
        aster_printf("parser expected token\n");
        *s1 = 0;
        aster_err();
        return;
    }
    s = s1;
    *(s++) = c;
    while((c = aster_nextChar()) > ' ') *(s++) = c;
    *s = 0;
}

void aster_capitalize(char *s)
{
    for(; *s; s++)
        if(*s >= 'a' && *s <= 'z') *s += 'A'-'a';
}

void aster_getNext(char *s)
{
    aster_getNextLower(s);
    aster_capitalize(s);
}

void aster_w_col()
{
    *(void (**)(void))(aster_dict+aster_here) = 0;
    *(int*)(aster_dict+ASTER_STATE) = ASTER_WORD;
    aster_getNext(aster_nextName);
    if(!*aster_nextName) return;
    aster_words[aster_nwords++] = (struct aster_word)
    {
        aster_nextName,
        0, aster_here, 0,
        0,
    };
    aster_nextName += strlen(aster_nextName)+1;
}

void aster_w_noname()
{
    *(int*)(aster_dict+ASTER_STATE) = ASTER_WORD;
    aster_words[aster_nwords++] = (struct aster_word)
    {
        "", 0,
        aster_here, 0,
        0,
    };
}

void aster_w_semi()
{
    *(void (**)(void))(aster_dict+aster_here) = aster_ret;
    aster_here += sizeof(void (**)(void))/sizeof(char);
    *(int*)(aster_dict+ASTER_STATE) = ASTER_RUN;
    if(!*aster_words[aster_nwords-1].name) {
        aster_stack[aster_sp++] = aster_words[aster_nwords-1].addr;
        return;
    }
    aster_words[aster_nwords-1].size =
        aster_here-aster_words[aster_nwords-1].addr;
}

void aster_werr(char *s)
{
    aster_printf("%s ?\n", s);
    aster_err();
}

void aster_w_see()
{
    struct aster_word *w;
    aster_getNext(aster_nextName);
    if(!*aster_nextName) return;
    w = aster_findWord(aster_nextName);
    if(!w) { aster_werr(aster_nextName); return; }
    if(w->flag & ASTER_COMPILEONLY)
        aster_printf("%s (COMPILE-ONLY)\n", aster_nextName);
    else if(w->flag & ASTER_IMMEDIATE)
        aster_printf("%s (IMMEDIATE)\n", aster_nextName);
    else
        aster_printf("%s\n", aster_nextName);
    if(w->flag & ASTER_C)
        aster_printf("function: 0x%x\n", (unsigned)(long long)w->fun);
    else
        aster_print(w->addr, w->addr+w->size);
}

void aster_w_forget()
{
    struct aster_word *w;
    int wi, i;
    aster_getNext(aster_nextName);
    if(!*aster_nextName) return;
    w = aster_findWord(aster_nextName);
    if(!w) { aster_werr(aster_nextName); return; }
    wi = w-aster_words;
    aster_nwords--;
    for(i = wi; i < aster_nwords; i++)
        aster_words[i] = aster_words[i+1];
}

void aster_w_include()
{
    *(void (**)(void))(aster_dict+aster_here) = 0;
    aster_getNextLower(aster_nextName);
    if(!*aster_nextName) return;
    aster_runFile(aster_nextName);
}

void aster_merr()
{
    aster_printf("memory out of range\n");
    aster_err();
}

#define aster_massert(M) if(M < 0 || \
    M+sizeof(int)/sizeof(char) >= ASTER_DICTSZ) aster_merr()
#define aster_cmassert(M) if(M < 0 || M >= ASTER_DICTSZ) aster_merr()

void aster_w_set()
{
    aster_sassert(2);
    aster_massert(aster_stack[aster_sp-1]);
    *(int*)(aster_dict+aster_stack[aster_sp-1]) = aster_stack[aster_sp-2];
    aster_sp -= 2;
}

void aster_w_get()
{
    aster_sassert(1);
    aster_massert(aster_stack[aster_sp-1]);
    aster_stack[aster_sp-1] = *(int*)(aster_dict+aster_stack[aster_sp-1]);
}

void aster_w_setc()
{
    aster_sassert(2);
    aster_cmassert(aster_stack[aster_sp-1]);
    aster_dict[aster_stack[aster_sp-1]] = (unsigned)aster_stack[aster_sp-2];
    aster_sp -= 2;
}

void aster_w_getc()
{
    aster_sassert(1);
    aster_cmassert(aster_stack[aster_sp-1]);
    aster_stack[aster_sp-1] =
        (unsigned char)aster_dict[aster_stack[aster_sp-1]];
}

void aster_w_here()
{
    aster_stack[aster_sp++] = aster_here;
}

void aster_w_allot()
{
    aster_sassert(1);
    aster_here += aster_stack[--aster_sp];
}

void aster_addConstant(int v, char *s)
{
    aster_words[aster_nwords++] = (struct aster_word)
    {
        s, 0, aster_here,
        (sizeof(void (*)(void))*2)/sizeof(char) + sizeof(int)/sizeof(char),
        0,
    };
    *(void (**)(void))(aster_dict+aster_here) = aster_push;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = v;
    aster_here += sizeof(int)/sizeof(char);
    *(void (**)(void))(aster_dict+aster_here) = aster_ret;
    aster_here += sizeof(void (*)(void))/sizeof(char);
}

void aster_w_constant()
{
    aster_sassert(1);
    aster_getNext(aster_nextName);
    if(!*aster_nextName) return;
    aster_addConstant(aster_stack[--aster_sp], aster_nextName);
    aster_nextName += strlen(aster_nextName)+1;
}

void aster_w_create()
{
    aster_stack[aster_sp++] = aster_here+
        (sizeof(void (*)(void))/sizeof(char))*2 + sizeof(int)/sizeof(char);
    aster_w_constant();
}

void aster_w_variable()
{
    aster_w_create();
    aster_here += sizeof(int)/sizeof(char);
}

void aster_w_parse()
{
    int i;
    aster_sassert(1);
    i = aster_stringPtr;
    do {
        aster_dict[i++] = aster_nextChar();
    } while(aster_dict[i-1] && aster_dict[i-1] != aster_stack[aster_sp-1]
        && aster_dict[i-1] != '\n');
    aster_sp--;
    aster_stack[aster_sp++] = aster_stringPtr;
    aster_stack[aster_sp++] = i-aster_stringPtr-1;
}

void aster_w_savestring()
{
    aster_sassert(1);
    aster_stringPtr += aster_stack[--aster_sp];
}

void aster_w_char()
{
    aster_getNextLower(aster_nextName);
    if(!*aster_nextName) return;
    aster_stack[aster_sp++] = *aster_nextName;
}

void aster_w_tick()
{
    struct aster_word *w;
    aster_getNext(aster_nextName);
    if(!*aster_nextName) return;
    w = aster_findWord(aster_nextName);
    if(!w) { aster_werr(aster_nextName); return; }
    if(w->flag & ASTER_C)
        aster_stack[aster_sp++] = (int)(long long)(w-aster_words)*-1-1;
    else
        aster_stack[aster_sp++] = w->addr;
}

void aster_w_literal()
{
    aster_sassert(1);
    *(void (**)(void))(aster_dict+aster_here) = aster_push;
    aster_here += sizeof(void (**)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = aster_stack[--aster_sp];
    aster_here += sizeof(int)/sizeof(char);
}

void aster_w_cell()
{
    aster_stack[aster_sp++] = sizeof(int)/sizeof(char);
}

void aster_w_execute()
{
    aster_sassert(1);
    if(aster_stack[--aster_sp] < 0)
        aster_words[(aster_stack[aster_sp]+1)*-1].fun();
    else {
        aster_rstack[aster_rsp++] = aster_pc;
        aster_pc = aster_stack[aster_sp];
        if(aster_rsp == 1) aster_run();
    }
}

void aster_compile()
{
    aster_sassert(1);
    if(aster_stack[--aster_sp] < 0) {
        *(void (**)(void))(aster_dict+aster_here) =
            aster_words[(aster_stack[aster_sp]+1)*-1].fun;
        aster_here += sizeof(void (*)(void))/sizeof(char);
    } else {
        *(void (**)(void))(aster_dict+aster_here) = aster_call;
        aster_here += sizeof(void (*)(void))/sizeof(char);
        *(int*)(aster_dict+aster_here) = aster_stack[aster_sp];
        aster_here += sizeof(int)/sizeof(char);
    }
}

void aster_w_postpone()
{
    struct aster_word *w;
    aster_getNext(aster_nextName);
    if(!*aster_nextName) return;
    w = aster_findWord(aster_nextName);
    if(!w) { aster_werr(aster_nextName); return; }
    if(w->flag & ASTER_IMMEDIATE) {
        if(w->flag & ASTER_C) {
            *(void (**)(void))(aster_dict+aster_here) = w->fun;
            aster_here += sizeof(void (*)(void))/sizeof(char);
        } else {
            *(void (**)(void))(aster_dict+aster_here) = aster_call;
            aster_here += sizeof(void (*)(void))/sizeof(char);
            *(int*)(aster_dict+aster_here) = w->addr;
            aster_here += sizeof(int)/sizeof(char);
        }
    } else {
        *(void (**)(void))(aster_dict+aster_here) = aster_push;
        aster_here += sizeof(void (*)(void))/sizeof(char);
        if(w->flag & ASTER_C)
            *(int*)(aster_dict+aster_here) =
                (int)(long long)(w-aster_words)*-1-1;
        else
            *(int*)(aster_dict+aster_here) = w->addr;
        aster_here += sizeof(int)/sizeof(char);
        *(void (**)(void))(aster_dict+aster_here) = aster_compile;
        aster_here += sizeof(void (*)(void))/sizeof(char);
    }
}

void aster_w_qcompile()
{
    aster_stack[aster_sp++] =
        (*(int*)(aster_dict+ASTER_STATE) == ASTER_WORD)*-1;
}

void aster_w_base()
{
    aster_stack[aster_sp++] = ASTER_BASE;
}

void aster_w_compileonly()
{
    aster_words[aster_nwords-1].flag |= ASTER_COMPILEONLY|ASTER_IMMEDIATE;
}

void aster_w_jmp()
{
    aster_sassert(1);
    *(void (**)(void))(aster_dict+aster_here) = aster_jmp;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = aster_stack[--aster_sp];
    aster_here += sizeof(int)/sizeof(char);
}

void aster_w_jz()
{
    aster_sassert(1);
    *(void (**)(void))(aster_dict+aster_here) = aster_jz;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = aster_stack[--aster_sp];
    aster_here += sizeof(int)/sizeof(char);
}

void aster_w_user()
{
    aster_stack[aster_sp++] = sizeof(void (*)(void))/sizeof(char);
}

void aster_restore()
{
    int i;
    aster_sassert(1);
    aster_here = aster_stack[--aster_sp];
    aster_nwords = aster_stack[--aster_sp];
}

void aster_w_marker()
{
    aster_getNext(aster_nextName);
    if(!*aster_nextName) return;
    aster_words[aster_nwords] = (struct aster_word)
    {
        aster_nextName, 0,
        aster_here, (sizeof(void (*)(void))/sizeof(char))*3
            +(sizeof(int)/sizeof(char))*2,
    };
    aster_nextName += strlen(aster_nextName)+1;
    *(void (**)(void))(aster_dict+aster_here) = aster_push;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = aster_nwords++;
    aster_here += sizeof(int);
    *(void (**)(void))(aster_dict+aster_here) = aster_push;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = aster_here;
    aster_here += sizeof(int);
    *(void (**)(void))(aster_dict+aster_here) = aster_restore;
    aster_here += sizeof(void (*)(void))/sizeof(char);
}

void aster_w_state()
{
    aster_stack[aster_sp++] = ASTER_STATE;
}

void aster_w_evaluate()
{
    char buf[2000];
    aster_sassert(2);
    strncpy(buf, aster_dict+aster_stack[aster_sp-2], aster_stack[aster_sp-1]);
    aster_sp -= 2;
    aster_runString(buf);
}

void aster_w_key()
{
    aster_stack[aster_sp++] = fgetc(stdin);
}

void aster_w_recurse()
{
    *(void (**)(void))(aster_dict+aster_here) = aster_call;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = aster_words[aster_nwords-1].addr;
    aster_here += sizeof(int)/sizeof(char);
}

void aster_w_find()
{
    char buf[200];
    struct aster_word *w;
    aster_sassert(1);
    strncpy(buf, aster_dict+aster_stack[aster_sp-1]+1,
        (unsigned char)aster_dict[aster_stack[aster_sp-1]]);
    buf[(unsigned char)aster_dict[aster_stack[aster_sp-1]]] = 0;
    aster_capitalize(buf);
    w = aster_findWord(buf);
    if(w) {
        if(w->flag & ASTER_C)
            aster_stack[aster_sp-1] = (int)(long long)(w-aster_words)*-1-1;
        else
            aster_stack[aster_sp-1] = w->addr;
        aster_stack[aster_sp++] = (w->flag & ASTER_IMMEDIATE) ? 1 : -1;
    } else aster_stack[aster_sp++] = 0;
}

void aster_w_accept()
{
    aster_sassert(2);
    aster_stack[aster_sp-2] =
        aster_accept(aster_dict+aster_stack[aster_sp-2],
            aster_stack[aster_sp-1]);
    aster_sp--;
}

int aster_nextFile()
{
    int i;
    for(i = 0; aster_files[i]; i++);
    return i;
}

void aster_w_openfile()
{
    int i;
    const char *aster_fileSpec[] = {
        "r", "w", "rw", "rb", "wb", "rwb",
    };
    aster_sassert(3);
    strncpy(aster_nextName, aster_dict+aster_stack[aster_sp-3],
        aster_stack[aster_sp-2]);
    aster_nextName[aster_stack[aster_sp-2]] = 0;
    i = aster_nextFile();
    aster_files[i] = fopen(aster_nextName,
        aster_fileSpec[aster_stack[aster_sp-1]]);
    aster_sp -= 3;
    aster_stack[aster_sp++] = i;
    aster_stack[aster_sp++] = (aster_files[i] == 0);
}

void aster_w_closefile()
{
    aster_sassert(1);
    if(aster_files[aster_stack[aster_sp-1]]) {
        fclose(aster_files[aster_stack[aster_sp-1]]);
        aster_stack[aster_sp-1] = 0;
    } else aster_stack[aster_sp-1] = 1;
}

void aster_w_readfile()
{
    aster_sassert(3);
    if(!aster_files[aster_stack[aster_sp-1]]) {
        aster_sp -= 3;
        aster_stack[aster_sp++] = 0;
        aster_stack[aster_sp++] = -1;
        return;
    }
    aster_stack[aster_sp-3] = fread(aster_dict+aster_stack[aster_sp-3], 1,
        aster_stack[aster_sp-2], aster_files[aster_stack[aster_sp-1]]);
    aster_sp--;
    aster_stack[aster_sp-1] = 0;
}

void aster_w_readline()
{
    int i;
    aster_sassert(3);
    if(!aster_files[aster_stack[aster_sp-1]]) {
        aster_sp -= 3;
        aster_stack[aster_sp++] = 0;
        aster_stack[aster_sp++] = -1;
        return;
    }
    for(i = 0; i < aster_stack[aster_sp-2]; i++) {
        if(!fread(aster_dict+aster_stack[aster_sp-3]+i, 1, 1,
            aster_files[aster_stack[aster_sp-1]])) break;
        if(aster_dict[aster_stack[aster_sp-3]+i] == '\n') break;
    }
    aster_sp -= 3;
    aster_stack[aster_sp++] = i;
    aster_stack[aster_sp++] = 0;
}

void aster_w_writefile()
{
    aster_sassert(3);
    if(!aster_files[aster_stack[aster_sp-1]]) {
        aster_sp -= 3;
        aster_stack[aster_sp++] = 0;
        aster_stack[aster_sp++] = -1;
        return;
    }
    fwrite(aster_dict+aster_stack[aster_sp-3], 1, aster_stack[aster_sp-2],
        aster_files[aster_stack[aster_sp-1]]);
    aster_sp -= 3;
    aster_stack[aster_sp++] = 0;
}

void aster_w_writeline()
{
    const char c = '\n';
    aster_w_writefile();
    if(!aster_stack[aster_sp-1]) {
        fwrite(&c, 1, 1, aster_files[aster_stack[aster_sp+1]]);
    }
}

void aster_w_argc()
{
    aster_stack[aster_sp++] = ASTER_ARGC;
}

void aster_w_arg()
{
    aster_sassert(1);
    if(aster_stack[aster_sp-1] < 0 ||
            aster_stack[aster_sp-1] >= *(int*)(aster_dict+ASTER_ARGC)) {
        aster_stack[aster_sp-1] = aster_args[0];
        aster_stack[aster_sp++] = 0;
    } else {
        aster_stack[aster_sp-1] = aster_args[aster_stack[aster_sp-1]];
        aster_stack[aster_sp] = strlen(aster_dict+aster_stack[aster_sp-1]);
        aster_sp++;
    }
}

void aster_init()
{
    *(int*)(aster_dict+ASTER_BASE) = 10;
    *(int*)(aster_dict+ASTER_STATE) = ASTER_RUN;
    *(void (**)(void))(aster_dict+ASTER_RET) = 0;
    aster_addC(aster_w_rph, ">R", 0);
    aster_addC(aster_w_rpl, "R>", 0);
    aster_addC(aster_w_rdrop, "RDROP", 0);
    aster_addC(aster_w_rget, "R@", 0);
    aster_addC(aster_w_dup, "DUP", 0);
    aster_addC(aster_w_qdup, "?DUP", 0);
    aster_addC(aster_w_drop, "DROP", 0);
    aster_addC(aster_w_over, "OVER", 0);
    aster_addC(aster_w_swap, "SWAP", 0);
    aster_addC(aster_w_nip, "NIP", 0);
    aster_addC(aster_w_tuck, "TUCK", 0);
    aster_addC(aster_w_rot, "ROT", 0);
    aster_addC(aster_w_mrot, "-ROT", 0);
    aster_addC(aster_w_pick, "PICK", 0);
    aster_addC(aster_w_depth, "DEPTH", 0);
    aster_addC(aster_w_2dup, "2DUP", 0);
    aster_addC(aster_w_2drop, "2DROP", 0);
    aster_addC(aster_w_2over, "2OVER", 0);
    aster_addC(aster_w_2swap, "2SWAP", 0);
    aster_addC(aster_w_2nip, "2NIP", 0);
    aster_addC(aster_w_2tuck, "2TUCK", 0);
    aster_addC(aster_w_add, "+", 0);
    aster_addC(aster_w_sub, "-", 0);
    aster_addC(aster_w_mul, "*", 0);
    aster_addC(aster_w_div, "/", 0);
    aster_addC(aster_w_mod, "MOD", 0);
    aster_addC(aster_w_divmod, "/MOD", 0);
    aster_addC(aster_w_inc, "1+", 0);
    aster_addC(aster_w_dec, "1-", 0);
    aster_addC(aster_w_shr, "2/", 0);
    aster_addC(aster_w_shl, "2*", 0);
    aster_addC(aster_w_and, "AND", 0);
    aster_addC(aster_w_or, "OR", 0);
    aster_addC(aster_w_xor, "XOR", 0);
    aster_addC(aster_w_invert, "INVERT", 0);
    aster_addC(aster_w_abs, "ABS", 0);
    aster_addC(aster_w_equ, "=", 0);
    aster_addC(aster_w_gre, ">", 0);
    aster_addC(aster_w_les, "<", 0);
    aster_addC(aster_w_greequ, ">=", 0);
    aster_addC(aster_w_lesequ, "<=", 0);
    aster_addC(aster_w_neq, "<>", 0);
    aster_addC(aster_w_zequ, "0=", 0);
    aster_addC(aster_w_zgre, "0>", 0);
    aster_addC(aster_w_zles, "0<", 0);
    aster_addC(aster_w_zgreequ, "0>=", 0);
    aster_addC(aster_w_zlesequ, "0<=", 0);
    aster_addC(aster_w_zneq, "0<>", 0);
    aster_addC(aster_w_prnum, ".", 0);
    aster_addC(aster_w_emit, "EMIT", 0);
    aster_addC(aster_w_words, "WORDS", 0);
    aster_addC(aster_w_prstack, ".S", 0);
    aster_addC(aster_w_bye, "BYE", 0);
    aster_addC(aster_w_backslash, "\\", ASTER_IMMEDIATE);
    aster_addC(aster_w_lbrace, "(", ASTER_IMMEDIATE);
    aster_addC(aster_w_immediate, "IMMEDIATE", 0);
    aster_addC(aster_w_col, ":", 0);
    aster_addC(aster_w_noname, ":NONAME", 0);
    aster_addC(aster_w_semi, ";", ASTER_IMMEDIATE);
    aster_addC(aster_w_see,  "SEE", 0);
    aster_addC(aster_w_forget, "FORGET", 0);
    aster_addC(aster_w_include, "INCLUDE", 0);
    aster_addC(aster_w_set, "!", 0);
    aster_addC(aster_w_get, "@", 0);
    aster_addC(aster_w_setc, "C!", 0);
    aster_addC(aster_w_getc, "C@", 0);
    aster_addC(aster_w_here, "HERE", 0);
    aster_addC(aster_w_allot, "ALLOT", 0);
    aster_addC(aster_w_constant, "CONSTANT", 0);
    aster_addC(aster_w_create, "CREATE", 0);
    aster_addC(aster_w_variable, "VARIABLE", 0);
    aster_addC(aster_w_parse, "PARSE", 0);
    aster_addC(aster_w_savestring, "SAVE-STRING", 0);
    aster_addC(aster_w_char, "CHAR", 0);
    aster_addC(aster_w_tick, "'", 0);
    aster_addC(aster_w_literal, "LITERAL",
        ASTER_IMMEDIATE|ASTER_COMPILEONLY);
    aster_addC(aster_w_cell, "CELL", 0);
    aster_addC(aster_w_execute, "EXECUTE", 0);
    aster_addC(aster_w_postpone, "POSTPONE",
        ASTER_IMMEDIATE|ASTER_COMPILEONLY);
    aster_addC(aster_w_qcompile, "COMPILE?", 0);
    aster_addC(aster_w_base, "BASE", 0);
    aster_addC(aster_w_compileonly, "COMPILE-ONLY", 0);
    aster_addC(aster_w_jmp, "JMP,", ASTER_COMPILEONLY|ASTER_IMMEDIATE);
    aster_addC(aster_w_jz, "JZ,", ASTER_COMPILEONLY|ASTER_IMMEDIATE);
    aster_addC(aster_ret, "EXIT", 0);
    aster_addC(aster_w_user, "USER", 0);
    aster_addC(aster_w_marker, "MARKER", 0);
    aster_addC(aster_w_state, "STATE", 0);
    aster_addC(aster_err, "ERROR", 0);
    aster_addC(aster_w_evaluate, "EVALUATE", 0);
    aster_addC(aster_w_key, "KEY", 0);
    aster_addC(aster_w_recurse, "RECURSE",
        ASTER_COMPILEONLY|ASTER_IMMEDIATE);
    aster_addC(aster_w_find, "FIND", 0);
    aster_addC(aster_w_accept, "ACCEPT", 0);
    aster_addC(aster_w_openfile, "OPEN-FILE", 0);
    aster_addC(aster_w_openfile, "CREATE-FILE", 0);
    aster_addC(aster_w_closefile, "CLOSE-FILE", 0);
    aster_addC(aster_w_readfile, "READ-FILE", 0);
    aster_addC(aster_w_readline, "READ-LINE", 0);
    aster_addC(aster_w_writefile, "WRITE-FILE", 0);
    aster_addC(aster_w_writeline, "WRITE-LINE", 0);
    aster_addConstant(0, "R/O");
    aster_addConstant(1, "W/O");
    aster_addConstant(2, "R/W");
    aster_addC(aster_w_argc, "ARGC", 0);
    aster_addC(aster_w_arg, "ARG", 0);
    aster_runString((char*)aster_bootstr);
}
