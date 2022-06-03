#include <stdlib.h>
#include "varpool.h"

typedef enum {
    OP_NONE_ = 0,
    #define OP_DEF(name, ...) name,
        #include "opcodes.h"
    #undef OP_DEF
    EXIT,
    LABEL,
    OP_CNT_
} OPCODE_;

typedef struct {
    OPCODE_ opcode;
    Var first;
    Var second;
    Var third;
    size_t jmpId;
    char name[100];         //TODO
} Command;
