#pragma once

#define STACK_TYPE size_t
#define ELEM_PRINTF_FORM "%lu"
#define CHEAP_DEBUG

#include "gstack.h"

static const size_t MAX_LINE_LEN = 1000;
static const size_t GLANG_MAX_LIT_LEN = 100;
static const size_t GLANG_LEX_LIM = 1000;

static const double GLANG_EPS = 1e-3;

enum gLang_Node_mode {
    gLang_Node_mode_none,
    gLang_Node_mode_sum,
    gLang_Node_mode_sub,
    gLang_Node_mode_mul,
    gLang_Node_mode_div,
    gLang_Node_mode_exp,
    gLang_Node_mode_opBrack,
    gLang_Node_mode_clBrack,
    gLang_Node_mode_opFigBrack,
    gLang_Node_mode_clFigBrack,
    gLang_Node_mode_Semicolon,
    gLang_Node_mode_less,
    gLang_Node_mode_great,
    gLang_Node_mode_assign,
    gLang_Node_mode_var,
    gLang_Node_mode_func,
    gLang_Node_mode_num,
    gLang_Node_mode_keyword,
    gLang_Node_mode_unknown,
    gLang_Node_mode_CNT,
};

static const char gLang_Node_modeView[gLang_Node_mode_CNT][GLANG_MAX_LIT_LEN] = {
        "NONE",
        "+",
        "-",
        "*",
        "/",
        "^",
        "(",
        ")",
        "{",
        "}",
        ";",
        "<",
        ">",
        "=",
        "VAR",
        "FUNC",
        "NUM",
        "KEYWORD",
        "UNKNOWN",
    };

enum gLang_Node_keyword {
    gLang_Node_keyword_if,
    gLang_Node_keyword_else,
    gLang_Node_keyword_CNT,
};

static const char gLang_Node_keywordView[gLang_Node_keyword_CNT][GLANG_MAX_LIT_LEN] = {
    "if",
    "else",
    };

struct gLang_Node
{
    gLang_Node_mode    mode;
    gLang_Node_keyword keyword;
    char funcName[GLANG_MAX_LIT_LEN];
    char  varName[GLANG_MAX_LIT_LEN];
    double value;
    size_t position;
} typedef gLang_Node;

static const char GLANG_DELIMS_LIST[] = " +-*/^(){};<>=";

typedef gLang_Node GTREE_TYPE;

#include "gtree.h"

bool gTree_storeData(gLang_Node data, size_t level, FILE *out) //TODO
{
    return 0;
}

bool gTree_restoreData(gLang_Node *data, FILE *in) //TODO
{
    return 0;
}

bool gTree_printData(gLang_Node data, FILE *out)
{
    assert(gPtrValid(out));
    if (data.mode >= gLang_Node_mode_CNT || data.mode < 0)
        data.mode = gLang_Node_mode_unknown;
    if (data.mode >= gLang_Node_mode_less && data.mode <= gLang_Node_mode_assign)
        fprintf(out, "{mode | %d (\'\\%s\')}", data.mode, gLang_Node_modeView[data.mode]);
    else
        fprintf(out, "{mode | %d (\'%s\')}", data.mode, gLang_Node_modeView[data.mode]);

    if (data.mode == gLang_Node_mode_num)
        fprintf(out, "| {%lf} ", data.value);
    else if (data.mode == gLang_Node_mode_func)
        fprintf(out, "| {\'%s\'} ", data.funcName);
    else if (data.mode == gLang_Node_mode_keyword)
        fprintf(out, "| {\'%s\'} ", gLang_Node_keywordView[data.keyword]);
    else if (data.mode == gLang_Node_mode_var)
        fprintf(out, "| {\'%s\'} ", data.varName);

    return 0;
}

enum gLang_status {
    gLang_status_OK,
    gLang_status_BadStructPtr,
    gLang_status_TreeErr,
    gLang_status_ObjPoolErr,
    gLang_status_FileErr,
    gLang_status_BadPtr,
    gLang_status_BadId,
    gLang_status_BadInput,
    gLang_status_EmptyLexer,
    gLang_status_EmptyTree,
    gLang_status_ParsingErr_UnknownLex,
    gLang_status_ParsingErr_NoBrack,
    gLang_status_DecompositionErr,
    gLang_status_CNT,
};

static const char gLang_statusMsg[gLang_status_CNT + 1][MAX_LINE_LEN] = {
        "OK",
        "Bad structure pointer provided",
        "Error in gTree",
        "Error in gObjPool",
        "Error in file IO",
        "Bad pointer provided",
        "Bad node id provided",
        "WARNING: bad input provided",
        "WARNING: lexemes stack is empty, have you run the lexer?",
        "WARNING: expression tree is empty, have you run the parser?",
        "Parsing error: unknown lexemes sequence",
        "Parsing error: no closing bracket",
        "Some error during tree decomposition",
        "UNKNOWN ERROR CODE!",
    };

#ifndef NLOGS
#define GLANG_ASSERT_LOG(expr, errCode) ({                                         \
        if (errCode >= gLang_status_CNT || errCode < 0)  {                          \
            ASSERT_LOG(false, gLang_status_CNT,                                      \
                    gLang_statusMsg[gLang_status_CNT], context->logStream);           \
        }                                                                              \
        ASSERT_LOG(expr, errCode, gLang_statusMsg[errCode], context->logStream);        \
    })
#else
#define GLANG_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gLang_statusMsg[errCode], NULL)
#endif

#define GLANG_CHECK_SELF_PTR(ptr) ASSERT_LOG(gPtrValid(ptr), gLang_status_BadStructPtr,     \
                                                 gLang_statusMsg[gLang_status_BadStructPtr], \
                                                 stderr)
#define GLANG_IS_OK(expr) ({                                                 \
        gLang_status macroStatus = (expr);                                    \
        GLANG_ASSERT_LOG(macroStatus == gLang_status_OK, macroStatus);         \
    })

#define GLANG_NODE_BY_ID(macroId) ({                                                                    \
    assert(macroId != -1);                                                                               \
    gTree_Node *macroNode = NULL;                                                                         \
    GLANG_ASSERT_LOG(gObjPool_get(&context->tree.pool, (macroId), &macroNode) == gObjPool_status_OK,       \
                            gLang_status_ObjPoolErr);                                                       \
    assert(gPtrValid(macroNode));                                                                            \
    macroNode;                                                                                                \
})


#define GLANG_POOL_ALLOC() ({                                                                 \
    size_t macroId = -1;                                                                       \
    gTree_Node *macroNode = NULL;                                                               \
    GLANG_ASSERT_LOG(gObjPool_alloc(&context->tree.pool, &macroId) == gObjPool_status_OK,        \
                            gLang_status_ObjPoolErr);                                             \
    GLANG_ASSERT_LOG(gObjPool_get(&context->tree.pool, macroId, &macroNode) == gObjPool_status_OK, \
                            gLang_status_ObjPoolErr);                                               \
      macroNode->sibling = -1;                                                                            \
      macroNode->parent  = -1;                                                                             \
      macroNode->child   = -1;                                                                              \
      macroId;                                                                                               \
})

#define GLANG_POOL_FREE(id) ({                                                           \
    GLANG_ASSERT_LOG(gObjPool_free(&context->tree.pool, id) == gObjPool_status_OK,        \
                            gLang_status_ObjPoolErr);                                      \
})


#define GLANG_TREE_CHECK(expr) ({                                             \
    GLANG_ASSERT_LOG((expr) == gTree_status_OK, gLang_status_TreeErr);         \
})


#define GLANG_ID_CHECK(id) GLANG_ASSERT_LOG(gObjPool_idValid(&context->tree.pool, id), gLang_status_BadId)

#define GLANG_IS_DELIM(macroCur) (strnConsistsChrs(macroCur, GLANG_DELIMS_LIST, 1, strlen(GLANG_DELIMS_LIST)) || isspace(*macroCur))

struct gLang {
    gTree          tree      = {};
    FILE          *logStream = {};
    GENERIC(stack) LexemeIds = {};
    size_t         lexemeCur = {};
    const char    *buffer    = {};
} typedef gLang;


static gLang_status gLang_ctor(gLang *context, FILE *newLogStream)
{
    if (!gPtrValid(context)) {
        FILE *out;
        if (!gPtrValid(newLogStream))
            out = stderr;
        else
            out = newLogStream;
        fprintf(out, "ERROR: bad structure ptr provided to derivator ctor!\n");
        return gLang_status_BadStructPtr;
    }

    context->logStream = stderr;
    if (gPtrValid(newLogStream))
        context->logStream = newLogStream;

    gTree_status status = gTree_ctor(&context->tree, context->logStream);
    GLANG_ASSERT_LOG(status == gTree_status_OK, gLang_status_TreeErr);

    GENERIC(stack_ctor)(&context->LexemeIds);

    return gLang_status_OK;
}

static gLang_status gLang_dtor(gLang *context)
{
    GLANG_CHECK_SELF_PTR(context);

    GLANG_TREE_CHECK(gTree_dtor(&context->tree));

    GENERIC(stack_dtor)(&context->LexemeIds);

    return gLang_status_OK;
}

static gLang_status gLang_lexer(gLang *context, const char *buffer)
{
    GLANG_CHECK_SELF_PTR(context);
    GLANG_ASSERT_LOG(gPtrValid(buffer), gLang_status_BadPtr);

    context->buffer = buffer;
    char *cur = (char*)buffer;
    size_t id = -1;
    gLang_Node *node = NULL;
    GENERIC(stack_clear)(&context->LexemeIds);

    while (*cur != '\0' && *cur != '\n') {
        #ifdef EXTRA_VERBOSE
            fprintf(stderr, "cur = %s\n", cur);
        #endif
        if (isspace(*cur)) {
            ++cur;
            continue;
        }
        id   = GLANG_POOL_ALLOC();
        node = &(GLANG_NODE_BY_ID(id)->data);
        GENERIC(stack_push)(&context->LexemeIds, id);
        node->position = cur - buffer;

        bool foundLit = false;
        for (size_t i = gLang_Node_mode_sum; i < gLang_Node_mode_var; ++i) {
            /* one-symbol literals case */
            if (*gLang_Node_modeView[i] == *cur) {
                node->mode = (gLang_Node_mode)i;
                foundLit = true;
                ++cur;
                break;
            }
        }
        if (foundLit) {
            #ifdef EXTRA_VERBOSE
                fprintf(stderr, "Found lit!\n");
                fprintf(stderr, " cur = #%s#\n", cur);
            #endif
            continue;
        }

        char *litEnd = cur;
        while (!GLANG_IS_DELIM(litEnd))
            ++litEnd;
        char literal[GLANG_MAX_LIT_LEN] = "";
        if (litEnd - cur >= GLANG_MAX_LIT_LEN) {
            fprintf(context->logStream, "ERROR: literal is too long (MAX_LIT_LEN = %lu)\n", GLANG_MAX_LIT_LEN);
            GLANG_ASSERT_LOG(false, gLang_status_ParsingErr_UnknownLex);
        }
        strncpy(literal, cur, litEnd - cur);

        if (isdigit(*cur)) {
            /* numeric literal case */
            if (isInteger(literal)) {
                node->mode = gLang_Node_mode_num;
                node->value = (double)(strtol(literal, NULL, 0));
            } else if (isDouble(literal)) {
                node->mode = gLang_Node_mode_num;
                node->value = strtod(literal, NULL);
            } else {
                node->mode = gLang_Node_mode_unknown;
                fprintf(stderr, "ERROR: bad function/variable name; it should not start with a digit!\n");
                GLANG_ASSERT_LOG(false, gLang_status_ParsingErr_UnknownLex);
            }
        } else {
            /* keyword case */
            size_t i = 0;
            while (i < gLang_Node_keyword_CNT && strncmp(literal, gLang_Node_keywordView[i], GLANG_MAX_LIT_LEN))
                ++i;
            if (i != gLang_Node_keyword_CNT) {
                node->mode = gLang_Node_mode_keyword;
                node->keyword = (gLang_Node_keyword)(i);
                cur = litEnd;
                continue;
            }
            /* function and variable case */
            for (size_t i = 0; i < GLANG_MAX_LIT_LEN && literal[i] != '\0'; ++i) {
                if (!(isdigit(literal[i]) || literal[i] == '_' || isalpha(literal[i]))) {
                    node->mode = gLang_Node_mode_unknown;
                    fprintf(stderr, "ERROR: bad function/variable name; it should only contain letters, digits and '_'!\n");
                    GLANG_ASSERT_LOG(false, gLang_status_ParsingErr_UnknownLex);
                }
            }

            while (isspace(*litEnd))
                ++litEnd;
            if (*litEnd == '(') {
                node->mode = gLang_Node_mode_func;
                strncpy(node->funcName, literal, GLANG_MAX_LIT_LEN);
            } else {
                node->mode = gLang_Node_mode_var;
                strncpy(node->varName, literal, GLANG_MAX_LIT_LEN);
            }
        }
        cur = litEnd;
    }


    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "LexemeIds = {");
        for (size_t i = 0; i < context->LexemeIds.len; ++i)
            fprintf(stderr, "%lu, ", context->LexemeIds.data[i]);
        fprintf(stderr, "}\n");
    #endif

    return gLang_status_OK;
}

static gLang_status gLang_parser_gram (gLang *context, size_t subRoot);

static gLang_status gLang_parser_stmnt(gLang *context, size_t subRoot);

static gLang_status gLang_parser_assig(gLang *context, size_t subRoot);

static gLang_status gLang_parser_if   (gLang *context, size_t subRoot);

static gLang_status gLang_parser_expr (gLang *context, size_t subRoot);

static gLang_status gLang_parser_prior(gLang *context, size_t subRoot);

static gLang_status gLang_parser_expn (gLang *context, size_t subRoot);

static gLang_status gLang_parser_term (gLang *context, size_t subRoot);

static gLang_status gLang_parser_var  (gLang *context, size_t subRoot);

static gLang_status gLang_parser_num  (gLang *context, size_t subRoot);

static gLang_status gLang_parser_func (gLang *context, size_t subRoot);


static gLang_status gLang_parser_gram(gLang *context, size_t subRoot)       //TODO add assertions
{
    while (context->lexemeCur < context->LexemeIds.len) {
        GLANG_IS_OK(gLang_parser_stmnt(context, subRoot));
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_stmnt(gLang *context, size_t subRoot)
{
    size_t *data = context->LexemeIds.data;
    gLang_Node *node = &(GLANG_NODE_BY_ID(data[context->lexemeCur])->data);
    if (node->mode == gLang_Node_mode_opFigBrack) {
        ++context->lexemeCur;
        GLANG_IS_OK(gLang_parser_stmnt(context, subRoot));
        node = &(GLANG_NODE_BY_ID(data[context->lexemeCur])->data)
        if (node->mode == gLang_Node_mode_clFigBrack) {
            ++context->lexemeCur;
        } else {
            fprintf(context->logStream, "ERROR: no closing figure brack for statement!\n");
            GLANG_ASSERT_LOG(false, gLang_status_ParsingErr_NoBrack);
        }
    } else {
        GLANG_IS_OK()       //TODO
    }
    return gLang_status_OK;
}


static gLang_status gLang_parser(gLang *context)
{
    GLANG_CHECK_SELF_PTR(context);
    size_t len = context->LexemeIds.len;

    GLANG_ASSERT_LOG(len != 0 && len < GLANG_LEX_LIM, gLang_status_EmptyLexer);

    return gLang_parser_gram(context, 0, len, context->tree.root);
}


static gLang_status gLang_optimize(gLang *context, const size_t rootId);
