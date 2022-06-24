#include "glang.h"

static size_t testCnt = 1;

void check(gLang *ctx, uint8_t bytes[])
{
    gLang_translate(ctx, false);
    gArr_delete_c(ctx->commands);
    ctx->commands = gArr_new_c(1000);
    fprintf(stderr, "len = %zu\n", ctx->bin->len);
    for (size_t i = 0; i < ctx->bin->len; ++i) {
        fprintf(stderr, "%2x ", ctx->bin->data[i]);
    }
    assert(!memcmp(bytes, ctx->bin->data, ctx->bin->len));
    fprintf(stderr, "\ntest %zu passed!\n\n", testCnt);
    ++testCnt;
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
    c.first.offset = 0x11;
    gArr_push_c(ctx->commands, c);
    c.first.offset = 0x1122;
    gArr_push_c(ctx->commands, c);
    c.first.offset = 0x112233;
    gArr_push_c(ctx->commands, c);
    c.first.offset = 0x11223344l;
    gArr_push_c(ctx->commands, c);
    c.first.offset = 0x1122334455l;
    gArr_push_c(ctx->commands, c);
    c.first.offset = 0x112233445566l;
    gArr_push_c(ctx->commands, c);
    c.first.offset = 0x11223344556677l;
    gArr_push_c(ctx->commands, c);
    c.first.offset = 0x1122334455667788l;
    gArr_push_c(ctx->commands, c);
    uint8_t bytes_5[] = {
    	0x41, 0xff, 0xd7,   // call r15
	    0xff, 0xd0,         // call rax
	    0x41, 0xff, 0xd2,   // call r10
	    0xff, 0xd4,         // call rsp
	    0xff, 0xd1,         // call rcx
    	0x49, 0xba, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs r10,0x11
	    0x41, 0xff, 0xd2,   // call r10
    	0x49, 0xba, 0x22, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs r10,0x1122
	    0x41, 0xff, 0xd2,   // call r10
    	0x49, 0xba, 0x33, 0x22, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs r10,0x112233
	    0x41, 0xff, 0xd2,   // call r10
    	0x49, 0xba, 0x44, 0x33, 0x22, 0x11, 0x00, 0x00, 0x00, 0x00, // movabs r10,0x11223344
	    0x41, 0xff, 0xd2,   // call r10
    	0x49, 0xba, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00, 0x00, 0x00, // mov r10,0x1122334455...
	    0x41, 0xff, 0xd2,   // call r10
    	0x49, 0xba, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00, 0x00, // mov r10,0x1122334455...
	    0x41, 0xff, 0xd2,   // call r10
    	0x49, 0xba, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00, // mov r10,0x1122334455...
	    0x41, 0xff, 0xd2,   // call r10
    	0x49, 0xba, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // mov r10,0x1122334455...
	    0x41, 0xff, 0xd2,   // call r10
    };
    check(ctx, bytes_5);

    c.opcode = CMP;
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

    uint8_t bytes_6[] = {
	    0x49, 0x39, 0xe7, // cmp r15,rsp
	    0x48, 0x39, 0xc0, // cmp rax,rax
	    0x4c, 0x39, 0xe3, // cmp rbx,r12
	    0x4d, 0x39, 0xf3, // cmp r11,r14
	    0x49, 0x39, 0xea, // cmp r10,rbp
	    0x48, 0x39, 0xed, // cmp rbp,rbp
    };

    check(ctx, bytes_6);


    c.opcode = MOV;
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

    c.first.reg = RBP;
    c.second.reg = REG_NONE_;
    c.second.num = -0x12345;
    gArr_push_c(ctx->commands, c);
    c.second.num = -0x98765432l;
    gArr_push_c(ctx->commands, c);


    uint8_t bytes_7[] = {
	    0x49, 0x89, 0xe7, // cmp r15,rsp
	    0x48, 0x89, 0xc0, // cmp rax,rax
	    0x4c, 0x89, 0xe3, // cmp rbx,r12
	    0x4d, 0x89, 0xf3, // cmp r11,r14
	    0x49, 0x89, 0xea, // cmp r10,rbp
	    0x48, 0x89, 0xed, // cmp rbp,rbp
    	0x48, 0xbd, 0xbb, 0xdc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, // movabs rbp,0xfffffffffffedcbb
        0x48, 0xbd, 0xce, 0xab, 0x89, 0x67, 0xff, 0xff, 0xff, 0xff, // movabs rbp,0xffffffff6789abce
    };

    check(ctx, bytes_7);


    c.opcode = CMOVL;
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

    uint8_t bytes_8[] = {
        0x4c, 0x0f, 0x4c, 0xfc, // cmovl r15,rsp
        0x48, 0x0f, 0x4c, 0xc0, // cmovl rax,rax
        0x49, 0x0f, 0x4c, 0xdc, // cmovl rbx,r12
        0x4d, 0x0f, 0x4c, 0xde, // cmovl r11,r14
        0x4c, 0x0f, 0x4c, 0xd5, // cmovl r10,rbp
        0x48, 0x0f, 0x4c, 0xed, // cmovl rbp,rbp
    };

    check(ctx, bytes_8);


    c.opcode = CMOVG;
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

    uint8_t bytes_9[] = {
        0x4c, 0x0f, 0x4f, 0xfc, // cmovg r15,rsp
        0x48, 0x0f, 0x4f, 0xc0, // cmovg rax,rax
        0x49, 0x0f, 0x4f, 0xdc, // cmovg rbx,r12
        0x4d, 0x0f, 0x4f, 0xde, // cmovg r11,r14
        0x4c, 0x0f, 0x4f, 0xd5, // cmovg r10,rbp
        0x48, 0x0f, 0x4f, 0xed, // cmovg rbp,rbp
    };

    check(ctx, bytes_9);


    c.opcode = JMP;
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
    c.first.offset = 0x1234567l;
    gArr_push_c(ctx->commands, c);

    uint8_t bytes_10[] = {
    	0x41, 0xff, 0xe7,   // jmp r15
	    0xff, 0xe0,         // jmp rax
	    0x41, 0xff, 0xe2,   // jmp r10
	    0xff, 0xe4,         // jmp rsp
	    0xff, 0xe1,         // jmp rcx
    	0x49, 0xba, 0x67, 0x45, 0x23, 0x01, 0x00, 0x00, 0x00, 0x00, // movabs r10,0x1234567
	    0x41, 0xff, 0xe2,   // jmp r10
    };

    check(ctx, bytes_10);


    c.opcode = TEST;
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

    uint8_t bytes_11[] = {
    	0x49, 0x85, 0xe7, // test r15,rsp
	    0x48, 0x85, 0xc0, // test rax,rax
    	0x4c, 0x85, 0xe3, // test rbx,r12
	    0x4d, 0x85, 0xf3, // test r11,r14
	    0x49, 0x85, 0xea, // test r10,rbp
	    0x48, 0x85, 0xed, // test rbp,rbp
    };

    check(ctx, bytes_11);


    c.opcode = TEST;
    c.first.reg = R15;
    c.second.reg = RSP;
    gArr_push_c(ctx->commands, c);
    c.opcode = RET;
    gArr_push_c(ctx->commands, c);
    c.opcode = TEST;
    c.first.reg = RAX;
    c.second.reg = RAX;
    gArr_push_c(ctx->commands, c);

    uint8_t bytes_12[] = {
    	0x49, 0x85, 0xe7, // test r15,rsp
        0xc3,             // ret
	    0x48, 0x85, 0xc0, // test rax,rax
    };

    check(ctx, bytes_12);


    ctx->labelFixup = (size_t*)calloc(6, sizeof(size_t));
    ctx->labelFixup[0] = 0x11;
    ctx->labelFixup[1] = 0x22;
    ctx->labelFixup[2] = 0x33;
    ctx->labelFixup[3] = 0x44;
    ctx->labelFixup[4] = 0x55;
    ctx->labelFixup[5] = 0x200;     // For now this will fail by assertion

    c.opcode = JE;
    for (size_t j = 0; j < 5; ++j) {
        c.labelId = j;
        gArr_push_c(ctx->commands, c);
    }

    uint8_t bytes_13[] = {
        0x74, 0x0f,
        0x74, 0x1e,
        0x74, 0x2d,
        0x74, 0x3c,
        0x74, 0x4b,
    };

    check(ctx, bytes_13);


    c.opcode = IMUL;
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

    uint8_t bytes_14[] = {
    	0x4c, 0x0f, 0xaf, 0xfc, // imul r15,rsp
	    0x48, 0x0f, 0xaf, 0xc0, // imul rax,rax
	    0x49, 0x0f, 0xaf, 0xdc, // imul rbx,r12
	    0x4d, 0x0f, 0xaf, 0xde, // imul r11,r14
	    0x4c, 0x0f, 0xaf, 0xd5, // imul r10,rbp
	    0x48, 0x0f, 0xaf, 0xed, // imul rbp,rbp
    };

    check(ctx, bytes_14);


    c.opcode = IDIV;
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
    c.first.reg = RBP;
    gArr_push_c(ctx->commands, c);

    uint8_t bytes_15[] = {
    	0x49, 0xf7, 0xff, // idiv r15
	    0x48, 0xf7, 0xf8, // idiv rax
	    0x49, 0xf7, 0xfa, // idiv r10
	    0x48, 0xf7, 0xfc, // idiv rsp
	    0x48, 0xf7, 0xf9, // idiv rcx
	    0x48, 0xf7, 0xfd, // idiv rbp
    };

    check(ctx, bytes_15);


    fprintf(stderr, "\nPassed all tests!\n");


    gArr_delete_c(ctx->commands);
    gArr_delete_b(ctx->bin);
    free(ctx->labelFixup);
}
