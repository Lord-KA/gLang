#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdlib.h>
#include <stdint.h>
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
    0,
    #define OP_DEF(name, args, ...) args,
        #include "opcodes.h"
    #undef OP_DEF
    1,
    2,
    0,
    0,
};

static const size_t OPCODE_LEN[OP_CNT_] = {};

/*
static const uint8_t OPCODE_BYTES[OP_CNT_][3] = {
    {},
    #define OP_DEF(name, args, bytes, ...) bytes,       //TODO update opcodes.h with real byte representation
        #include "opcodes.h"
    #undef OP_DEF
    {0x12},
    {0x13},
    {0x14},
    {0x15},
};
*/

static const char OPCODE_MSG[OP_CNT_][10] = {
    "NONE",
    #define OP_DEF(name, ...) #name ,
        #include "opcodes.h"
    #undef OP_DEF
    "OUT",
    "POW",
    "EXIT",
    "LABLE",
};

/*
void setOpcodeLens()
{
    size_t i = 0
    #define OP_DEF(name, args, bytes) ({ \
            char b = 'a';   \
            OPCODE_LEN[++i] = sizeof(b);    \
            })
        OP_DEF("name", "args", ({0x123, 0x45}));
        // #include "opcodes.h"
    #undef OP_DEF
}
*/

typedef struct {
    OPCODE_ opcode;
    Var first;
    Var second;
    Var third;
    size_t jmpId;
    size_t labelId;
    char name[100];         //TODO
    size_t line;
} Command;

#endif
