#include "varpool.h"

int main()
{
    varPool *p = varPool_new(stderr);
    Var *v1 = varPool_alloc(p, 0);
    fprintf(stderr, "reg = %d | offset = %zu | temp = %d | allocated = %d\n", v1->reg, v1->offset, v1->temp, v1->allocated);

    for (size_t i = 0; i < 14; ++i) {
        v1 = varPool_alloc(p, i);
        fprintf(stderr, "reg = %d | offset = %zu | temp = %d | allocated = %d\n", v1->reg, v1->offset, v1->temp, v1->allocated);
    }
    for (size_t i = 11; i < 15; ++i) {
        p->inReg[i].allocated = false;
    }
    fprintf(stderr, "HERE!\n");
    fprintf(stderr, "inMemCap = %zu | inMemCnt = %zu | overall = %zu\n", p->inMemCap, p->inMemCnt, p->overall);
    for (size_t i = 0; i < 6; ++i) {
        v1 = varPool_alloc(p, -1);
        fprintf(stderr, "reg = %d | offset = %zu | temp = %d | allocated = %d\n", v1->reg, v1->offset, v1->temp, v1->allocated);
    }
    p->inMem[1].allocated = false;
    p->inMem[2].allocated = false;
    p->inMem[3].allocated = false;
    fprintf(stderr, "HERE!\n");
    fprintf(stderr, "inMemCap = %zu | inMemCnt = %zu | overall = %zu\n", p->inMemCap, p->inMemCnt, p->overall);
    for (size_t i = 0; i < 6; ++i) {
        v1 = varPool_alloc(p, i + 14);
        fprintf(stderr, "reg = %d | offset = %zu | temp = %d | allocated = %d\n", v1->reg, v1->offset, v1->temp, v1->allocated);
    }
    fprintf(stderr, "inMemCap = %zu | inMemCnt = %zu | overall = %zu\n", p->inMemCap, p->inMemCnt, p->overall);

    varPool_dump(p, stderr);
    p = varPool_delete(p);
}
