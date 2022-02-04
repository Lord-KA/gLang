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
    /*
    char program[] = "                      \
            g() {return 1;}                 \
            f(x, y, z)                      \
            {                               \
                return x + y + z;           \
            }                               \
            main(x)                         \
            {                               \
                a = b + 17 * fuck;          \
                if (f(a, 12 + 1, 4) < 18) { \
                    while (a > 0)           \
                        a = a - 1;          \
                    l = a - b;              \
                }                           \
                return a;                   \
            }                               \
    ";
    */
    char program[] = "                      \
            fib(a) {                        \
                if (a > 2)                  \
                    return f(a - 1) + f(a - 2);\
                if (a > 1)                  \
                    return 1;               \
                return 1;                   \
            }                               \
            main()                          \
            {                               \
                a = 99;                     \
                b = 100;                    \
                c = 17;                     \
                d = 13;                     \
                return fib(6);              \
            }                               \
    ";
    /*
    char program[] = "                      \
           main()                           \
            {                               \
                a = 0-1;                      \
                b = 3;                      \
                return b^a;           \
                if (a > 0)                  \
                    return a / b;           \
                return 0;                   \
            }                               \
    ";
    */

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

    gLang_compile(ctx, stdout);
    /*
    gLang_optimize(ctx, ctx->tree.root);
    out = fopen("optimized.gv", "w");
    gTree_dumpPoolGraphViz(&ctx->tree, out);
    fclose(out);
    */

    gLang_dtor(ctx);
}
