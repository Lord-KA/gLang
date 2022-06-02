#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

typedef enum {
    REG_NONE_ = 0,
    RAX,            // Service
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    RSP,            // Service
    RBP,            // Service
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    REG_CNT_
} REGISTER_;

typedef struct {
    FILE *out;
    bool inUse[REG_CNT_];
} regPool;

typedef struct {
    REGISTER_ reg;
    size_t offset;
    bool temp;
} Var;

regPool *regPool_new(FILE *out)
{
    assert(out != NULL);
    regPool *p = calloc(1, sizeof(regPool));
    assert(p != NULL);
    if (p == NULL)
        return p;
    p->out = out;
    p->inUse[RAX] = true;
    p->inUse[RSP] = true;
    p->inUse[RBP] = true;
    return p;
}

regPool *regPool_delete(regPool *p)
{
    assert(p != NULL);
    free(p);
    return NULL;
}

Var regPool_alloc(regPool *p)
{
    assert(p != NULL);
    size_t i;
    for (i = 1; i < REG_CNT_ && p->inUse[i]; ++i);

    Var res = {};
    if (i == REG_CNT_) {
        res.offset = 0x1234;            //TODO calc offset here
    } else {
        res.reg = i;
        res.offset = -1;
    }
    p->inUse[i] = true;
    return res;
}

void regPool_free(regPool *p, Var v)
{
    assert(p != NULL);
    assert(v.reg != REG_CNT_ && v.reg != RAX && v.reg != RSP && v.reg != RBP && v.reg != REG_NONE_);
    if (v.reg == REG_NONE_) {
        //TODO free reg offset?
    } else {
        p->inUse[v.reg] = false;
    }
}
