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
    /*
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
                return fib(d);              \
            }                               \
    ";
    */
    /*
    char program[] = "                      \
            fib_rec(a) {                    \
                if (a > 2)                  \
                    return fib_rec(a - 1) + \
                             fib_rec(a - 2);\
                if (a > 1)                  \
                    return 1;               \
                return 1;                   \
            }                               \
            fib_iter(n)                     \
            {                               \
                a = 1;                      \
                b = 0;                      \
                while (n > 1) {             \
                    tmp = a;                \
                    a = a + b;              \
                    b = tmp;                \
                    n = n - 1;              \
                    print n;                \
                }                           \
                return a;                   \
            }                               \
            main()                          \
            {                               \
                return fib_iter(6);         \
            }                               \
    ";
    */
    char program[] = "                      \
            abs(x)                          \
            {                               \
                if (x < 0)                  \
                    return (-1) * x;        \
                return x;                   \
            }                               \
            sign(x)                         \
            {                               \
                if (x < 0)                  \
                    return -1;              \
                return 1;                   \
            }                               \
            quadric_solver(a, b, c)         \
            {                               \
                if (abs(a) < 1) {           \
                    print -1 * c / b;       \
                    return 0;               \
                }                           \
                det = b^2 - 4 * a * c;      \
                if (det > 0) {              \
                    two = 2;                \
                    print ((-1 * b - sqrt(det)) / two) / a;\
                    print ((-1 * b + sqrt(det)) / two) / a; \
                    return 0;               \
                }                           \
                return 1;                   \
            }                               \
            main()                          \
            {                               \
                return quadric_solver(1, 2, -3);\
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

    gLang_optimize(ctx, ctx->tree.root);
    out = fopen("optimized.gv", "w");
    gTree_dumpPoolGraphViz(&ctx->tree, out);
    fclose(out);

    gLang_compile(ctx, stdout);

    gLang_dtor(ctx);
}
