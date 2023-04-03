#include "aster.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

char aster_dict[ASTER_DICTSZ];
int aster_stack[ASTER_STACKSZ];
int aster_rstack[ASTER_RSTACKSZ];
int aster_sp=0, aster_rsp=0, aster_pc=0;
int aster_here=ASTER_DICTSTART;
int aster_stringPtr=ASTER_STRINGSTART;
struct aster_word aster_words[ASTER_WORDSSZ];
int aster_nwords=0;
char (*aster_nextChar)(void) = 0;
char *aster_string;
FILE *aster_fp;
char aster_nameBuf[ASTER_NAMEBUFSZ];
char *aster_nextName = aster_nameBuf;
int aster_args[ASTER_ARGSSZ];

void aster_initArgs(int argc, char **args)
{
    int i, i1;
    i1 = (argc > 1);
    for(i = i1; i < argc; i++)
    {
        aster_args[i-i1] = aster_stringPtr;
        strcpy(aster_dict+aster_stringPtr, args[i]);
        aster_stringPtr += strlen(aster_dict+aster_stringPtr)+1;
    }
    *(int*)(aster_dict+ASTER_ARGC) = argc-i1;
}

void aster_defEmit(int c)
{
    printf("%c", c);
}

int aster_defKey()
{
    return fgetc(stdin);
}

void (*aster_emit)(int) = aster_defEmit;
int (*aster_key)(void) = aster_defKey;

int aster_printf(const char *s, ...)
{
    int i, r;
    char buf[400];
    va_list valist;
    va_start(valist, s);
    r = vsprintf(buf, s, valist);
    va_end(valist);
    for(i = 0; buf[i]; i++)
        aster_emit(buf[i]);
    return r;
}

void aster_addC(void (*fun)(void), const char *name, int flag)
{
    aster_words[aster_nwords++] = (struct aster_word)
    {
        name,
        flag|ASTER_C, 0, 0,
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
        /*aster_w_prstack();
        aster_print(aster_pc, aster_pc+sizeof(void (*)(void))/sizeof(char));*/
        aster_pc += sizeof(void (*)(void))/sizeof(char);;
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
    base = *(int*)(aster_dict+ASTER_BASE);
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

void aster_err()
{
    if(*(int*)(aster_dict+ASTER_STATE) != ASTER_RUN) {
        aster_here = aster_words[aster_nwords-1].addr;
        *(int*)(aster_dict+ASTER_STATE) = ASTER_RUN;
    }
    aster_pc = ASTER_RET;
    aster_rsp = 0;
    aster_sp = 0;
    while(aster_nextChar());
    aster_printf("error\n");
}

void aster_doToken(char *s)
{
    struct aster_word *w;
    int n;
    w = aster_findWord(s);
    if(*(int*)(aster_dict+ASTER_STATE) == ASTER_RUN) {
        if(w) {
            if(w->flag & ASTER_COMPILEONLY) {
                aster_printf("%s is compile-only\n", s);
                aster_err();
                return;
            }
            if(w->flag & ASTER_C) w->fun();
            else {
                aster_rstack[aster_rsp++] = ASTER_RET;
                aster_pc = w->addr;
                aster_run();
            }
        } else if(aster_num(s, &n))
            aster_stack[aster_sp++] = n;
        else { aster_printf("%s ?\n", s); aster_err(); }
    } else if(w) {
        if(w->flag & ASTER_IMMEDIATE) {
            if(w->flag & ASTER_C) w->fun();
            else {
                aster_rstack[aster_rsp++] = ASTER_RET;
                aster_pc = w->addr;
                aster_run();
            }
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
    } else { aster_printf("%s ?\n", s); aster_err(); }
}

void aster_print(int addr, int addr2)
{
    void (*fun)(void);
    struct aster_word *w;
    while(addr < addr2)
    {
        fun = *(void (**)(void))(aster_dict+addr);
        aster_printf("%.8X ", addr);
        addr += sizeof(void (*)(void));
        if(fun == aster_jmp) {
            aster_printf("jmp 0x%x", *(int*)(aster_dict+addr));
            addr += sizeof(int)/sizeof(char);
        } else if(fun == aster_jz) {
            aster_printf("jz 0x%x", *(int*)(aster_dict+addr));
            addr += sizeof(int)/sizeof(char);
        } else if(fun == aster_call) {
            aster_printf("call 0x%x", *(int*)(aster_dict+addr));
            addr += sizeof(int)/sizeof(char);
        } else if(fun == aster_push) {
            aster_printf("push %d", *(int*)(aster_dict+addr));
            addr += sizeof(int)/sizeof(char);
        } else if(fun == aster_ret) {
            aster_printf("ret");
        } else if(w = aster_findC(fun)) aster_printf("%s", w->name);
        else aster_printf("0x%x", (unsigned)(size_t)fun);
        aster_printf("\n");
    }
}

void aster_runAll()
{
    char buf[512];
    char *s;
    s = buf;
    char c;
    for(;;)
    {
        *s = aster_nextChar();
        if(*s <= ' ') {
            c = *s;
            if(s != buf) {
                *s = 0;
                aster_doToken(buf);
                s = buf;
            }
            if(c == 0) return;
        } else {
            if(*s >= 'a' && *s <= 'z') *s += 'A'-'a';
            s++;
        }
    }
}

char aster_nextChar_string()
{
    if(!(*aster_string)) return 0;
    return *(aster_string++);
}

void aster_runString(char *s)
{
    char (*old_nextChar)(void);
    char *old_string;
    old_nextChar = aster_nextChar;
    old_string = aster_string;
    aster_nextChar = aster_nextChar_string;
    aster_string = s;
    aster_runAll();
    aster_string = old_string;
    aster_nextChar = old_nextChar;
}

char aster_nextChar_file()
{
    if(feof(aster_fp)) return 0;
    return fgetc(aster_fp);
}

void aster_runFile(const char *filename)
{
    char (*old_nextChar)(void);
    FILE *old_fp;
    old_nextChar = aster_nextChar;
    old_fp = aster_fp;
    aster_fp = fopen(filename, "r");
    if(!aster_fp) {
        aster_printf("failed to open %s\n", filename);
        aster_err();
        return;
    }
    aster_nextChar = aster_nextChar_file;
    aster_runAll();
    aster_fp = old_fp;
    aster_nextChar = old_nextChar;
}

int aster_accept(char *s, int max)
{
    int i, c;
    for(i = 0;;)
    {
        c = aster_key();
        if(c==127||c=='\b') {
            if(i) { i--; s[i] = 0; aster_printf("\b \b"); }
            continue;
        }
        if(c == '\n'||!c) return i;
#ifdef ASTER_NCURSES
        aster_emit(c);
#endif
        if(i <= max) s[i++] = c;
    }
}

void aster_runPrompt()
{
    char buf[512];
    for(;;)
    {
        if(*(int*)(aster_dict+ASTER_STATE) != ASTER_WORD)
            aster_printf("  ok\n");
        else aster_printf("  compiled\n");
        buf[aster_accept(buf, 512-1)] = 0;
#ifdef ASTER_NCURSES
        aster_emit(' ');
#endif
        aster_runString(buf);
    }
}

