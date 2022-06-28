#include "glang.h"

int main(int argc, char **argv)
{
    int c = 0;
    char *inFileName  = NULL;
    char *outFileName = NULL;

    while ((c = getopt(argc, argv, "hi:o:")) != -1) {
        switch(c) {
        case 'h':
            printf("This is a custom compiler for gLang made by _Lord_KA_:\n"
                   "Usage: -h          : show this help\n"
                   "       -i FileName : input from file\n"
                   "       -o FileName : output to file\n");
            return 0;
        case 'o':
            outFileName = optarg;
            break;
        case 'i':
            inFileName = optarg;
            break;
        }
    }

    FILE *in  = stdin;
    FILE *out = stdout;
    if (outFileName != NULL) {
        out = fopen(outFileName, "w");
        if (out == NULL) {
            fprintf(stderr, "Failed to open the file!\n");
            return 1;
        }
    }
    if (inFileName != NULL) {
        in = fopen(inFileName, "r");
        if (in == NULL) {
            fprintf(stderr, "Failed to open the file!\n");
            return 1;
        }
    }

    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "inFileName = %s, outFileName = %s\n", inFileName, outFileName);
    #endif

    fseek(in, 0L, SEEK_END);
    size_t bufLen = ftell(in);
    fseek(in, 0L, SEEK_SET);
    char *buffer = (char*)calloc(bufLen + 1, sizeof(char));
    fread(buffer, sizeof(char), bufLen, in);

    if (ferror(in)) {
        fprintf(stderr, "Failed to read from file!\n");
        free(buffer);
        return 1;
    }

    #ifdef EXTRA_VERBOSE
        size_t len = strlen(buffer);
        fprintf(stderr, "len = %lu\n", len);
    #endif

    gLang langStruct;
    gLang *ctx = &langStruct;
    gLang_ctor(ctx, stderr);

    gLang_lexer(ctx, buffer);
    #ifdef EXTRA_VERBOSE
        FILE *gvOut = fopen("before.gv", "w");
        gTree_dumpPoolGraphViz(&ctx->tree, gvOut);
        fclose(gvOut);
    #endif

    gLang_parser(ctx);
    #ifdef EXTRA_VERBOSE
        gvOut = fopen("after.gv", "w");
        gTree_dumpPoolGraphViz(&ctx->tree, gvOut);
        fclose(gvOut);
    #endif

    gLang_optimize(ctx, ctx->tree.root);
    #ifdef EXTRA_VERBOSE
        gvOut = fopen("optimized.gv", "w");
        gTree_dumpPoolGraphViz(&ctx->tree, gvOut);
        fclose(gvOut);
    #endif

    gLang_compile(ctx);

    fprintf(stderr, "\n\ncommands len = %zu\n", ctx->commands->len);

    #ifdef EXTRA_VERBOSE
        gvOut = fopen("varsfilled.gv", "w");
        gTree_dumpPoolGraphViz(&ctx->tree, gvOut);
        fclose(gvOut);
    #endif

    #ifdef EXTRA_VERBOSE
        gvOut = fopen("commandsDump.txt", "w");
        gLang_commandsDump(ctx, gvOut);
        fclose(gvOut);
    #endif

    gLang_translate(ctx, true);

    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "\n\nLabel fixups:\n");
        if (ctx->labelFixup != NULL) {
            for (size_t i = 1; i < ctx->labelCnt; ++i) {
                assert(ctx->labelFixup[i] != 0);
                fprintf(stderr, "%zu\t| offset = %zu\n", i, ctx->labelFixup[i]);
            }
        } else {
            fprintf(stderr, "EMPTY!\n");
        }
    #endif

    gLang_translate(ctx, false);

finish:
    fclose(out);
    gLang_dtor(ctx);
    free(buffer);
}
