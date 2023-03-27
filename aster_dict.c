#include "aster.h"
#include <stdio.h>

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
    aster_pc = aster_rstack[--aster_rsp];
}

void aster_jz()
{
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
    aster_rstack[aster_rsp++] = aster_stack[--aster_sp];
}

void aster_w_rpl()
{
    aster_stack[aster_sp++] = aster_rstack[--aster_rsp];
}

void aster_w_add()
{
    aster_stack[aster_sp-2] += aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_sub()
{
    aster_stack[aster_sp-2] -= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_mul()
{
    aster_stack[aster_sp-2] *= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_div()
{
    aster_stack[aster_sp-2] /= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_mod()
{
    aster_stack[aster_sp-2] %= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_divmod()
{
    int d;
    d = aster_stack[aster_sp-2] / aster_stack[aster_sp-1];
    aster_stack[aster_sp-2] %= aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = d;
}

void aster_w_inc()
{
    aster_stack[aster_sp-1]++;
}

void aster_w_dec()
{
    aster_stack[aster_sp-1]--;
}

void aster_w_shr()
{
    aster_stack[aster_sp-1] >>= 1;
}

void aster_w_shl()
{
    aster_stack[aster_sp-1] <<= 1;
}

void aster_w_prnum()
{
    printf("%d ", aster_stack[aster_sp-1]);
}

void aster_w_emit()
{
    printf("%c", aster_stack[aster_sp-1]);
}

void aster_w_space()
{
    printf(" ");
}

void aster_w_cr()
{
    printf("\n");
}

void aster_w_prstack()
{
    int i;
    for(i = 0; i < aster_sp; i++)
        printf("%d ", aster_stack[i]);
    printf("\n");
}

void aster_w_if()
{
    *(void (**)(void))(aster_dict+aster_here) = aster_jz;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = 0;
    aster_rstack[aster_rsp++] = aster_here;
    aster_here += sizeof(int)/sizeof(char);
}

void aster_w_then()
{
    *(int*)(aster_dict+aster_rstack[--aster_rsp]) = aster_here;
}

void aster_w_else()
{
    *(void (**)(void))(aster_dict+aster_here) = aster_jmp;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) = 0;
    aster_here += sizeof(int)/sizeof(char);
    *(int*)(aster_dict+aster_rstack[aster_rsp-1]) = aster_here;
    aster_rstack[aster_rsp-1] = aster_here-sizeof(int)/sizeof(char);
}

void aster_do()
{
    aster_rstack[aster_rsp++] = aster_stack[--aster_sp];
    aster_rstack[aster_rsp++] = aster_stack[--aster_sp];
}

void aster_w_unloop()
{
    aster_rsp -= 2;
}

void aster_w_do()
{
    *(void (**)(void))(aster_dict+aster_here) = aster_do;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    aster_rstack[aster_rsp++] = aster_here;
    aster_rstack[aster_rsp++] = 0;
}

void aster_loop()
{
    aster_rstack[aster_rsp-2]++;
    aster_stack[aster_sp++] = (aster_rstack[aster_rsp-2] == aster_rstack[aster_rsp-1]);
}

void aster_plusloop()
{
    int o;
    o = aster_rstack[aster_rsp-2];
    aster_rstack[aster_rsp-2] += aster_stack[--aster_sp];
    aster_stack[aster_sp++] =
        ((aster_rstack[aster_rsp-2] >= aster_rstack[aster_rsp-1]) !=
        (o >= aster_rstack[aster_rsp-1]));
}

void aster_w_loop()
{
    int i;
    *(void (**)(void))(aster_dict+aster_here) = aster_loop;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(void (**)(void))(aster_dict+aster_here) = aster_jz;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) =
        aster_rstack[aster_rsp-2-aster_rstack[aster_rsp-1]];
    aster_here += sizeof(int)/sizeof(char);
    for(i = 0; i < aster_rstack[aster_rsp-1]; i++)
        *(int*)(aster_dict+aster_rstack[aster_rsp-2-aster_rstack[aster_rsp-1]+i]) =
            aster_here;
    aster_rsp -= aster_rstack[aster_rsp-1]+2;
    *(void (**)(void))(aster_dict+aster_here) = aster_w_unloop;
    aster_here += sizeof(void (*)(void))/sizeof(char);
}

void aster_w_plusloop()
{
    int i;
    *(void (**)(void))(aster_dict+aster_here) = aster_plusloop;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(void (**)(void))(aster_dict+aster_here) = aster_jz;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) =
        aster_rstack[aster_rsp-2-aster_rstack[aster_rsp-1]];
    aster_here += sizeof(int)/sizeof(char);
    for(i = 0; i < aster_rstack[aster_rsp-1]; i++)
        *(int*)(aster_dict+aster_rstack[aster_rsp-2-aster_rstack[aster_rsp-1]+i]) =
            aster_here;
    aster_rsp -= aster_rstack[aster_rsp-1]+2;
    *(void (**)(void))(aster_dict+aster_here) = aster_w_unloop;
    aster_here += sizeof(void (*)(void))/sizeof(char);
}

void aster_w_leave()
{
    *(void (**)(void))(aster_dict+aster_here) = aster_jmp;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    aster_stack[aster_sp] = aster_stack[aster_sp-1]+1;
    aster_sp++;
    aster_stack[aster_sp-2] = aster_here;
    aster_here += sizeof(int)/sizeof(char);
}

void aster_w_i()
{
    aster_stack[aster_sp++] = aster_rstack[aster_rsp-2];
}

void aster_w_j()
{
    aster_stack[aster_sp++] = aster_rstack[aster_rsp-4];
}

void aster_init()
{
    aster_addC(aster_w_rph, ">R", 0);
    aster_addC(aster_w_rpl, "R>", 0);
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
    aster_addC(aster_w_prnum, ".", 0);
    aster_addC(aster_w_prstack, ".S", 0);
    aster_addC(aster_w_space, "SPACE", 0);
    aster_addC(aster_w_cr, "CR", 0);
    aster_addC(aster_w_if, "IF", ASTER_IMMEDIATE);
    aster_addC(aster_w_then, "THEN", ASTER_IMMEDIATE);
    aster_addC(aster_w_else, "ELSE", ASTER_IMMEDIATE);
    aster_addC(aster_w_do, "DO", ASTER_IMMEDIATE);
    aster_addC(aster_w_loop, "LOOP", ASTER_IMMEDIATE);
    aster_addC(aster_w_plusloop, "+LOOP", ASTER_IMMEDIATE);
    aster_addC(aster_w_unloop, "UNLOOP", ASTER_IMMEDIATE);
    aster_addC(aster_w_leave, "LEAVE", ASTER_IMMEDIATE);
    aster_addC(aster_w_i, "I", 0);
    aster_addC(aster_w_j, "J", 0);
}