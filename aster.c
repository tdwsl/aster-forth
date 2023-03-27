#include "aster.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

char aster_dict[ASTER_DICTSZ];
int aster_stack[ASTER_STACKSZ];
int aster_rstack[ASTER_RSTACKSZ];
int aster_sp=0, aster_rsp=0, aster_pc=0, aster_here=0, aster_old=0;
struct aster_word aster_words[ASTER_WORDSSZ];
int aster_nwords=0;

void aster_addC(void (*fun)(void), const char *name, int flag)
{
    aster_words[aster_nwords++] = (struct aster_word)
    {
        name,
        flag|ASTER_C, 0,
        fun,
    };
}

struct aster_word *aster_findWord(const char *name)
{
    int i;
    for(i = aster_nwords-1; i >= 0; i--)
        if(!strcmp(name, aster_words[i].name))
            return &aster_words[i];
    return 0;
}

struct aster_word *aster_findC(void (*fun)(void))
{
    int i;
    for(i = aster_nwords-1; i >= 0; i--)
        if(aster_words[i].fun == fun)
            return &aster_words[i];
    return 0;
}

void aster_run()
{
    void (*fun)(void);
    while(fun = *(void (**)(void))(aster_dict+aster_pc))
    {
        /*printf(":%X\n", aster_pc);*/
        aster_pc += sizeof(void (*)(void));
        fun();
    }
}

int aster_num(char *s, int *n)
{
    int i;
    int base;
    int f = 0;
    if(*s == '-') { f = 1; s++; }
    if(*s == 0) return 0;
    base = 10;
    *n = 0;
    do {
        if(base <= 10) {
            if(*s >= '0' && *s < '0'+base)
                *n = *n * base + *s - '0';
            else return 0;
        } else {
            if(*s >= '0' && *s <= '9')
                *n = *n * base + *s - '0';
            else if(*s >= 'A' && *s < 'A'+base-10)
                *n = *n * base + *s - 'A' + 10;
            else return 0;
        }
    } while(*(++s));
    if(f) *n *= -1;
    return 1;
}

void aster_doToken(char *s)
{
    struct aster_word *w;
    int n;
    w = aster_findWord(s);
    if(w) {
        if(w->flag & ASTER_IMMEDIATE) {
            if(w->flag & ASTER_C) w->fun();
            else { aster_pc = w->addr; aster_run(); }
        } else if(w->flag & ASTER_C) {
            *(void (**)(void))(aster_dict+aster_here) = w->fun;
            aster_here += sizeof(void (*)(void))/sizeof(char);
        } else {
            *(void (**)(void))(aster_dict+aster_here) = aster_call;
            aster_here += sizeof(void (*)(void))/sizeof(char);
            *(int*)(aster_dict+aster_here) = w->addr;
            aster_here += sizeof(int)/sizeof(char);
        }
    } else if(aster_num(s, &n)) {
        *(void (**)(void))(aster_dict+aster_here) = aster_push;
        aster_here += sizeof(void (*)(void))/sizeof(char);
        *(int*)(aster_dict+aster_here) = n;
        aster_here += sizeof(int)/sizeof(char);
    } else { printf("%s ?\n", s); exit(1); }
}

void aster_print(int addr)
{
    void (*fun)(void);
    struct aster_word *w;
    while(addr < aster_here)
    {
        fun = *(void (**)(void))(aster_dict+addr);
        printf("%.8X ", addr);
        addr += sizeof(void (*)(void));
        if(fun == aster_jmp) {
            printf("jmp 0x%x", *(int*)(aster_dict+addr));
            addr += sizeof(int)/sizeof(char);
        } else if(fun == aster_jz) {
            printf("jz 0x%x", *(int*)(aster_dict+addr));
            addr += sizeof(int)/sizeof(char);
        } else if(fun == aster_call) {
            printf("call 0x%x", *(int*)(aster_dict+addr));
            addr += sizeof(int)/sizeof(char);
        } else if(fun == aster_push) {
            printf("push %d", *(int*)(aster_dict+addr));
            addr += sizeof(int)/sizeof(char);
        } else if(fun == aster_ret) {
            printf("ret");
        } else if(w = aster_findC(fun)) printf("%s", w->name);
        else printf("0x%x", fun);
        printf("\n");
    }
}
