#ifndef ASTER_H
#define ASTER_H

extern int aster_stack[];
extern int aster_rstack[];
extern unsigned char aster_rsp, aster_sp;
extern unsigned char aster_error;

extern const char *aster_sSU, *aster_sSO, *aster_sRU, *aster_sRO;

#define aster_sassert(S) if(aster_sp < (S)) { \
    printf("stack underflow !\n"); aster_error = 1; }
#define aster_soassert(S) if(aster_sp < (S)) { \
    printf("stack overflow !\n"); aster_error = 1; }

#define aster_rassert(S) if(aster_rsp < (S)) { \
    printf("return stack underflow !\n"); aster_error = 1; }
#define aster_roassert(S) if(aster_rsp < (S)) { \
    printf("return stack overflow !\n"); aster_error = 1; }

void aster_init(int argc, char **args);
void aster_runFile(const char *filename);
void aster_runString(char *s);
void aster_runStdin();
void aster_addConstant(int v, const char *name);
void aster_addC(void (*fun)(void), const char *name, char flags);

#endif

