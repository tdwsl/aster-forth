#include "aster.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
    assert(aster_rsp);
    aster_pc = aster_rstack[--aster_rsp];
}

void aster_jz()
{
    assert(aster_sp);
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
    assert(aster_sp);
    aster_rstack[aster_rsp++] = aster_stack[--aster_sp];
}

void aster_w_rpl()
{
    assert(aster_rsp);
    aster_stack[aster_sp++] = aster_rstack[--aster_rsp];
}

void aster_w_dup()
{
    assert(aster_sp);
    aster_stack[aster_sp] = aster_stack[aster_sp-1];
    aster_sp++;
}

void aster_w_drop()
{
    assert(aster_sp);
    aster_sp--;
}

void aster_w_over()
{
    assert(aster_sp >= 2);
    aster_stack[aster_sp] = aster_stack[aster_sp-2];
    aster_sp++;
}

void aster_w_swap()
{
    int i;
    assert(aster_sp >= 2);
    i = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = i;
}

void aster_w_nip()
{
    assert(aster_sp >= 2);
    aster_sp--;
    aster_stack[aster_sp-1] = aster_stack[aster_sp];
}

void aster_w_tuck()
{
    assert(aster_sp >= 2);
    aster_stack[aster_sp] = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp];
    aster_sp++;
}

void aster_w_rot()
{
    int i;
    assert(aster_sp >= 3);
    i = aster_stack[aster_sp-3];
    aster_stack[aster_sp-3] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = i;
}

void aster_w_mrot()
{
    int i;
    assert(aster_sp >= 3);
    i = aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-3];
    aster_stack[aster_sp-3] = i;
}

void aster_w_pick()
{
    assert(aster_sp > aster_stack[aster_sp-1]);
    aster_stack[aster_sp-1] =
        aster_stack[aster_sp-2-aster_stack[aster_sp-1]];
}

void aster_w_2dup()
{
    assert(aster_sp >= 2);
    aster_stack[aster_sp] = aster_stack[aster_sp-2];
    aster_stack[aster_sp+1] = aster_stack[aster_sp-1];
    aster_sp += 2;
}

void aster_w_2drop()
{
    assert(aster_sp >= 2);
    aster_sp -= 2;
}

void aster_w_2over()
{
    assert(aster_sp >= 4);
    aster_stack[aster_sp] = aster_stack[aster_sp-4];
    aster_stack[aster_sp+1] = aster_stack[aster_sp-3];
    aster_sp += 2;
}

void aster_w_2swap()
{
    int i1, i2;
    assert(aster_sp >= 4);
    i1 = aster_stack[aster_sp-2];
    i2 = aster_stack[aster_sp-1];
    aster_stack[aster_sp-2] = aster_stack[aster_sp-4];
    aster_stack[aster_sp-1] = aster_stack[aster_sp-3];
    aster_stack[aster_sp-4] = i1;
    aster_stack[aster_sp-3] = i2;
}

void aster_w_2nip()
{
    assert(aster_sp >= 4);
    aster_stack[aster_sp-4] = aster_stack[aster_sp-2];
    aster_stack[aster_sp-3] = aster_stack[aster_sp-1];
    aster_sp -= 2;
}

void aster_w_2tuck()
{
    assert(aster_sp >= 4);
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
    assert(aster_sp >= 2);
    aster_stack[aster_sp-2] += aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_sub()
{
    assert(aster_sp >= 2);
    aster_stack[aster_sp-2] -= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_mul()
{
    assert(aster_sp >= 2);
    aster_stack[aster_sp-2] *= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_div()
{
    assert(aster_sp >= 2);
    aster_stack[aster_sp-2] /= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_mod()
{
    assert(aster_sp >= 2);
    aster_stack[aster_sp-2] %= aster_stack[aster_sp-1];
    aster_sp--;
}

void aster_w_divmod()
{
    int d;
    assert(aster_sp >= 2);
    d = aster_stack[aster_sp-2] / aster_stack[aster_sp-1];
    aster_stack[aster_sp-2] %= aster_stack[aster_sp-1];
    aster_stack[aster_sp-1] = d;
}

void aster_w_inc()
{
    assert(aster_sp);
    aster_stack[aster_sp-1]++;
}

void aster_w_dec()
{
    assert(aster_sp);
    aster_stack[aster_sp-1]--;
}

void aster_w_shr()
{
    assert(aster_sp);
    aster_stack[aster_sp-1] >>= 1;
}

void aster_w_shl()
{
    assert(aster_sp);
    aster_stack[aster_sp-1] <<= 1;
}

void aster_w_prnum()
{
    assert(aster_sp);
    printf("%d ", aster_stack[--aster_sp]);
}

void aster_w_emit()
{
    assert(aster_sp);
    printf("%c", aster_stack[--aster_sp]);
}

void aster_w_space()
{
    printf(" ");
}

void aster_w_cr()
{
    printf("\n");
}

void aster_w_words()
{
    int i, x;
    x = 0;
    for(i = aster_nwords-1; i >= 0; i--)
    {
        x += strlen(aster_words[i].name)+1;
        if(x >= 64) { printf("\n"); x = strlen(aster_words[i].name)+1; }
        printf("%s ", aster_words[i].name);
    }
    printf("\n");
}

void aster_w_prstack()
{
    int i;
    printf("<%d> ", aster_sp);
    for(i = 0; i < aster_sp; i++)
        printf("%d ", aster_stack[i]);
    printf("\n");
}

void aster_w_if()
{
    assert(aster_sp);
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
    assert(aster_sp >= 2);
    aster_rstack[aster_rsp++] = aster_stack[--aster_sp];
    aster_rstack[aster_rsp++] = aster_stack[--aster_sp];
}

void aster_w_unloop()
{
    assert(aster_rsp >= 2);
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
    aster_stack[aster_sp++] =
        (aster_rstack[aster_rsp-2] == aster_rstack[aster_rsp-1]);
}

void aster_plusloop()
{
    int o;
    assert(aster_sp);
    o = aster_rstack[aster_rsp-2];
    aster_rstack[aster_rsp-2] += aster_stack[--aster_sp];
    aster_stack[aster_sp++] =
        ((aster_rstack[aster_rsp-2] >= aster_rstack[aster_rsp-1]) !=
        (o >= aster_rstack[aster_rsp-1]));
}

void aster_w_loop()
{
    int i;
    assert(aster_rsp >= 2);
    *(void (**)(void))(aster_dict+aster_here) = aster_loop;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(void (**)(void))(aster_dict+aster_here) = aster_jz;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) =
        aster_rstack[aster_rsp-2-aster_rstack[aster_rsp-1]];
    aster_here += sizeof(int)/sizeof(char);
    for(i = 0; i < aster_rstack[aster_rsp-1]; i++)
        *(int*)(aster_dict+
            aster_rstack[aster_rsp-2-aster_rstack[aster_rsp-1]+i]) =
                aster_here;
    aster_rsp -= aster_rstack[aster_rsp-1]+2;
    *(void (**)(void))(aster_dict+aster_here) = aster_w_unloop;
    aster_here += sizeof(void (*)(void))/sizeof(char);
}

void aster_w_plusloop()
{
    int i;
    assert(aster_rsp >= 2);
    *(void (**)(void))(aster_dict+aster_here) = aster_plusloop;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(void (**)(void))(aster_dict+aster_here) = aster_jz;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    *(int*)(aster_dict+aster_here) =
        aster_rstack[aster_rsp-2-aster_rstack[aster_rsp-1]];
    aster_here += sizeof(int)/sizeof(char);
    for(i = 0; i < aster_rstack[aster_rsp-1]; i++)
        *(int*)(aster_dict+
            aster_rstack[aster_rsp-2-aster_rstack[aster_rsp-1]+i]) =
                aster_here;
    aster_rsp -= aster_rstack[aster_rsp-1]+2;
    *(void (**)(void))(aster_dict+aster_here) = aster_w_unloop;
    aster_here += sizeof(void (*)(void))/sizeof(char);
}

void aster_w_leave()
{
    assert(aster_rsp >= 2);
    *(void (**)(void))(aster_dict+aster_here) = aster_jmp;
    aster_here += sizeof(void (*)(void))/sizeof(char);
    aster_stack[aster_sp] = aster_stack[aster_sp-1]+1;
    aster_sp++;
    aster_stack[aster_sp-2] = aster_here;
    aster_here += sizeof(int)/sizeof(char);
}

void aster_w_i()
{
    assert(aster_rsp >= 2);
    aster_stack[aster_sp++] = aster_rstack[aster_rsp-2];
}

void aster_w_j()
{
    assert(aster_rsp >= 4);
    aster_stack[aster_sp++] = aster_rstack[aster_rsp-4];
}

void aster_getNext(char *s1)
{
    char c;
    char *s;
    while((c = aster_nextChar()) <= ' ' && c);
    assert(c);
    s = s1;
    *(s++) = c;
    while((c = aster_nextChar()) > ' ') *(s++) = c;
    *s = 0;
    for(s = s1; *s; s++)
        if(*s >= 'a' && *s <= 'z') *s += 'A'-'a';
}

void aster_w_col()
{
    assert(aster_status == ASTER_RUN);
    *(void (**)(void))(aster_dict+aster_here) = 0;
    aster_pc = aster_old;
    aster_run();
    aster_here = aster_old;
    aster_status = ASTER_WORD;
    aster_getNext(aster_nextName);
}

void aster_w_semi()
{
    assert(aster_status == ASTER_WORD);
    assert(aster_rsp == 0);
    *(void (**)(void))(aster_dict+aster_here) = aster_ret;
    aster_here += sizeof(void (**)(void))/sizeof(char);
    aster_words[aster_nwords++] = (struct aster_word)
    {
        aster_nextName,
        0, aster_old,
        0,
    };
    aster_nextName += strlen(aster_nextName)+1;
    aster_status = ASTER_RUN;
    aster_print(aster_old, aster_here);
    aster_old = aster_here;
}

void aster_init()
{
    aster_addC(aster_w_rph, ">R", 0);
    aster_addC(aster_w_rpl, "R>", 0);
    aster_addC(aster_w_dup, "DUP", 0);
    aster_addC(aster_w_drop, "DROP", 0);
    aster_addC(aster_w_over, "OVER", 0);
    aster_addC(aster_w_swap, "SWAP", 0);
    aster_addC(aster_w_nip, "NIP", 0);
    aster_addC(aster_w_tuck, "TUCK", 0);
    aster_addC(aster_w_rot, "ROT", 0);
    aster_addC(aster_w_mrot, "-ROT", 0);
    aster_addC(aster_w_pick, "PICK", 0);
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
    aster_addC(aster_w_prnum, ".", 0);
    aster_addC(aster_w_prstack, ".S", 0);
    aster_addC(aster_w_space, "SPACE", 0);
    aster_addC(aster_w_cr, "CR", 0);
    aster_addC(aster_w_words, "WORDS", 0);
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
    aster_addC(aster_w_col, ":", ASTER_IMMEDIATE);
    aster_addC(aster_w_semi, ";", ASTER_IMMEDIATE);
}
