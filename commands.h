#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdlib.h>
#include "varpool.h"

typedef enum {
    OP_NONE_ = 0,
    #define OP_DEF(name, ...) name,
        #include "opcodes.h"
    #undef OP_DEF
    OUT,
    POW,
    EXIT,
    LABLE,
    OP_CNT_
} OPCODE_;

static const size_t OPCODE_ARGS[OP_CNT_] = {
    #define OP_DEF(name, args, ...) args,
        #include "opcodes.h"
    #undef OP_DEF
    1,
    2,
    0,
    0,
};

static const char OPCODE_MSG[OP_CNT_][10] = {
    #define OP_DEF(name, ...) #name ,
        #include "opcodes.h"
    #undef OP_DEF
    "OUT",
    "POW",
    "EXIT",
    "LABLE",
};

typedef struct {
    OPCODE_ opcode;
    Var first;
    Var second;
    Var third;
    size_t jmpId;
    size_t labelId;
    char name[100];         //TODO
} Command;

#endif
