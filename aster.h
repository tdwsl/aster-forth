#ifndef ASTER_H
#define ASTER_H

#define ASTER_DICTSZ      245760
#define ASTER_MAXWORDS    8000
#define ASTER_BUFSZ       200
#define ASTER_LINEBUFSZ   560
#define ASTER_NAMEBUFSZ   72000
#define ASTER_NFILES      48
#define ASTER_MAXFUNS     900

#define ASTER_IMMEDIATE   1
#define ASTER_FUNCTION    2

#define ASTER_INTSZ       (sizeof(int)/sizeof(char))

#define ASTER_BASE        0
#define ASTER_STATUS      ASTER_INTSZ
#define ASTER_ARGC        ASTER_INTSZ*2
#define ASTER_START       ASTER_INTSZ*3

#define ASTER_LIT         0
#define ASTER_JMP         1
#define ASTER_JZ          2
#define ASTER_RET         3
#define ASTER_USER        4

extern unsigned char aster_dict[];
extern int aster_stack[];
extern int aster_rstack[];
extern unsigned char aster_rsp, aster_sp;
extern unsigned char aster_error;
extern unsigned char aster_usedArgs;
typedef void (*aster_fun)(void);
extern aster_fun aster_functions[];
extern int aster_nfunctions;

static const char *aster_sSU = "stack underflow !\n";
static const char *aster_sSO = "stack overflow !\n";
static const char *aster_sRU = "return stack underflow !\n";
static const char *aster_sRO = "return stack overflow !\n";
static const char *aster_sOB = "invalid memory address\n";

#define aster_sassert(S) if(aster_sp < (S)) { \
    sprintf(aster_buf, "%s", aster_sSU); aster_error = 1; return; }
#define aster_soassert(S) if(aster_sp < (S)) { \
    sprintf(aster_buf, "%s", aster_sSO); aster_error = 1; return; }

#define aster_rassert(S) if(aster_rsp < (S)) { \
    sprintf(aster_buf, "%s", aster_sRU); aster_error = 1; return; }
#define aster_roassert(S) if(aster_rsp < (S)) { \
    sprintf(aster_buf, "%s", aster_sRO); aster_error = 1; return; }

#define aster_bassert(A) if((A) < 0 || (A) >= ASTER_DICTSZ) { \
    sprintf(aster_buf, "%s", aster_sOB); aster_error = 1; return; }

#define aster_execute(A) aster_runAddr(A)

extern char aster_buf[];

void aster_init(int argc, char **args);
void aster_runFile(const char *filename);
void aster_runString(char *s);
void aster_runStdin();
void aster_runAddr(int pc);
void aster_addConstant(int v, const char *name);
void aster_addC(void (*fun)(void), const char *name, char flags);

#endif

