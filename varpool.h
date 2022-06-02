#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>


typedef enum {
    REG_NONE_ = 0,
    RAX,            // Service
    RBX,            // Service
    RCX,            // Service
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
} varPool;

typedef struct {
    REGISTER_ reg;
    size_t offset;
    bool temp;
    size_t varId;
} Var;

varPool *varPool_new(FILE *out)
{
    assert(out != NULL);
    varPool *p = (varPool*)calloc(1, sizeof(varPool));
    assert(p != NULL);
    if (p == NULL)
        return p;
    p->out = out;
    p->inUse[REG_NONE_] = true;
    p->inUse[RAX] = true;
    p->inUse[RBX] = true;
    p->inUse[RCX] = true;
    p->inUse[RSP] = true;
    p->inUse[RBP] = true;
    return p;
}

varPool *varPool_delete(varPool *p)
{
    assert(p != NULL);
    free(p);
    return NULL;
}

Var varPool_alloc(varPool *p)
{
    assert(p != NULL);
    size_t i;
    for (i = 1; i < REG_CNT_ && p->inUse[i]; ++i);

    Var res = {};
    if (i == REG_CNT_) {
        res.offset = 0x1234;            //TODO calc offset here
    } else {
        res.reg = (REGISTER_)i;
        res.offset = -1;
    }
    p->inUse[i] = true;
    return res;
}

void varPool_free(varPool *p, Var v)
{
    assert(p != NULL);
    assert(v.reg != REG_CNT_ && v.reg > RCX && v.reg != RSP && v.reg != RBP);
    if (v.reg == REG_NONE_) {
        //TODO free reg offset?
    } else {
        p->inUse[v.reg] = false;
    }
}
