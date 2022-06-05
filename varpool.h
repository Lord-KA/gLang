#ifndef VARPOOL_H
#define VARPOOL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>


typedef enum {
    REG_NONE_ = 0,

    RDI,            // Registers for passing args with calling convention
    RSI,
    RDX,
    RCX,
    R8,
    R9,

    RBX,            // Reserved     // Registers preserved
    RSP,            // Service
    RBP,            // Service
    R12,
    R13,
    R14,
    R15,

    RAX,            // Reserved
    R10,            // Reserved
    R11,

    REG_CNT_
} REGISTER_;

static const char REGISTER_MSG[REG_CNT_][10] = {
    "REG_NONE",
    "RDI",
    "RSI",
    "RDX",
    "RCX",
    "R8",
    "R9",
    "RBX",            // Reserved     // Registers preserved
    "RSP",            // Service
    "RBP",            // Service
    "R12",
    "R13",
    "R14",
    "R15",
    "RAX",            // Reserved
    "R10",            // Reserved
    "R11",
};

typedef struct {
    REGISTER_ reg;
    size_t offset;
    long num;
    bool temp;
    bool allocated;
    size_t nodeId;
} Var;

typedef struct {
    FILE *out;
    Var *inReg;
    Var *inMem;
    size_t inMemCap;
    size_t inMemCnt;
    size_t overall;
} varPool;


varPool *varPool_new(FILE *out);

varPool *varPool_delete(varPool *p);

Var *varPool_allocInMem(varPool *p, size_t nodeId);

Var *varPool_alloc(varPool *p, size_t nodeId);

Var *varPool_free(varPool *p, Var *v);

void varPool_dump(varPool *p, FILE *out);

#endif
