#include "glang.h"


void check(gLang *ctx, uint8_t bytes[])
{
    gLang_translate(ctx, false);
    gArr_delete_c(ctx->commands);
    ctx->commands = gArr_new_c(1000);
    fprintf(stderr, "len = %zu\n", ctx->bin->len);
    for (size_t i = 0; i < ctx->bin->len; ++i) {
        fprintf(stderr, "%2x ", ctx->bin->data[i]);
    }
    fprintf(stderr, "\n\n");
    assert(!memcmp(bytes, ctx->bin->data, ctx->bin->len));
}

int main()
{
    gLang context = {};
    gLang *ctx = &context;
    ctx->logStream = stderr;
    ctx->commands = gArr_new_c(1000);
    Command c  {};

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
    gLang_translate(ctx, false);

    uint8_t bytes_1[] = {
        	0x41, 0x57, // push r15
	        0x41, 0x50, // push r8
	        0x57,       // push rdi
	        0x56,       // push rsi
	        0x51,       // push rcx
    };
    check(ctx, bytes_1);

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

    uint8_t bytes_2[] = {
    	0x41, 0x5f, // pop r15
	    0x41, 0x58, // pop r8
	    0x5f,       // pop rdi
	    0x5e,       // pop rsi
	    0x59,       // pop rcx
    };
    check(ctx, bytes_2);

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

    uint8_t bytes_3[] = {
        0x49, 0x01, 0xe7, // add r15,rsp
	    0x48, 0x01, 0xc0, // add rax,rax
	    0x4c, 0x01, 0xe3, // add rbx,r12
	    0x4d, 0x01, 0xf3, // add r11,r14
	    0x49, 0x01, 0xea, // add r10,rbp
	    0x48, 0x01, 0xed, // add rbp,rbp
    };
    check(ctx, bytes_3);

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

    uint8_t bytes_4[] = {
        0x49, 0x29, 0xe7, // sub r15,rsp
	    0x48, 0x29, 0xc0, // sub rax,rax
	    0x4c, 0x29, 0xe3, // sub rbx,r12
	    0x4d, 0x29, 0xf3, // sub r11,r14
    	0x49, 0x29, 0xea, // sub r10,rbp
	    0x48, 0x29, 0xed, // sub rbp,rbp
    };
    check(ctx, bytes_4);

    c.opcode = CALL;
    c.first.reg = R15;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RAX;
    gArr_push_c(ctx->commands, c);
    c.first.reg = R10;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RSP;
    gArr_push_c(ctx->commands, c);
    c.first.reg = RCX;
    gArr_push_c(ctx->commands, c);
    c.first.reg = REG_NONE_;
    c.first.offset = 0x012345;
    gArr_push_c(ctx->commands, c);

    uint8_t bytes_5[] = {
    	0x41, 0xff, 0xd7,   // call r15
	    0xff, 0xd0,         // call rax
	    0x41, 0xff, 0xd2,   // call r10
	    0xff, 0xd4,         // call rsp
	    0xff, 0xd1,         // call rcx
    	0x41, 0xba, 0x45, 0x23, 0x01, 0x00, // mov r10d,0x12345
	    0x41, 0xff, 0xd2,   // call r10
    };
    check(ctx, bytes_5);


    c.opcode = CMP;

finish:
    //gLang_dtor(ctx);
    fprintf(stderr, "\n");
}
