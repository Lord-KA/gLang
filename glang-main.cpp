#include "glang.h"

int main()
{
    gLang langStruct;
    gLang *ctx = &langStruct;
    gLang_ctor(ctx, stderr);
    gLang_lexer(ctx, "fuck = 10; if (fuck > 0) fuck = fuck- 1;");
    FILE *out = fopen("before.gv", "w");
    gTree_dumpPoolGraphViz(&ctx->tree, out);
    fclose(out);
    gLang_dtor(ctx);
}
