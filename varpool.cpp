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
    p->inMemCap = 20;           //TODO
    p->overall  = 0;
    p->inReg = (Var*)calloc(REG_CNT_ + p->inMemCap, sizeof(Var));
    if (p->inReg == NULL) {
        free(p->inReg);
        free(p);
        return NULL;
    }
    for (size_t i = 0; i < REG_CNT_; ++i) {
        p->inReg[i].reg = (REGISTER_)i;
        p->inReg[i].offset = -1;
        p->inReg[i].num = 0;
    }
    p->inMem = p->inReg + REG_CNT_;
    p->inReg[REG_NONE_].allocated = true;
    p->inReg[RAX].allocated = true;
    p->inReg[R10].allocated = true;
    p->inReg[RBX].allocated = true;
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

    res->allocated = true;
    res->temp = (nodeId == -1);
    res->nodeId = nodeId;
    ++p->overall;
    return res;
}


Var *varPool_free(varPool *p, Var *v)
{
    assert(p != NULL);
    assert(v->reg != REG_CNT_ && v->reg != RSP && v->reg != RBP && v->reg != RAX && v->reg != RBX && v->reg != R10);
    assert(v->temp);
    v->allocated = false;
    v->temp = false;
    --p->overall;
    return NULL;
}

void varPool_dump(varPool *p, FILE *out)
{
    assert(p != NULL);
    assert(out != NULL);

    fprintf(out, "varPool dump:\ninMemCnt = %zu\ninMemCap = %zu\noverall = %zu\n\n", p->inMemCnt, p->inMemCap, p->overall);
    for (size_t i = 0; i < REG_CNT_; ++i) {
        Var *v = p->inReg + i;
        if (v->reg != REG_CNT_ && v->reg != RSP && v->reg != RBP && v->reg != RAX && v->reg != RBX) {
            fprintf(stderr, "%s (%d)\t| offset = %zu\t| num = %zu\t| temp = %d\t| allocated = %d\t| nodeId = %zu\n", REGISTER_MSG[v->reg], v->reg, v->offset, v->num, v->temp, v->allocated, v->nodeId);
        }
    }
    for (size_t i = 0; i < p->inMemCap; ++i) {
        Var *v = p->inMem + i;
        if (v->reg != REG_CNT_ && v->reg != RSP && v->reg != RBP && v->reg != RAX && v->reg != RBX) {
            fprintf(stderr, "%s (%d)\t| offset = %zu\t| num = %zu\t| temp = %d\t| allocated = %d\t| nodeId = %zu\n", REGISTER_MSG[v->reg], v->reg, v->offset, v->num, v->temp, v->allocated, v->nodeId);
        }
    }
    fprintf(stderr, "\n\n\n");
}
