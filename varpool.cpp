#include "varpool.h"

varPool *varPool_new(FILE *out)
{
    assert(out != NULL);
    varPool *p = (varPool*)calloc(1, sizeof(varPool));
    assert(p != NULL);
    if (p == NULL)
        return p;
    p->out = out;
    p->inMemCnt = 0;
    p->inMemCap = 2;
    p->overall  = 0;
    p->inReg = (Var*)calloc(REG_CNT_ + p->inMemCap, sizeof(Var));
    if (p->inReg == NULL) {
        free(p->inReg);
        free(p);
        return NULL;
    }
    p->inMem = p->inReg + REG_CNT_;
    p->inReg[REG_NONE_].allocated = true;
    p->inReg[RAX].allocated = true;
    p->inReg[RBX].allocated = true;
    p->inReg[RCX].allocated = true;
    p->inReg[RSP].allocated = true;
    p->inReg[RBP].allocated = true;
    return p;
}

varPool *varPool_delete(varPool *p)
{
    assert(p != NULL);
    free(p->inReg);
    free(p);
    return NULL;
}

Var *varPool_allocInMem(varPool *p, size_t nodeId)
{
    size_t i;
    for (i = 0; i < p->inMemCap; ++i) {
        if (!p->inMem[i].allocated)
            break;
    }
    if (i == p->inMemCap) {
        void *tmp = realloc(p->inReg, (REG_CNT_ + p->inMemCap * 2) * sizeof(Var));
        if (tmp == NULL) {
            return NULL;
        }
        p->inReg = (Var*)tmp;
        p->inMem = p->inReg + REG_CNT_;
        memset(p->inMem + p->inMemCap, 0, p->inMemCap * sizeof(Var));
        p->inMemCap *= 2;
    }
    if (i + 1 > p->inMemCnt)
        p->inMemCnt = i + 1;
    Var *res = p->inMem + i;
    assert(!res->allocated);
    res->offset = i;
    res->reg = REG_NONE_;
    res->allocated = true;
    res->temp = (nodeId == -1);
    res->nodeId = nodeId;
    ++p->overall;

    return res;
}

Var *varPool_alloc(varPool *p, size_t nodeId)
{
    assert(p != NULL);
    size_t i;
    for (i = 1; i < REG_CNT_ && p->inReg[i].allocated; ++i);

    if (i == REG_CNT_)
        return varPool_allocInMem(p, nodeId);

    Var *res = p->inReg + i;
    assert(!res->allocated);

    res->reg = (REGISTER_)i;
    res->offset = -1;
    res->allocated = true;
    res->temp = (nodeId == -1);
    res->nodeId = nodeId;
    ++p->overall;
    return res;
}


Var *varPool_free(varPool *p, Var *v)
{
    assert(p != NULL);
    assert(v->reg != REG_CNT_ && v->reg > RCX && v->reg != RSP && v->reg != RBP);
    assert(v->temp);
    v->allocated = false;
    --p->overall;
    return NULL;
}
