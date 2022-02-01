#include "glang.h"

int main()
{
    gLang langStruct;
    gLang *ctx = &langStruct;
    gLang_ctor(ctx, stderr);
    /*
    char program[] = R"(
            f(x, y, z)
            {
                return x + y + z;
            }
            main(x)
            {
                a = b + 17 * fuck;
                if (f(a, 12 + 1, 4) < 18)
                    while (a > 0)
                        a = a - 1;
                return a;
            })";
    */
    char program[] = "                      \
            f(x, y, z)                      \
            {                               \
                return x + y + z;           \
            }                               \
            main(x)                         \
            {                               \
                a = b + 17 * fuck;          \
                if (f(a, 12 + 1, 4) < 18)   \
                    while (a > 0)           \
                        a = a - 1;          \
                return a;                   \
            }";
    size_t len = strlen(program);
    fprintf(stderr, "len = %lu\n", len);
    gLang_lexer(ctx, program);
    FILE *out = fopen("before.gv", "w");
    gTree_dumpPoolGraphViz(&ctx->tree, out);
    fclose(out);

    gLang_parser(ctx);
    out = fopen("after.gv", "w");
    gTree_dumpPoolGraphViz(&ctx->tree, out);
    fclose(out);

    gLang_dtor(ctx);
}
