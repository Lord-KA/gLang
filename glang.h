#pragma once

#define STACK_TYPE size_t
#define ELEM_PRINTF_FORM "%lu"
#define CHEAP_DEBUG

#include "gstack.h"

#include "commands.h"
#include "varpool.h"

#define T Command
#include "garray.h"
#undef T

static const size_t MAX_LINE_LEN = 1000;
static const size_t GLANG_MAX_LIT_LEN = 100;
static const size_t GLANG_LEX_LIM = 1000;

static const double GLANG_EPS = 1e-3;

enum gLang_Node_mode {
    gLang_Node_mode_none,
    gLang_Node_mode_add,
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
    gLang_Node_keyword_print,
    gLang_Node_keyword_while,
    gLang_Node_keyword_sqrt,
    gLang_Node_keyword_sin,
    gLang_Node_keyword_cos,
    gLang_Node_keyword_CNT,
};

static const char gLang_Node_keywordView[gLang_Node_keyword_CNT][GLANG_MAX_LIT_LEN] = {
        "if",
        "else",
        "return",
        "print",
        "while",
        "sqrt",
        "sin",
        "cos",
    };

struct gLang_Node
{
    gLang_Node_mode    mode;
    gLang_Node_keyword keyword;
    char funcName[GLANG_MAX_LIT_LEN];
    char  varName[GLANG_MAX_LIT_LEN];
    Var   *var;                                 /* To be filled by compiler; An intermediate structure that holds the position of var in register or memory */
    double value;
    size_t position;
} typedef gLang_Node;

static const char GLANG_DELIMS_LIST[] = " +-*/^(){};,<>=";

typedef gLang_Node GTREE_TYPE;

#include "gtree.h"

enum gLang_status {
    gLang_status_OK,
    gLang_status_NothingToDo,
    gLang_status_BadStructPtr,
    gLang_status_TreeErr,
    gLang_status_ObjPoolErr,
    gLang_status_AllocErr,
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
        "Error during allocation, ran out of memory?",
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

/*
 * WARNING: do not forget to undef new macro
 */

#ifndef NLOGS
#define GLANG_ASSERT_LOG(expr, errCode) ({                                           \
        bool macroRes = expr;                                                         \
        if (!macroRes && errCode >= gLang_status_ParsingErr_UnknownLex && errCode <= gLang_status_ParsingErr_NoSemicolon)\
            fprintf(ctx->logStream, "Error in lexeme No. %lu:\n", GLANG_CUR_NODE_ID());\
        if (errCode >= gLang_status_CNT || errCode < 0)  {                             \
            ASSERT_LOG(false, gLang_status_CNT,                                         \
                    gLang_statusMsg[gLang_status_CNT], ctx->logStream);              \
        }                                                                                 \
        ASSERT_LOG(macroRes, errCode, gLang_statusMsg[errCode], ctx->logStream);           \
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
    GLANG_ASSERT_LOG(gObjPool_get(&ctx->tree.pool, (macroId), &macroNode) == gObjPool_status_OK,       \
                            gLang_status_ObjPoolErr);                                                       \
    assert(gPtrValid(macroNode));                                                                            \
    macroNode;                                                                                                \
})


#define GLANG_POOL_ALLOC() ({                                                                 \
    size_t macroId = -1;                                                                       \
    gTree_Node *macroNode = NULL;                                                               \
    GLANG_ASSERT_LOG(gObjPool_alloc(&ctx->tree.pool, &macroId) == gObjPool_status_OK,        \
                            gLang_status_ObjPoolErr);                                             \
    GLANG_ASSERT_LOG(gObjPool_get(&ctx->tree.pool, macroId, &macroNode) == gObjPool_status_OK, \
                            gLang_status_ObjPoolErr);                                               \
    macroNode->sibling = -1;                                                                         \
    macroNode->parent  = -1;                                                                          \
    macroNode->child   = -1;                                                                           \
    macroId;                                                                                            \
})


#ifdef GLANG_NO_FREE
#define GLANG_POOL_FREE(macroId)
#else
#define GLANG_POOL_FREE(macroId) ({                                                          \
    GLANG_ASSERT_LOG(gObjPool_free(&ctx->tree.pool, macroId) == gObjPool_status_OK,       \
                            gLang_status_ObjPoolErr);                                          \
})
#endif

#define GLANG_TREE_CHECK(expr) ({                                             \
    GLANG_ASSERT_LOG((expr) == gTree_status_OK, gLang_status_TreeErr);         \
})


#define GLANG_ID_CHECK(id) GLANG_ASSERT_LOG(gObjPool_idValid(&ctx->tree.pool, id), gLang_status_BadId)

#define GLANG_IS_DELIM(macroCur) (*macroCur == '\0' || isspace(*macroCur) || strnConsistsChrs(macroCur, GLANG_DELIMS_LIST, 1, strlen(GLANG_DELIMS_LIST)))

#define GLANG_CUR_NODE() ({                                                                         \
    gLang_Node *macroNode = NULL;                                                                    \
    if (ctx->lexemeCur < ctx->LexemeIds.len)                                                  \
        macroNode = &(GLANG_NODE_BY_ID(ctx->LexemeIds.data[ctx->lexemeCur])->data);            \
    macroNode;                                                                                          \
})

#define GLANG_CUR_NODE_ID() ({                                                                      \
    size_t macroNodeId = -1;                                                                         \
    if (ctx->lexemeCur < ctx->LexemeIds.len)                                                  \
        macroNodeId = ctx->LexemeIds.data[ctx->lexemeCur];                                     \
    macroNodeId;                                                                                        \
})

#ifdef EXTRA_VERBOSE
#define GLANG_PARSER_CHECK() ({                                                                 \
    fprintf(ctx->logStream, "%s curLit = %lu\n", __func__, GLANG_CUR_NODE_ID());             \
    GLANG_CHECK_SELF_PTR(ctx);                                                                \
    GLANG_ID_CHECK(rootId);                                                                        \
    GLANG_ASSERT_LOG(rootId != -1, gLang_status_ParsingErr_BadRootId);                              \
    GLANG_ASSERT_LOG(ctx->lexemeCur < ctx->LexemeIds.len, gLang_status_ParsingErr_BadCur);   \
})
#else
#define GLANG_PARSER_CHECK() ({                                                                 \
    GLANG_CHECK_SELF_PTR(ctx);                                                               \
    GLANG_ID_CHECK(rootId);                                                                       \
    GLANG_ASSERT_LOG(rootId != -1, gLang_status_ParsingErr_BadRootId);                             \
    GLANG_ASSERT_LOG(ctx->lexemeCur < ctx->LexemeIds.len, gLang_status_ParsingErr_BadCur);  \
})
#endif


struct gLang {
    gTree       tree      = {};
    FILE       *logStream = {};
    FILE       *asmOut    = {};
    const char *buffer    = {};
    size_t      labelCnt  = {};
    GENERIC(stack) LexemeIds = {};
    size_t         lexemeCur = {};
    varPool       **varTables    = {};
    size_t          varTablesCur = {};
    size_t          varTablesLen = {};
    gArr  *commands = {};
} typedef gLang;


gLang_status gLang_ctor(gLang *ctx, FILE *newLogStream);

gLang_status gLang_dtor(gLang *ctx);


gLang_status gLang_lexer(gLang *ctx, const char *buffer);


gLang_status gLang_parser(gLang *ctx);

static gLang_status gLang_parser_funcDef(gLang *ctx, size_t rootId);

static gLang_status gLang_parser_gram (gLang *ctx, size_t rootId);

static gLang_status gLang_parser_stmnt(gLang *ctx, size_t rootId);

static gLang_status gLang_parser_assig(gLang *ctx, size_t rootId);

static gLang_status gLang_parser_if   (gLang *ctx, size_t rootId);

static gLang_status gLang_parser_expr (gLang *ctx, size_t rootId);

static gLang_status gLang_parser_cmp  (gLang *ctx, size_t rootId);

static gLang_status gLang_parser_prior(gLang *ctx, size_t rootId);

static gLang_status gLang_parser_expn (gLang *ctx, size_t rootId);

static gLang_status gLang_parser_term (gLang *ctx, size_t rootId);

static gLang_status gLang_parser_func (gLang *ctx, size_t rootId);

static gLang_status gLang_parser_while(gLang *ctx, size_t rootId);

static gLang_status gLang_parser_blk  (gLang *ctx, size_t rootId);

static gLang_status gLang_parser_retrn(gLang *ctx, size_t rootId);

static gLang_status gLang_parser_print(gLang *ctx, size_t rootId);


gLang_status gLang_optimize(gLang *ctx, const size_t rootId);


static gLang_status gLang_fillVarTable(gLang *ctx, size_t rootId);


static gLang_status gLang_compileExpr(gLang *ctx, size_t rootId);

static gLang_status gLang_compileStmnt(gLang *ctx, size_t rootId);

static gLang_status gLang_compileBlk(gLang *ctx, size_t siblingId);

static gLang_status gLang_getArgs(gLang *ctx, size_t siblingId);

gLang_status gLang_compile(gLang *ctx, FILE *out);

gLang_status gLang_commandsDump(gLang *ctx, FILE *out);
