#include "glang.h"

int main()
{
    gLang langStruct;
    gLang *ctx = &langStruct;
    gLang_ctor(ctx, stderr);

    FILE *in = stdin;
    fseek(in, 0L, SEEK_END);
    size_t bufLen = ftell(in);
    fseek(in, 0L, SEEK_SET);
    char *buffer = (char*)calloc(bufLen + 1, sizeof(char));
    fread(buffer, sizeof(char), bufLen, in);

    if (ferror(in)) {
        fprintf(stderr, "Failed to read from file!\n");
        return 1;
    }

    size_t len = strlen(buffer);
    fprintf(stderr, "len = %lu\n", len);
    gLang_lexer(ctx, buffer);
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
    free(buffer);
}
