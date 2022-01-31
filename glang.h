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
    gLang_Node_mode_semicolon,
    gLang_Node_mode_comma,
    gLang_Node_mode_less,
    gLang_Node_mode_great,
    gLang_Node_mode_assign,
    gLang_Node_mode_var,
    gLang_Node_mode_func,
    gLang_Node_mode_num,
    gLang_Node_mode_keyword,
    gLang_Node_mode_func_args,
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
        ",",
        "<",
        ">",
        "=",
        "VAR",
        "FUNC",
        "NUM",
        "KEYWORD",
        "FUNC_ARGS",
        "UNKNOWN",
    };

enum gLang_Node_keyword {
    gLang_Node_keyword_if,
    gLang_Node_keyword_else,
    gLang_Node_keyword_return,
    gLang_Node_keyword_while,
    gLang_Node_keyword_CNT,
};

static const char gLang_Node_keywordView[gLang_Node_keyword_CNT][GLANG_MAX_LIT_LEN] = {
        "if",
        "else",
        "return",
        "while",
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

static const char GLANG_DELIMS_LIST[] = " +-*/^(){};,<>=";

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
    if (data.mode >= gLang_Node_mode_sum && data.mode <= gLang_Node_mode_assign)
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
    gLang_status_NothingToDo,
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
    gLang_status_ParsingErr_BadCur,
    gLang_status_ParsingErr_BadRootId,
    gLang_status_ParsingErr_EmptyOutp,
    gLang_status_ParsingErr_NoSemicolon,
    gLang_status_DecompositionErr,
    gLang_status_CNT,
};

static const char gLang_statusMsg[gLang_status_CNT + 1][MAX_LINE_LEN] = {
        "OK",
        "Parsing block has not detected the pattern",
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
        "Parsing error: bad cursor recieved by parser",
        "Parsing error: bad rootId provided to parser subfunc",
        "Parsing error: no semicolon at the end of a statement",
        "Some error during tree decomposition",
        "UNKNOWN ERROR CODE!",
    };

#ifndef NLOGS               //TODO
#define GLANG_ASSERT_LOG(expr, errCode) ({                                           \
        bool macroRes = expr;                                                         \
        if (!macroRes && errCode >= gLang_status_ParsingErr_UnknownLex && errCode <= gLang_status_ParsingErr_NoSemicolon)\
            fprintf(context->logStream, "Error in lexeme No. %lu:\n", GLANG_CUR_NODE_ID());\
        if (errCode >= gLang_status_CNT || errCode < 0)  {                             \
            ASSERT_LOG(false, gLang_status_CNT,                                         \
                    gLang_statusMsg[gLang_status_CNT], context->logStream);              \
        }                                                                                 \
        ASSERT_LOG(macroRes, errCode, gLang_statusMsg[errCode], context->logStream);           \
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
    macroNode->sibling = -1;                                                                         \
    macroNode->parent  = -1;                                                                          \
    macroNode->child   = -1;                                                                           \
    macroId;                                                                                            \
})

// #define NO_FREE //TODO

#ifdef NO_FREE
#define GLANG_POOL_FREE(id)
#else
#define GLANG_POOL_FREE(macroId) ({                                                          \
    GLANG_ASSERT_LOG(gObjPool_free(&context->tree.pool, macroId) == gObjPool_status_OK,       \
                            gLang_status_ObjPoolErr);                                          \
})
#endif

#define GLANG_TREE_CHECK(expr) ({                                             \
    GLANG_ASSERT_LOG((expr) == gTree_status_OK, gLang_status_TreeErr);         \
})


#define GLANG_ID_CHECK(id) GLANG_ASSERT_LOG(gObjPool_idValid(&context->tree.pool, id), gLang_status_BadId)

#define GLANG_IS_DELIM(macroCur) (*macroCur == '\0' || isspace(*macroCur) || strnConsistsChrs(macroCur, GLANG_DELIMS_LIST, 1, strlen(GLANG_DELIMS_LIST)))

#define GLANG_CUR_NODE() ({                                                                     \
    gLang_Node *macroNode = NULL;                                                                     \
    if (context->lexemeCur < context->LexemeIds.len)                                            \
        macroNode = &(GLANG_NODE_BY_ID(context->LexemeIds.data[context->lexemeCur])->data);          \
    macroNode;                                                                                       \
})

#define GLANG_CUR_NODE_ID() ({                                                                     \
    size_t macroNodeId = -1;                                                                         \
    if (context->lexemeCur < context->LexemeIds.len)                                                    \
        macroNodeId = context->LexemeIds.data[context->lexemeCur];                                  \
    macroNodeId;                                                                                           \
})


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


static gLang_status gLang_parser_funcDef(gLang *context, size_t rootId);

static gLang_status gLang_parser_gram (gLang *context, size_t rootId);

static gLang_status gLang_parser_stmnt(gLang *context, size_t rootId);

static gLang_status gLang_parser_assig(gLang *context, size_t rootId);

static gLang_status gLang_parser_if   (gLang *context, size_t rootId);

static gLang_status gLang_parser_expr (gLang *context, size_t rootId);

static gLang_status gLang_parser_cmp  (gLang *context, size_t rootId);

static gLang_status gLang_parser_prior(gLang *context, size_t rootId);

static gLang_status gLang_parser_expn (gLang *context, size_t rootId);

static gLang_status gLang_parser_term (gLang *context, size_t rootId);

static gLang_status gLang_parser_func (gLang *context, size_t rootId);

static gLang_status gLang_parser_while(gLang *context, size_t rootId);

static gLang_status gLang_parser_blk  (gLang *context, size_t rootId);

static gLang_status gLang_parser_ret  (gLang *context, size_t rootId);

#define GLANG_PARSER_CHECK() ({\
    fprintf(stderr, "%s curLit = %lu\n", __func__, GLANG_CUR_NODE_ID());\
    GLANG_CHECK_SELF_PTR(context);\
    GLANG_ASSERT_LOG(rootId != -1, gLang_status_ParsingErr_BadRootId);\
    GLANG_ASSERT_LOG(context->lexemeCur < context->LexemeIds.len, gLang_status_ParsingErr_BadCur);   \
})

static gLang_status gLang_parser_funcDef(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    size_t lastChildId = -1;
    while (context->lexemeCur < context->LexemeIds.len) {
        GLANG_IS_OK(gLang_parser_func(context, rootId));
        if (lastChildId == -1)
            lastChildId = GLANG_NODE_BY_ID(rootId)->child;
        else
            lastChildId = GLANG_NODE_BY_ID(lastChildId)->sibling;

        GLANG_IS_OK(gLang_parser_blk(context, lastChildId));
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_gram(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_status status = gLang_status_OK;
    while (status != gLang_status_NothingToDo) {
        fprintf(stderr, "GRAM 1 curNode = %lu\n", GLANG_CUR_NODE_ID());
        status = gLang_parser_stmnt(context, rootId);
        fprintf(stderr, "GRAM 2 curNode = %lu\n", GLANG_CUR_NODE_ID());
        GLANG_ASSERT_LOG(status == gLang_status_OK || status == gLang_status_NothingToDo, status);
    }
    return gLang_status_OK;
}


static gLang_status gLang_parser_stmnt(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    size_t *data = context->LexemeIds.data;
    gLang_Node *node = GLANG_CUR_NODE();

    fprintf(stderr, "STMNT 0 curNode = %lu\n", GLANG_CUR_NODE_ID());
    gLang_status status = gLang_parser_if(context, rootId);
    if (status == gLang_status_OK)
        goto finish;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_while(context, rootId);
    if (status == gLang_status_OK)
        return gLang_status_OK;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_ret(context, rootId);
    if (status == gLang_status_OK)
        goto finish;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_assig(context, rootId);
    if (status == gLang_status_OK)
        goto finish;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_blk(context, rootId);
    if (status == gLang_status_OK)
        return gLang_status_OK;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_expr(context, rootId);
    if (status == gLang_status_OK)
        goto finish;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    return gLang_status_NothingToDo;

finish:
    fprintf(stderr, "STMNT 3 curNode = %lu\n", GLANG_CUR_NODE_ID());
    node = GLANG_CUR_NODE();
    if (node->mode == gLang_Node_mode_semicolon) {
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        ++context->lexemeCur;
    } else {
        fprintf(context->logStream, "ERROR: no semicolon at the end of the line!\n");   //TODO add position outp
        GLANG_ASSERT_LOG(false, gLang_status_ParsingErr_NoSemicolon);
    }
    return gLang_status_OK;
}


static gLang_status gLang_parser_assig(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    if (context->lexemeCur + 2 >= context->LexemeIds.len)
        return gLang_status_NothingToDo;

    size_t varId = GLANG_CUR_NODE_ID();
    gLang_Node *node = GLANG_CUR_NODE();

    if (node->mode != gLang_Node_mode_var)
        return gLang_status_NothingToDo;

    context->lexemeCur += 1;
    size_t assignId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    if (node == NULL || node->mode != gLang_Node_mode_assign) {
        context->lexemeCur -= 1;
        return gLang_status_NothingToDo;
    }

    GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId,   assignId));
    GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, assignId, varId));
    context->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(context, assignId));
    return gLang_status_OK;
}

static gLang_status gLang_parser_if(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    if (context->lexemeCur + 3 >= context->LexemeIds.len)
        return gLang_status_NothingToDo;

    size_t ifId = GLANG_CUR_NODE_ID();
    gLang_Node *node = GLANG_CUR_NODE();

    if (node->mode != gLang_Node_mode_keyword || node->keyword != gLang_Node_keyword_if)
        return gLang_status_NothingToDo;

    context->lexemeCur += 1;
    size_t opBrackId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_opBrack, gLang_status_ParsingErr_NoBrack);

    GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, ifId));
    context->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(context, ifId));

    size_t clBrackId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
    context->lexemeCur += 1;

    GLANG_IS_OK(gLang_parser_stmnt(context, ifId));

    GLANG_POOL_FREE(opBrackId);
    GLANG_POOL_FREE(clBrackId);
    return gLang_status_OK;
}


static gLang_status gLang_parser_expr(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();

    size_t tmpId = GLANG_POOL_ALLOC();

    gLang_status status = gLang_parser_cmp(context, tmpId);
    if (status == gLang_status_NothingToDo) {
        GLANG_POOL_FREE(tmpId);
        return status;
    } else {
        GLANG_IS_OK(status);
    }

    if (context->lexemeCur < context->LexemeIds.len &&
            GLANG_CUR_NODE()->mode >= gLang_Node_mode_less && GLANG_CUR_NODE()->mode < gLang_Node_mode_assign) {
        size_t cmpId = GLANG_CUR_NODE_ID();
        context->lexemeCur += 1;

        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, cmpId));
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, cmpId,  GLANG_NODE_BY_ID(tmpId)->child));
        GLANG_IS_OK(gLang_parser_cmp(context, cmpId));
    } else {
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, GLANG_NODE_BY_ID(tmpId)->child));
    }
    GLANG_POOL_FREE(tmpId);
    return gLang_status_OK;
}

static gLang_status gLang_parser_cmp(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();

    size_t sumId = GLANG_POOL_ALLOC();
    GLANG_NODE_BY_ID(sumId)->data.mode = gLang_Node_mode_sum;

    gLang_status status = gLang_parser_term(context, sumId);
    if (status == gLang_status_NothingToDo) {
        GLANG_POOL_FREE(sumId);
        return status;
    } else {
        GLANG_IS_OK(status);
    }

    gLang_Node *node = GLANG_CUR_NODE();
    while (node != NULL && (node->mode == gLang_Node_mode_sum || node->mode == gLang_Node_mode_sub)) {
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        context->lexemeCur += 1;
        if (node->mode == gLang_Node_mode_sum) {
            GLANG_IS_OK(gLang_parser_term(context, sumId));
        } else {
            size_t mulId = GLANG_POOL_ALLOC();
            size_t oneId = GLANG_POOL_ALLOC();
            gLang_Node *mulNode = &GLANG_NODE_BY_ID(mulId)->data;
            gLang_Node *oneNode = &GLANG_NODE_BY_ID(oneId)->data;
            mulNode->mode = gLang_Node_mode_mul;
            oneNode->mode = gLang_Node_mode_num;
            oneNode->value = -1;

            GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, mulId, oneId));
            GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, sumId, mulId));
            GLANG_IS_OK(gLang_parser_term(context, mulId));
        }
        node = GLANG_CUR_NODE();
    }
    size_t childId = GLANG_NODE_BY_ID(sumId)->child;
    if (GLANG_NODE_BY_ID(childId)->sibling == -1) {
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, childId));
        GLANG_POOL_FREE(sumId);
    } else {
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, sumId));
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_term(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();

    size_t mulId = GLANG_POOL_ALLOC();
    GLANG_NODE_BY_ID(mulId)->data.mode = gLang_Node_mode_mul;

    gLang_status status = gLang_parser_expn(context, mulId);
    if (status == gLang_status_NothingToDo) {
        GLANG_POOL_FREE(mulId);
        return status;
    } else {
        GLANG_IS_OK(status);
    }

    gLang_Node *node = GLANG_CUR_NODE();
    while (node != NULL && (node->mode == gLang_Node_mode_mul || node->mode == gLang_Node_mode_div)) {
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        context->lexemeCur += 1;
        if (node->mode == gLang_Node_mode_mul) {
            GLANG_IS_OK(gLang_parser_expn(context, mulId));
        } else {
            size_t expId = GLANG_POOL_ALLOC();
            size_t oneId = GLANG_POOL_ALLOC();
            gLang_Node *expNode = &GLANG_NODE_BY_ID(expId)->data;
            gLang_Node *oneNode = &GLANG_NODE_BY_ID(oneId)->data;
            expNode->mode = gLang_Node_mode_exp;
            oneNode->mode = gLang_Node_mode_num;
            oneNode->value = -1;

            GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, expId, oneId));
            GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, mulId, expId));
            GLANG_IS_OK(gLang_parser_expn(context, expId));
        }
        node = GLANG_CUR_NODE();
    }
    size_t childId = GLANG_NODE_BY_ID(mulId)->child;
    if (GLANG_NODE_BY_ID(childId)->sibling == -1) {
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, childId));
        GLANG_POOL_FREE(mulId);
    } else {
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, mulId));
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_expn(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();

    size_t expId = GLANG_POOL_ALLOC();
    GLANG_NODE_BY_ID(expId)->data.mode = gLang_Node_mode_exp;

    gLang_status status = gLang_parser_prior(context, expId);
    if (status == gLang_status_NothingToDo) {
        GLANG_POOL_FREE(expId);
        return status;
    } else {
        GLANG_IS_OK(status);
    }

    gLang_Node *node = GLANG_CUR_NODE();
    if (node != NULL && node->mode == gLang_Node_mode_exp) {
        context->lexemeCur += 1;

        GLANG_IS_OK(gLang_parser_prior(context, expId));
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, expId));
    } else {
        size_t childId = GLANG_NODE_BY_ID(expId)->child;
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, childId));
        GLANG_POOL_FREE(expId);
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_prior(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    if (node->mode == gLang_Node_mode_opBrack) {
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        context->lexemeCur += 1;
        GLANG_IS_OK(gLang_parser_expn(context, rootId));
        node = GLANG_CUR_NODE();
        GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);
        GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        context->lexemeCur += 1;
    } else if (node->mode == gLang_Node_mode_num || node->mode == gLang_Node_mode_var) {
        fprintf(stderr, "PRIOR 1 curNode = %lu\n", GLANG_CUR_NODE_ID());
        GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, GLANG_CUR_NODE_ID()));
        context->lexemeCur += 1;
    } else if (node->mode == gLang_Node_mode_func) {
        fprintf(stderr, "FUNC call\n");
        GLANG_IS_OK(gLang_parser_func(context, rootId));
    } else {
        return gLang_status_NothingToDo;
    }

    fprintf(stderr, "PRIOR 2 curNode = %lu\n", GLANG_CUR_NODE_ID());
    return gLang_status_OK;
}

static gLang_status gLang_parser_func(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    size_t funcId = GLANG_CUR_NODE_ID();

    if (node->mode != gLang_Node_mode_func)
        return gLang_status_NothingToDo;

    ++context->lexemeCur;
    size_t opBrackId = GLANG_CUR_NODE_ID();
    node = &GLANG_NODE_BY_ID(opBrackId)->data;
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_opBrack, gLang_status_ParsingErr_UnknownLex);
    GLANG_POOL_FREE(GLANG_CUR_NODE_ID());

    size_t argsId = GLANG_POOL_ALLOC();
    GLANG_NODE_BY_ID(argsId)->data.mode = gLang_Node_mode_func_args;
    GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, funcId, argsId));

    ++context->lexemeCur;
    node = GLANG_CUR_NODE();

    while (node != NULL && node->mode != gLang_Node_mode_clBrack) {
        if (node->mode != gLang_Node_mode_comma) {
            GLANG_IS_OK(gLang_parser_expr(context, argsId));
        } else {
            GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
            ++context->lexemeCur;
        }

        node = GLANG_CUR_NODE();
    }
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
    GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
    context->lexemeCur += 1;

    GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, funcId));

    return gLang_status_OK;
}

static gLang_status gLang_parser_blk  (gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    size_t funcId = GLANG_CUR_NODE_ID();

    if (node->mode != gLang_Node_mode_opFigBrack)
        return gLang_status_NothingToDo;

    GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
    ++context->lexemeCur;

    GLANG_IS_OK(gLang_parser_gram(context, rootId));

    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node != NULL && node->mode == gLang_Node_mode_clFigBrack, gLang_status_ParsingErr_NoBrack);
    GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
    ++context->lexemeCur;

    return gLang_status_OK;
}


static gLang_status gLang_parser_ret(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    size_t retId = GLANG_CUR_NODE_ID();

    if (node->mode != gLang_Node_mode_keyword || node->keyword != gLang_Node_keyword_return)
        return gLang_status_NothingToDo;

    context->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(context, retId));

    GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, retId));

    return gLang_status_OK;
}

static gLang_status gLang_parser_while(gLang *context, size_t rootId)
{
    GLANG_PARSER_CHECK();
    if (context->lexemeCur + 3 >= context->LexemeIds.len)
        return gLang_status_NothingToDo;

    size_t whileId = GLANG_CUR_NODE_ID();
    gLang_Node *node = GLANG_CUR_NODE();

    if (node->mode != gLang_Node_mode_keyword || node->keyword != gLang_Node_keyword_while)
        return gLang_status_NothingToDo;

    context->lexemeCur += 1;
    size_t opBrackId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_opBrack, gLang_status_ParsingErr_NoBrack);

    GLANG_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, whileId));
    context->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(context, whileId));

    size_t clBrackId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
    context->lexemeCur += 1;

    GLANG_IS_OK(gLang_parser_stmnt(context, whileId));

    GLANG_POOL_FREE(opBrackId);
    GLANG_POOL_FREE(clBrackId);
    return gLang_status_OK;
}

static gLang_status gLang_parser(gLang *context)
{
    GLANG_CHECK_SELF_PTR(context);
    size_t len = context->LexemeIds.len;
    context->lexemeCur = 0;

    GLANG_ASSERT_LOG(len != 0 && len < GLANG_LEX_LIM, gLang_status_EmptyLexer);
    GLANG_IS_OK(gLang_parser_funcDef(context, context->tree.root));

    return gLang_status_OK;
}
