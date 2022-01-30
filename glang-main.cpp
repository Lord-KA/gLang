#include "glang.h"

int main()
{
    gLang langStruct;
    gLang *ctx = &langStruct;
    gLang_ctor(ctx, stderr);
    // gLang_lexer(ctx, "main(x){if (f(x, 12 + 1, 4) < 18) return 1;}");
    gLang_lexer(ctx, "      \
            main(x)         \
            {               \
                a = b + 17 * fuck;          \
                if (f(a, 12 + 1, 4) < 18)   \
                    return 1;               \
            }");
    // gLang_lexer(ctx, "a - b < 10 / 3 * fuck - 15");
    FILE *out = fopen("before.gv", "w");
    gTree_dumpPoolGraphViz(&ctx->tree, out);
    fclose(out);

    // gLang_parser_expr(ctx, ctx->tree.root);
    gLang_parser(ctx);
    out = fopen("after.gv", "w");
    gTree_dumpPoolGraphViz(&ctx->tree, out);
    fclose(out);

    gLang_dtor(ctx);
}
