#include "glang.h"

int main()
{
    gLang context = {};
    gLang *ctx = &context;
    ctx->logStream = stderr;
    ctx->commands = gArr_new_c(1000);
    Command c = {};
    /*
    c.opcode = PUSH;
    c.first.reg = R15;
    gArr_push_c(ctx->commands, c);
    c.first.reg = R8;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RDI;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RSI;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RCX;
    gArr_push_c(ctx->commands, c);

    c.opcode = POP;
    c.first.reg = R15;
    gArr_push_c(ctx->commands, c);
    c.first.reg = R8;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RDI;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RSI;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RCX;
    gArr_push_c(ctx->commands, c);

    c.opcode = ADD;
    c.first.reg = R15;
    c.second.reg = RSP;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RAX;
    c.second.reg = RAX;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RBX;
    c.second.reg = R12;
    gArr_push_c(ctx->commands, c);
    c.first.reg = R11;
    c.second.reg = R14;
    gArr_push_c(ctx->commands, c);
    c.first.reg = R10;
    c.second.reg = RBP;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RBP;
    c.second.reg = RBP;
    gArr_push_c(ctx->commands, c);
    */

    c.opcode = SUB;
    c.first.reg = R15;
    c.second.reg = RSP;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RAX;
    c.second.reg = RAX;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RBX;
    c.second.reg = R12;
    gArr_push_c(ctx->commands, c);
    c.first.reg = R11;
    c.second.reg = R14;
    gArr_push_c(ctx->commands, c);
    c.first.reg = R10;
    c.second.reg = RBP;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RBP;
    c.second.reg = RBP;
    gArr_push_c(ctx->commands, c);



    assert(ctx->commands->len > 0);
    gLang_translate(ctx, false);
    for (size_t i = 0; i < ctx->bin->len; ++i) {
        fprintf(stderr, "%x ", ctx->bin->data[i]);
    }
    //gLang_dtor(ctx);
    fprintf(stderr, "\n");
}
