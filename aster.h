#ifndef ASTER_H
#define ASTER_H

#include <stdio.h>

#define ASTER_DICTSZ 4194304
#define ASTER_STACKSZ 128
#define ASTER_RSTACKSZ 128
#define ASTER_WORDSSZ 5999
#define ASTER_NAMEBUFSZ 4096

enum
{
    ASTER_C = 1,
    ASTER_IMMEDIATE = 2,
};

enum
{
    ASTER_RUN,
    ASTER_WORD,
};

extern char aster_dict[ASTER_DICTSZ];
extern int aster_stack[ASTER_STACKSZ];
extern int aster_rstack[ASTER_RSTACKSZ];
extern int aster_sp, aster_rsp, aster_pc, aster_here, aster_old;
extern int aster_status;

struct aster_word
{
    const char *name;
    int flag, addr;
    short size;
    void (*fun)(void);
};

extern struct aster_word aster_words[ASTER_WORDSSZ];
extern int aster_nwords;
extern char (*aster_nextChar)(void);
extern char *aster_string;
extern FILE *aster_fp;
extern char aster_nameBuf[ASTER_NAMEBUFSZ];
extern char *aster_nextName;

void aster_init();
void aster_addCore();
void aster_addC(void (*fun)(void), const char *name, int flag);
struct aster_word *aster_findWord(const char *name);
struct aster_word *aster_findC(void (*fun)(void));
void aster_print(int addr, int addr2);
void aster_run();
void aster_runAll();
void aster_runString(char *s);
void aster_runFile(const char *filename);
void aster_runPrompt();

void aster_call();
void aster_ret();
void aster_jmp();
void aster_jz();
void aster_push();

#endif
