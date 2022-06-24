#ifndef VARPOOL_H
#define VARPOOL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>


typedef enum {
    REG_NONE_ = 0,

    RDI,            // Registers for passing args with calling convention
    RSI,
    RDX,
    RCX,
    R8,
    R9,

    RBX,            // Registers preserved
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

typedef enum {
    BASIC = 0,
    EXTENDED,
    SERVICE,
} REGISTER_TYPE_;

static const REGISTER_TYPE_ REG_TYPE[REG_CNT_] = {
    SERVICE,      // REG_NONE_
    BASIC,        // RDI
    BASIC,
    BASIC,
    BASIC,
    EXTENDED,     // R8
    EXTENDED,     // R9
    BASIC,        // RBX
    BASIC,        // RSP
    BASIC,        // RBP
    EXTENDED,
    EXTENDED,
    EXTENDED,
    EXTENDED,
    BASIC,         // RAX
    EXTENDED,      // R10
    EXTENDED,      // R11
};



static const char REGISTER_MSG[REG_CNT_][10] = {
    "REG_NONE",
    "RDI",
    "RSI",
    "RDX",
    "RCX",
    "R8",
    "R9",
    "RBX",            // Registers preserved
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

static const uint8_t REG_CODE[REG_CNT_] = {
    0xFF,
    0b0111,
    0b0110,
    0b0010,
    0b0001,
    0b0000,
    0b0001,
    0b0011,
    0b0100,
    0b0101,
    0b0100,
    0b0101,
    0b0110,
    0b0111,
    0b0000,
    0b0010,
    0b0011,
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


varPool *varPool_new();

varPool *varPool_delete(varPool *p);

Var *varPool_allocInMem(varPool *p, size_t nodeId);

Var *varPool_alloc(varPool *p, size_t nodeId);

Var *varPool_free(varPool *p, Var *v);

void varPool_dump(varPool *p, FILE *out);

#endif
