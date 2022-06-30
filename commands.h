#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdlib.h>
#include <stdint.h>
#include "varpool.h"
#include "constants.h"

typedef enum {
    OP_NONE_ = 0,
    #define OP_DEF(name, ...) name,
        #include "opcodes.h"
    #undef OP_DEF
    OP_CNT_
} OPCODE_;

static const size_t OPCODE_ARGS[OP_CNT_] = {
    0,
    #define OP_DEF(name, args, ...) args,
        #include "opcodes.h"
    #undef OP_DEF
};

static const size_t OPCODE_LEN[OP_CNT_] = {};

static const char OPCODE_MSG[OP_CNT_][10] = {
    "NONE",
    #define OP_DEF(name, ...) #name ,
        #include "opcodes.h"
    #undef OP_DEF
};

typedef struct {
    OPCODE_ opcode;
    Var first;
    Var second;
    Var third;
    size_t labelId;
    char name[GLANG_MAX_LIT_LEN];
    size_t line;
} Command;

#endif
