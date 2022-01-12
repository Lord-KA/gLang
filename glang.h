#pragma once

#define STACK_TYPE size_t
#define ELEM_PRINTF_FORM "%lu"
#define CHEAP_DEBUG

#include "gstack.h"

enum gDerivator_Node_mode {
    gDerivator_Node_mode_none,
    gDerivator_Node_mode_sum,
    gDerivator_Node_mode_sub,
    gDerivator_Node_mode_mul,
    gDerivator_Node_mode_div,
    gDerivator_Node_mode_exp,
    gDerivator_Node_mode_opBrack,
    gDerivator_Node_mode_clBrack,
    gDerivator_Node_mode_var,
    gDerivator_Node_mode_func,
    gDerivator_Node_mode_num,
    gDerivator_Node_mode_unknown,
    gDerivator_Node_mode_CNT,
};

static const char gDerivator_Node_modeView[gDerivator_Node_mode_CNT][10] = {
        "NONE",
        "+",
        "-",
        "*",
        "/",
        "^",
        "(",
        ")",
        "x",
        "FUNC",
        "NUM",
        "UNKNOWN",
    };

static const char gDerivator_Node_modeViewLatex[gDerivator_Node_mode_CNT][10] = {
        "NONE",
        "+",
        "-",
        "\\cdot",
        "\\frac",
        "^",
        "(",
        ")",
        "x",
        "FUNC",
        "NUM",
        "UNKNOWN",
    };

enum gDerivator_Node_func {
    gDerivator_Node_func_sin,
    gDerivator_Node_func_cos,
    gDerivator_Node_func_ln,
    gDerivator_Node_func_log,
    gDerivator_Node_func_CNT,
};

static const char gDerivator_Node_funcView[gDerivator_Node_func_CNT][10] = {
        "sin",
        "cos",
        "ln",
        "log",
    };

struct gDerivator_Node 
{
    gDerivator_Node_mode mode;
    gDerivator_Node_func func;
    double value;
    size_t position;
} typedef gDerivator_Node;

static const char GDERIVATOR_DELIMS_LIST[] = " +-*/^()";

typedef gDerivator_Node GTREE_TYPE;

#include "gtree.h"

static const size_t MAX_LINE_LEN = 1000;
static const size_t GDERIVATOR_MAX_LIT_LEN = 100;
static const size_t GDERIVATOR_LEX_LIM = 1000;

static const double GDERIVATOR_EPS = 1e-3;

bool gTree_storeData(gDerivator_Node data, size_t level, FILE *out) //TODO
{
    return 0;
}

bool gTree_restoreData(gDerivator_Node *data, FILE *in) //TODO
{
    return 0;
}

bool gTree_printData(gDerivator_Node data, FILE *out)
{
    assert(gPtrValid(out));
    if (data.mode >= gDerivator_Node_mode_CNT || data.mode < 0)
        data.mode = gDerivator_Node_mode_unknown;
    fprintf(out, "{mode | %d (\'%s\')}", data.mode, gDerivator_Node_modeView[data.mode]);

    if (data.mode == gDerivator_Node_mode_num)
        fprintf(out, "| {%lf} ", data.value);
    else if (data.mode == gDerivator_Node_mode_func)
        fprintf(out, "| {\'%s\'} ", gDerivator_Node_funcView[data.func]);

    return 0;
}

enum gDerivator_status {
    gDerivator_status_OK,  
    gDerivator_status_BadStructPtr,
    gDerivator_status_TreeErr,
    gDerivator_status_ObjPoolErr,
    gDerivator_status_FileErr,
    gDerivator_status_BadPtr,
    gDerivator_status_BadId,
    gDerivator_status_BadInput,
    gDerivator_status_EmptyLexer,
    gDerivator_status_EmptyTree,
    gDerivator_status_ParsingErr_UnknownLex,
    gDerivator_status_ParsingErr_NoBrack,
    gDerivator_status_DecompositionErr,
    gDerivator_status_CNT,
};

static const char gDerivator_statusMsg[gDerivator_status_CNT + 1][MAX_LINE_LEN] = {
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
#define GDERIVATOR_ASSERT_LOG(expr, errCode) ({                                         \
        if (errCode >= gDerivator_status_CNT || errCode < 0)  {                          \
            ASSERT_LOG(false, gDerivator_status_CNT,                                      \
                    gDerivator_statusMsg[gDerivator_status_CNT], context->logStream);      \
        }                                                                                   \
        ASSERT_LOG(expr, errCode, gDerivator_statusMsg[errCode], context->logStream);        \
    })
#else
#define GDERIVATOR_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gDerivator_statusMsg[errCode], NULL)
#endif

#define GDERIVATOR_CHECK_SELF_PTR(ptr) ASSERT_LOG(gPtrValid(ptr), gDerivator_status_BadStructPtr,     \
                                                 gDerivator_statusMsg[gDerivator_status_BadStructPtr], \
                                                 stderr)
#define GDERIVATOR_IS_OK(expr) ({                                                 \
        gDerivator_status macroStatus = (expr);                                    \
        GDERIVATOR_ASSERT_LOG(macroStatus == gDerivator_status_OK, macroStatus);    \
    })

#define GDERIVATOR_NODE_BY_ID(macroId) ({                                                               \
    assert(macroId != -1);                                                                               \
    gTree_Node *macroNode = NULL;                                                                         \
    GDERIVATOR_ASSERT_LOG(gObjPool_get(&context->tree.pool, (macroId), &macroNode) == gObjPool_status_OK,  \
                            gDerivator_status_ObjPoolErr);                                                  \
    assert(gPtrValid(macroNode));                                                                            \
    macroNode;                                                                                                \
})                                                                   


#define GDERIVATOR_POOL_ALLOC() ({                                                                 \
    size_t macroId = -1;                                                                            \
    gTree_Node *macroNode = NULL;                                                                    \
    GDERIVATOR_ASSERT_LOG(gObjPool_alloc(&context->tree.pool, &macroId) == gObjPool_status_OK,        \
                            gDerivator_status_ObjPoolErr);                                             \
    GDERIVATOR_ASSERT_LOG(gObjPool_get(&context->tree.pool, macroId, &macroNode) == gObjPool_status_OK, \
                            gDerivator_status_ObjPoolErr);                                               \
      macroNode->sibling = -1;                                                                            \
      macroNode->parent  = -1;                                                                             \
      macroNode->child   = -1;                                                                              \
      macroId;                                                                                               \
})

#define GDERIVATOR_POOL_FREE(id) ({                                                           \
    GDERIVATOR_ASSERT_LOG(gObjPool_free(&context->tree.pool, id) == gObjPool_status_OK,        \
                            gDerivator_status_ObjPoolErr);                                      \
})


#define GDERIVATOR_TREE_CHECK(expr) ({                                             \
    GDERIVATOR_ASSERT_LOG((expr) == gTree_status_OK, gDerivator_status_TreeErr);    \
})


#define GDERIVATOR_ID_CHECK(id) GDERIVATOR_ASSERT_LOG(gObjPool_idValid(&context->tree.pool, id), gDerivator_status_BadId)


struct gDerivator {
    gTree tree;
    FILE *logStream;
    GENERIC(stack) LexemeIds = {};
    const char *buffer;
} typedef gDerivator;


static gDerivator_status gDerivator_ctor(gDerivator *context, FILE *newLogStream)
{
    if (!gPtrValid(context)) {                                          
        FILE *out;                                                   
        if (!gPtrValid(newLogStream)) 
            out = stderr;                                            
        else                                                         
            out = newLogStream;                                      
        fprintf(out, "ERROR: bad structure ptr provided to derivator ctor!\n");
        return gDerivator_status_BadStructPtr;                         
    }
    
    context->logStream = stderr;
    if (gPtrValid(newLogStream))
        context->logStream = newLogStream;

    gTree_status status = gTree_ctor(&context->tree, context->logStream);
    GDERIVATOR_ASSERT_LOG(status == gTree_status_OK, gDerivator_status_TreeErr);

    GENERIC(stack_ctor)(&context->LexemeIds);

    return gDerivator_status_OK;
}

static gDerivator_status gDerivator_dtor(gDerivator *context)
{
    GDERIVATOR_CHECK_SELF_PTR(context);

    gTree_status status = gTree_dtor(&context->tree);
    GDERIVATOR_ASSERT_LOG(status == gTree_status_OK, gDerivator_status_TreeErr);

    GENERIC(stack_dtor)(&context->LexemeIds);

    return gDerivator_status_OK;
}

static gDerivator_status gDerivator_lexer(gDerivator *context, const char *buffer)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    GDERIVATOR_ASSERT_LOG(gPtrValid(buffer), gDerivator_status_BadPtr);

    context->buffer = buffer;
    char *cur = (char*)buffer;
    size_t id = -1;
    gDerivator_Node *node = NULL;
    GENERIC(stack_clear)(&context->LexemeIds);

    while (*cur != '\0' && *cur != '\n') {
        #ifdef EXTRA_VERBOSE
            fprintf(stderr, "cur = %s\n", cur);
        #endif  
        if (isspace(*cur)) {
            ++cur;
            continue;
        }
        id   = GDERIVATOR_POOL_ALLOC();
        node = &(GDERIVATOR_NODE_BY_ID(id)->data);
        GENERIC(stack_push)(&context->LexemeIds, id);
        node->position = cur - buffer;

        bool foundLit = false;
        for (size_t i = gDerivator_Node_mode_sum; i <= gDerivator_Node_mode_var; ++i) {                 //TODO make vars universal
            if (*gDerivator_Node_modeView[i] == *cur) {
                node->mode = (gDerivator_Node_mode)i;
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

        bool isFunc = false;                                                                              //TODO switch to Trim alg
        for (size_t i = 0; i < gDerivator_Node_func_CNT; ++i) {
            if (strncmp(cur, gDerivator_Node_funcView[i], strlen(gDerivator_Node_funcView[i])) == 0) {  //TODO check if '\n' is a valid problem
                cur += strlen(gDerivator_Node_funcView[i]);
                node->mode = gDerivator_Node_mode_func;
                node->func = (gDerivator_Node_func)i;
                isFunc = true;
                break;
            }
        }
        if (isFunc && *cur != '\0' && *cur != '\n' && !strnConsistsChrs(cur, GDERIVATOR_DELIMS_LIST, 1, strlen(GDERIVATOR_DELIMS_LIST))) {
            node->mode = gDerivator_Node_mode_unknown;
            fprintf(stderr, "ERROR: Unknown word has been found in func!\n");
            ++cur;
        }
        if (isFunc) {
            #ifdef EXTRA_VERBOSE
                fprintf(stderr, "Found func!");
                fprintf(stderr, " cur = #%s#\n", cur);
            #endif
            continue;
        }

        char *litEnd = cur;
        while (!strnConsistsChrs(litEnd, GDERIVATOR_DELIMS_LIST, 1, strlen(GDERIVATOR_DELIMS_LIST)) && *litEnd != '\0' && *litEnd != '\n') 
            ++litEnd;
        char literal[GDERIVATOR_MAX_LIT_LEN] = "";
        strncpy(literal, cur, litEnd - cur);
        if (isInteger(literal)) {
            node->mode = gDerivator_Node_mode_num;
            node->value = (double)(strtol(literal, NULL, 0));
            cur = litEnd;
            continue;
        } else if (isDouble(literal)) {
            node->mode = gDerivator_Node_mode_num;
            node->value = strtod(literal, NULL);
            cur = litEnd;
            continue;
        }
        node->mode = gDerivator_Node_mode_unknown;
        fprintf(stderr, "ERROR: Unknown word has been found!\n");
        *cur;
        ++cur;
    }

    
    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "LexemeIds = {");
        for (size_t i = 0; i < context->LexemeIds.len; ++i) 
            fprintf(stderr, "%lu, ", context->LexemeIds.data[i]);
        fprintf(stderr, "}\n");
    #endif
    
    return gDerivator_status_OK;
}


static gDerivator_status gDerivator_parser_expr (gDerivator *context, const size_t start, const size_t end, const size_t subRoot);

static gDerivator_status gDerivator_parser_prior(gDerivator *context, const size_t start, const size_t end, const size_t subRoot);

static gDerivator_status gDerivator_parser_expn (gDerivator *context, const size_t start, const size_t end, const size_t subRoot);

static gDerivator_status gDerivator_parser_term (gDerivator *context, const size_t start, const size_t end, const size_t subRoot);


static gDerivator_status gDerivator_parser(gDerivator *context)
{ 
    GDERIVATOR_CHECK_SELF_PTR(context);
    size_t len = context->LexemeIds.len;

    GDERIVATOR_ASSERT_LOG(len != 0 && len < GDERIVATOR_LEX_LIM, gDerivator_status_EmptyLexer);      

    return gDerivator_parser_expr(context, 0, len, context->tree.root);
}


static gDerivator_status gDerivator_parser_expr(gDerivator *context, 
                                                const size_t start, 
                                                const size_t end, 
                                                const size_t subRoot)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    GDERIVATOR_ID_CHECK(subRoot);
    assert(start <  context->LexemeIds.len);
    assert(end   <= context->LexemeIds.len);
    assert(start <= end);

    gDerivator_status status = gDerivator_status_OK;

    size_t subStart = -1, subEnd = -1;
    size_t *data = context->LexemeIds.data;
    long brackCnt = 0;

    size_t firstPos = -1;
    for (size_t i = start; i < end; ++i) {
        gDerivator_Node *node = &(GDERIVATOR_NODE_BY_ID(data[i])->data);
        if (node->mode == gDerivator_Node_mode_opBrack) 
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack) 
            --brackCnt;

        if ((node->mode == gDerivator_Node_mode_sum || node->mode == gDerivator_Node_mode_sub) && brackCnt == 0) {
            if (firstPos == -1)
                firstPos = i;
            else 
                firstPos = -2;
        }
    }

    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "start = %lu\nend = %lu\nfirstPos = %lu\n", start, end, firstPos);
    #endif
    if (firstPos == -1) {
        return gDerivator_parser_term(context, start, end, subRoot);
    } else if (firstPos != -2) {
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subRoot, data[firstPos]));

        status = gDerivator_parser_term(context, start,        firstPos, data[firstPos]);
        GDERIVATOR_IS_OK(status);   
        status = gDerivator_parser_term(context, firstPos + 1, end,      data[firstPos]);
        GDERIVATOR_IS_OK(status);
        return status;
    }

    size_t sumRoot = -1;
    bool isSum = true;
    GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, subRoot, &sumRoot, 
                gDerivator_Node{gDerivator_Node_mode_sum, gDerivator_Node_func_CNT, 0, 0}));
    brackCnt = 0;
    for (subStart = subEnd = start; subEnd < end; ++subEnd) {
        gDerivator_Node *node = &(GDERIVATOR_NODE_BY_ID(data[subEnd])->data);
        if (node->mode == gDerivator_Node_mode_opBrack) 
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack) 
            --brackCnt;

        if ((node->mode == gDerivator_Node_mode_sum || node->mode == gDerivator_Node_mode_sub) && brackCnt == 0) {
            #ifdef EXTRA_VERBOSE
                fprintf(stderr, "subStart = %lu\nsubEnd = %lu\n", subStart, subEnd);
            #endif
            if (isSum) {
                status = gDerivator_parser_term(context, subStart, subEnd, sumRoot);
            } else {
                size_t mulRoot = -1;
                GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, sumRoot, &mulRoot, 
                        gDerivator_Node{gDerivator_Node_mode_mul, gDerivator_Node_func_CNT, 0, 0}));

                GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, mulRoot, NULL, 
                        gDerivator_Node{gDerivator_Node_mode_num, gDerivator_Node_func_CNT, -1, 0}));

                status = gDerivator_parser_term(context, subStart, subEnd, mulRoot);
            }
            GDERIVATOR_IS_OK(status);
            isSum = (node->mode == gDerivator_Node_mode_sum);
            GDERIVATOR_POOL_FREE(data[subEnd]);
            ++subEnd;
            subStart = subEnd;
        }
    }
    if (isSum) {
        status = gDerivator_parser_term(context, subStart, subEnd, sumRoot);
    } else {
        size_t mulRoot = -1;
        GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, sumRoot, &mulRoot, 
                gDerivator_Node{gDerivator_Node_mode_mul, gDerivator_Node_func_CNT, 0, 0}));

        GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, mulRoot, NULL, 
                gDerivator_Node{gDerivator_Node_mode_num, gDerivator_Node_func_CNT, -1, 0}));

        status = gDerivator_parser_term(context, subStart, subEnd, mulRoot);
    }
 
    return status;
}

static gDerivator_status gDerivator_parser_term(gDerivator *context,
                                                const size_t start,
                                                const size_t end,
                                                const size_t subRoot)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    GDERIVATOR_ID_CHECK(subRoot);
    assert(start <  context->LexemeIds.len);
    assert(end   <= context->LexemeIds.len);
    assert(start <= end);

    gDerivator_status status = gDerivator_status_OK;

    size_t subStart = -1, subEnd = -1;
    size_t *data = context->LexemeIds.data;
    long brackCnt = 0;

    size_t firstPos = -1;
    for (size_t i = start; i < end; ++i) {
        gDerivator_Node *node = &(GDERIVATOR_NODE_BY_ID(data[i])->data);
        if (node->mode == gDerivator_Node_mode_opBrack) 
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack) 
            --brackCnt;

        if ((node->mode == gDerivator_Node_mode_mul || node->mode == gDerivator_Node_mode_div) && brackCnt == 0) {
            if (firstPos == -1)
                firstPos = i;
            else 
                firstPos = -2;
        }
    }

    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "start = %lu\nend = %lu\nfirstPos = %lu\n", start, end, firstPos);
    #endif
    if (firstPos == -1) {
        return gDerivator_parser_expn(context, start, end, subRoot);
    } else if (firstPos != -2) {
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subRoot, data[firstPos]));

        status = gDerivator_parser_expn(context, start,        firstPos, data[firstPos]);
        GDERIVATOR_IS_OK(status);
        status = gDerivator_parser_expn(context, firstPos + 1, end,      data[firstPos]);
        GDERIVATOR_IS_OK(status);
        return status;
    }

    size_t mulRoot = -1;
    bool isMul = true;
    GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, subRoot, &mulRoot, 
                gDerivator_Node{gDerivator_Node_mode_mul, gDerivator_Node_func_CNT, 0, 0}));
    brackCnt = 0;
    for (subStart = subEnd = start; subEnd < end; ++subEnd) {
        gDerivator_Node *node = &(GDERIVATOR_NODE_BY_ID(data[subEnd])->data);
        if (node->mode == gDerivator_Node_mode_opBrack) 
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack) 
            --brackCnt;

        if ((node->mode == gDerivator_Node_mode_mul || node->mode == gDerivator_Node_mode_div) && brackCnt == 0) {
            #ifdef EXTRA_VERBOSE
                fprintf(stderr, "subStart = %lu\nsubEnd = %lu\n", subStart, subEnd);
            #endif
            if (isMul) {
                status = gDerivator_parser_expn(context, subStart, subEnd, mulRoot);
                GDERIVATOR_IS_OK(status);
            } else {
                size_t expRoot = -1;
                GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, mulRoot, &expRoot, 
                        gDerivator_Node{gDerivator_Node_mode_exp, gDerivator_Node_func_CNT, 0, 0}));

                status = gDerivator_parser_expn(context, subStart, subEnd, expRoot);
                GDERIVATOR_IS_OK(status);

                GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, expRoot, NULL, 
                        gDerivator_Node{gDerivator_Node_mode_num, gDerivator_Node_func_CNT, -1, 0}));
            }
            isMul = (node->mode == gDerivator_Node_mode_mul);
            GDERIVATOR_POOL_FREE(data[subEnd]);
            subStart = subEnd + 1;
        }
    }
   if (isMul) {
        status = gDerivator_parser_expn(context, subStart, subEnd, mulRoot);
        GDERIVATOR_IS_OK(status);
    } else {
        size_t expRoot = -1;
        GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, mulRoot, &expRoot, 
                gDerivator_Node{gDerivator_Node_mode_exp, gDerivator_Node_func_CNT, 0, 0}));

        status = gDerivator_parser_expn(context, subStart, subEnd, expRoot);
        GDERIVATOR_IS_OK(status);

        GDERIVATOR_TREE_CHECK(gTree_addChild(&context->tree, expRoot, NULL, 
                gDerivator_Node{gDerivator_Node_mode_num, gDerivator_Node_func_CNT, -1, 0}));
    } 

    return status;
}

static gDerivator_status gDerivator_parser_expn(gDerivator *context,
                                                const size_t start,
                                                const size_t end,
                                                const size_t root)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    GDERIVATOR_ID_CHECK(root);
    assert(start <  context->LexemeIds.len);
    assert(end   <= context->LexemeIds.len);
    assert(start <= end);

    gDerivator_status status = gDerivator_status_OK;

    size_t subStart = -1, subEnd = -1;
    size_t *data = context->LexemeIds.data;
    long brackCnt = 0;

    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "start = %lu\nend = %lu\n", start, end);
    #endif

    brackCnt = 0;
    for (subStart = subEnd = start; subEnd < end; ++subEnd) {
        gDerivator_Node *node = &(GDERIVATOR_NODE_BY_ID(data[subEnd])->data);
        if (node->mode == gDerivator_Node_mode_opBrack) 
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack) 
            --brackCnt;

        if (node->mode == gDerivator_Node_mode_exp && brackCnt == 0) {
            #ifdef EXTRA_VERBOSE
                fprintf(stderr, "subStart = %lu\nsubEnd = %lu\n", subStart, subEnd);
            #endif
                
            size_t expRoot = data[subEnd];
            status = gDerivator_parser_prior(context, subStart, subEnd, expRoot);
            GDERIVATOR_IS_OK(status);

             
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, root, expRoot));

            ++subEnd;
            subStart = subEnd;
            status = gDerivator_parser_expn(context, subStart, end, expRoot);
            GDERIVATOR_IS_OK(status);
            
            return status;
        }
    }
    status = gDerivator_parser_prior(context, start, end, root);
    GDERIVATOR_IS_OK(status);

    return status;
}

static gDerivator_status gDerivator_parser_prior(gDerivator *context, 
                                                 const size_t start,
                                                 const size_t end, 
                                                 const size_t subRoot)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    GDERIVATOR_ID_CHECK(subRoot);
    assert(start <  context->LexemeIds.len);
    assert(end   <= context->LexemeIds.len);
    assert(start <= end);

    gDerivator_status status = gDerivator_status_OK;

    size_t *data = context->LexemeIds.data;
    if (GDERIVATOR_NODE_BY_ID(data[start])->data.mode == gDerivator_Node_mode_opBrack) {            
        GDERIVATOR_ASSERT_LOG(GDERIVATOR_NODE_BY_ID(data[end - 1])->data.mode == gDerivator_Node_mode_clBrack,
                                    gDerivator_status_ParsingErr_NoBrack);
        status = gDerivator_parser_expr(context, start + 1, end - 1, subRoot);
        GDERIVATOR_POOL_FREE(data[start]);
        GDERIVATOR_POOL_FREE(data[end - 1]);

    } else if (end - start == 1) {
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subRoot, data[start]));

    } else if (GDERIVATOR_NODE_BY_ID(data[start])->data.mode == gDerivator_Node_mode_func) {
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subRoot, data[start]));
        status = gDerivator_parser_prior(context, start + 1, end, data[start]);

    } else {
        GDERIVATOR_ASSERT_LOG(false, gDerivator_status_ParsingErr_UnknownLex);
    }
    return status;
}


static gDerivator_status gDerivator_derivate(gDerivator *context, const size_t rootId)          
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    GDERIVATOR_ID_CHECK(rootId);

    size_t len = context->LexemeIds.len;
    GDERIVATOR_ASSERT_LOG(len != 0 && len < GDERIVATOR_LEX_LIM, gDerivator_status_EmptyLexer);
    GDERIVATOR_ASSERT_LOG(GDERIVATOR_NODE_BY_ID(context->tree.root)->child != -1, gDerivator_status_EmptyTree);
 
    gTree_Node *node = GDERIVATOR_NODE_BY_ID(rootId);
    gTree_Node *child = NULL;
    gDerivator_status status = gDerivator_status_OK;
    size_t childId = node->child;
    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "rootId = %lu; childId = %lu\n", rootId, childId);
    #endif

    if ((node->data.mode == gDerivator_Node_mode_sum) || (node->data.mode == gDerivator_Node_mode_sub)) {
        while (childId != -1) {
            size_t siblingId = GDERIVATOR_NODE_BY_ID(childId)->sibling;
            status = gDerivator_derivate(context, childId);
            GDERIVATOR_IS_OK(status);
            
            childId = siblingId;
        }
    } else if (node->data.mode == gDerivator_Node_mode_mul) {
        size_t sumNodeId = GDERIVATOR_POOL_ALLOC();  
        gTree_Node *sumNode = GDERIVATOR_NODE_BY_ID(sumNodeId);
        sumNode->data.mode = gDerivator_Node_mode_sum;
        
        size_t siblingId = childId;
        while (siblingId != -1) {
            size_t mulNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *mulNode = GDERIVATOR_NODE_BY_ID(mulNodeId);
            mulNode->data.mode = gDerivator_Node_mode_mul;
            
            size_t iterId = childId;
            while (iterId != -1) {
                if (iterId != siblingId) {            
                    size_t clonedId = -1;
                    GDERIVATOR_TREE_CHECK(gTree_cloneSubtree (&context->tree, iterId,   &clonedId));
                    GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, clonedId));
                }
                iterId = GDERIVATOR_NODE_BY_ID(iterId)->sibling;
            }

            size_t clonedId = -1;
            GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, siblingId, &clonedId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, clonedId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, sumNodeId, mulNodeId));
            GDERIVATOR_IS_OK(gDerivator_derivate(context, clonedId));

            siblingId = GDERIVATOR_NODE_BY_ID(siblingId)->sibling;
        }
        GDERIVATOR_TREE_CHECK(gTree_replaceNode(&context->tree, rootId, sumNodeId));
        GDERIVATOR_TREE_CHECK(gTree_killSubtree(&context->tree, rootId));

    } else if (node->data.mode == gDerivator_Node_mode_div) {
        size_t leftChildId = childId;
        gTree_Node *leftChild = GDERIVATOR_NODE_BY_ID(leftChildId);
        size_t rightChildId = leftChild->sibling;
        gTree_Node *rightChild = GDERIVATOR_NODE_BY_ID(rightChildId);

        leftChild->sibling  = -1;
        assert(rightChild->sibling == -1);

        node->child = -1;
        
        size_t subNodeId = GDERIVATOR_POOL_ALLOC();
        gTree_Node *subNode = GDERIVATOR_NODE_BY_ID(subNodeId);
        subNode->data.mode = gDerivator_Node_mode_sub;
        
        size_t mulNodeId = GDERIVATOR_POOL_ALLOC();
        gTree_Node *mulNode = GDERIVATOR_NODE_BY_ID(mulNodeId);
        mulNode->data.mode = gDerivator_Node_mode_mul;

        size_t rightCloneId = -1;
        GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, rightChildId, &rightCloneId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, rightCloneId));
        GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, rightChildId, &rightCloneId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, rightCloneId));


        size_t leftSubMulNodeId = GDERIVATOR_POOL_ALLOC();
        gTree_Node *leftSubMulNode = GDERIVATOR_NODE_BY_ID(leftSubMulNodeId);
        leftSubMulNode->data.mode = gDerivator_Node_mode_mul;

        size_t rightSubMulNodeId = GDERIVATOR_POOL_ALLOC();
        gTree_Node *rightSubMulNode = GDERIVATOR_NODE_BY_ID(rightSubMulNodeId);
        rightSubMulNode->data.mode = gDerivator_Node_mode_mul;

        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subNodeId, leftSubMulNodeId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subNodeId, rightSubMulNodeId));
        
        size_t leftCloneId = -1;
        GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, leftChildId, &leftCloneId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, leftSubMulNodeId, leftCloneId));
        GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, rightChildId, &rightCloneId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, leftSubMulNodeId, rightCloneId));
        GDERIVATOR_IS_OK(gDerivator_derivate(context, rightCloneId));

        GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, leftChildId, &leftCloneId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rightSubMulNodeId, leftCloneId));
        GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, rightChildId, &rightCloneId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rightSubMulNodeId, rightCloneId));
        GDERIVATOR_IS_OK(gDerivator_derivate(context, leftCloneId));

       
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, subNodeId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, mulNodeId));

        GDERIVATOR_TREE_CHECK(gTree_killSubtree(&context->tree, leftChildId));
        GDERIVATOR_TREE_CHECK(gTree_killSubtree(&context->tree, rightChildId));

    } else if (node->data.mode == gDerivator_Node_mode_num) {
        node->data.value = 0;
    } else if (node->data.mode == gDerivator_Node_mode_var) {
        node->data.value = 1;
        node->data.mode = gDerivator_Node_mode_num;
    } else if (node->data.mode == gDerivator_Node_mode_exp) {
        size_t mulNodeId = GDERIVATOR_POOL_ALLOC();  
        gTree_Node *mulNode = GDERIVATOR_NODE_BY_ID(mulNodeId);
        mulNode->data.mode = gDerivator_Node_mode_mul;
            
        size_t clonedId = -1;;
        GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, childId, &clonedId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, clonedId));
        GDERIVATOR_IS_OK(gDerivator_derivate(context, clonedId));

        node  = GDERIVATOR_NODE_BY_ID(rootId);
        child = GDERIVATOR_NODE_BY_ID(childId);
        size_t siblingId = child->sibling;
        gTree_Node *sibling = GDERIVATOR_NODE_BY_ID(siblingId);
 
        if (sibling->data.mode == gDerivator_Node_mode_num) {
            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode = gDerivator_Node_mode_num;
            sibling = GDERIVATOR_NODE_BY_ID(siblingId);
            numNode->data.value = sibling->data.value;
            sibling->data.value -= 1;

            GDERIVATOR_TREE_CHECK(gTree_replaceNode(&context->tree, rootId, mulNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, numNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, rootId));
        } else {
            fprintf(stderr, "ERROR: func Exp derivation has not been implemented yet!\n");
            assert(false);
        }
    } else if (node->data.mode == gDerivator_Node_mode_func) {
        gDerivator_Node_func func = node->data.func;

        size_t mulNodeId = GDERIVATOR_POOL_ALLOC();  
        gTree_Node *mulNode = GDERIVATOR_NODE_BY_ID(mulNodeId);
        mulNode->data.mode = gDerivator_Node_mode_mul;
            
        size_t clonedId = -1;;
        GDERIVATOR_TREE_CHECK(gTree_cloneSubtree(&context->tree, childId, &clonedId));
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, clonedId));
        GDERIVATOR_IS_OK(gDerivator_derivate(context, clonedId));
 
        if (func == gDerivator_Node_func_sin) {  
            size_t cosNodeId = GDERIVATOR_POOL_ALLOC();  
            gTree_Node *cosNode = GDERIVATOR_NODE_BY_ID(cosNodeId);
            cosNode->data.mode = gDerivator_Node_mode_func;
            cosNode->data.func = gDerivator_Node_func_cos;

            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, cosNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, cosNodeId, childId));

        } else if (func == gDerivator_Node_func_cos) {
            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode = gDerivator_Node_mode_num;
            numNode->data.value = -1;

            size_t sinNodeId = GDERIVATOR_POOL_ALLOC();  
            gTree_Node *sinNode = GDERIVATOR_NODE_BY_ID(sinNodeId);
            sinNode->data.mode = gDerivator_Node_mode_func;
            sinNode->data.func = gDerivator_Node_func_sin;

            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, sinNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, numNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, sinNodeId, childId));

        } else if (func == gDerivator_Node_func_ln) {
            size_t divNodeId = GDERIVATOR_POOL_ALLOC();  
            gTree_Node *divNode = GDERIVATOR_NODE_BY_ID(divNodeId);
            divNode->data.mode = gDerivator_Node_mode_div;

            size_t oneNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *oneNode = GDERIVATOR_NODE_BY_ID(oneNodeId);
            oneNode->data.mode = gDerivator_Node_mode_num;
            oneNode->data.value = 1;

            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, divNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, divNodeId, oneNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, divNodeId, childId));          

        } else {
            fprintf(stderr, "ERROR: this func derivation has not been implemented yet!\n");
            assert(false);
        }
        GDERIVATOR_TREE_CHECK(gTree_replaceNode(&context->tree, rootId, mulNodeId));
        GDERIVATOR_POOL_FREE(rootId);

 
    } else {
        fprintf(stderr, "ERROR: this mode derivation has not been implemented yet!\n");
        assert(false);
    }

    return status;
}


static gDerivator_status gDerivator_optimize(gDerivator *context, const size_t rootId)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    GDERIVATOR_ID_CHECK(rootId);

    size_t childId = GDERIVATOR_NODE_BY_ID(rootId)->child;

    while (childId != -1) {
        size_t siblingId = GDERIVATOR_NODE_BY_ID(childId)->sibling;
        gDerivator_optimize(context, childId);
        childId = siblingId;
    }

    gTree_Node *node = GDERIVATOR_NODE_BY_ID(rootId);
    gTree_Node *child = NULL;
    childId = GDERIVATOR_NODE_BY_ID(rootId)->child;

    if (node->data.mode == gDerivator_Node_mode_mul) {
        double mulNum = 1;
        double powVar = 0;
        bool calculable = true;
        while (childId != -1) {
            child = GDERIVATOR_NODE_BY_ID(childId);
            size_t siblingId = child->sibling;
            if (child->data.mode == gDerivator_Node_mode_num) {
                mulNum *= child->data.value;
                GDERIVATOR_TREE_CHECK(gTree_delSubtree(&context->tree, childId));
            } else {
                calculable = false;
                if (child->data.mode == gDerivator_Node_mode_var) {
                    powVar += 1;
                    GDERIVATOR_TREE_CHECK(gTree_delSubtree(&context->tree, childId));
                } else if (child->data.mode == gDerivator_Node_mode_exp) { 
                    size_t subChildId = child->child;
                    gTree_Node *subChild = GDERIVATOR_NODE_BY_ID(subChildId);
                    size_t subSiblingId = subChild->sibling;
                    gTree_Node *subSibling = GDERIVATOR_NODE_BY_ID(subSiblingId);

                    if (subSibling->sibling == -1 && 
                            subChild->data.mode == gDerivator_Node_mode_var && subSibling->data.mode == gDerivator_Node_mode_num) {
                        powVar += subSibling->data.value;
                        GDERIVATOR_TREE_CHECK(gTree_delSubtree(&context->tree, childId));
                    }
                   
                }
            }
            childId = siblingId;
        }

        if (calculable || (fabs(mulNum) < GDERIVATOR_EPS)) {           
            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gDerivator_Node_mode_num;
            numNode->data.value = mulNum;
            GDERIVATOR_TREE_CHECK(gTree_replaceNode(&context->tree, rootId, numNodeId));
            GDERIVATOR_TREE_CHECK(gTree_killSubtree(&context->tree, rootId));

            return gDerivator_status_OK;
        } else if ((fabs(mulNum - 1) > GDERIVATOR_EPS)) {
            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gDerivator_Node_mode_num;
            numNode->data.value = mulNum;
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, numNodeId));
        }

        if (fabs(powVar - 1) < GDERIVATOR_EPS) {
            size_t varNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *varNode = GDERIVATOR_NODE_BY_ID(varNodeId);
            varNode->data.mode  = gDerivator_Node_mode_var;
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, varNodeId));
        } else if (fabs(powVar) > GDERIVATOR_EPS) {
            size_t expNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *expNode = GDERIVATOR_NODE_BY_ID(expNodeId);
            expNode->data.mode  = gDerivator_Node_mode_exp;

            size_t varNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *varNode = GDERIVATOR_NODE_BY_ID(varNodeId);
            varNode->data.mode  = gDerivator_Node_mode_var;

            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gDerivator_Node_mode_num;
            numNode->data.value = powVar;

            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, expNodeId, varNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, expNodeId, numNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rootId,    expNodeId));
        }
    } else if (node->data.mode == gDerivator_Node_mode_sum) {
        double sumNum = 0;
        double sumVar = 0;
        bool calculable = true;
        while (childId != -1) {
            child = GDERIVATOR_NODE_BY_ID(childId);
            size_t siblingId = child->sibling;
            if (child->data.mode == gDerivator_Node_mode_num) {
                sumNum += child->data.value;
                GDERIVATOR_TREE_CHECK(gTree_delSubtree(&context->tree, childId));
            } else {
                calculable = false;
                if (child->data.mode == gDerivator_Node_mode_var) {
                    sumVar += 1;
                    GDERIVATOR_TREE_CHECK(gTree_delSubtree(&context->tree, childId));
                } else if (child->data.mode == gDerivator_Node_mode_mul) {
                    size_t subChildId = child->child;
                    gTree_Node *subChild = GDERIVATOR_NODE_BY_ID(subChildId);
                    size_t subSiblingId = subChild->sibling;
                    gTree_Node *subSibling = GDERIVATOR_NODE_BY_ID(subSiblingId);

                    if (subSibling->sibling == -1 && (
                            (subChild->data.mode == gDerivator_Node_mode_var && subSibling->data.mode == gDerivator_Node_mode_num) ||
                            (subChild->data.mode == gDerivator_Node_mode_num && subSibling->data.mode == gDerivator_Node_mode_var))) {
                        if (subChild->data.mode == gDerivator_Node_mode_num) 
                            sumVar += subChild->data.value;
                        else 
                            sumVar += subSibling->data.value;
                        GDERIVATOR_TREE_CHECK(gTree_delSubtree(&context->tree, childId));
                    }
                }
            }
            childId = siblingId;
        }
        if (calculable) {                                          
            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gDerivator_Node_mode_num;
            numNode->data.value = sumNum;
            GDERIVATOR_TREE_CHECK(gTree_replaceNode(&context->tree, rootId, numNodeId));
            GDERIVATOR_TREE_CHECK(gTree_killSubtree(&context->tree, rootId));

            return gDerivator_status_OK;
        } else if (fabs(sumNum) > GDERIVATOR_EPS) {
            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gDerivator_Node_mode_num;
            numNode->data.value = sumNum;
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, numNodeId));
        }

        if (fabs(sumVar - 1) < GDERIVATOR_EPS) {
            size_t varNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *varNode = GDERIVATOR_NODE_BY_ID(varNodeId);
            varNode->data.mode  = gDerivator_Node_mode_var;
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rootId, varNodeId));
        } else if (fabs(sumVar) > GDERIVATOR_EPS) {
            size_t mulNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *mulNode = GDERIVATOR_NODE_BY_ID(mulNodeId);
            mulNode->data.mode  = gDerivator_Node_mode_mul;

            size_t varNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *varNode = GDERIVATOR_NODE_BY_ID(varNodeId);
            varNode->data.mode  = gDerivator_Node_mode_var;

            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gDerivator_Node_mode_num;
            numNode->data.value = sumVar;

            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, varNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, mulNodeId, numNodeId));
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, rootId,    mulNodeId));
        }
    } else if (node->data.mode == gDerivator_Node_mode_sub) {
        GDERIVATOR_ID_CHECK(childId);
        gTree_Node *child = GDERIVATOR_NODE_BY_ID(childId);
        size_t siblingId = child->sibling;
        GDERIVATOR_ID_CHECK(siblingId);
        gTree_Node *sibling = GDERIVATOR_NODE_BY_ID(siblingId);

        if (child->data.mode == gDerivator_Node_mode_num &&
                sibling->data.mode == gDerivator_Node_mode_num) {
            double res = child->data.value - sibling->data.value;
            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gDerivator_Node_mode_num;
            numNode->data.value = res;
            GDERIVATOR_TREE_CHECK(gTree_replaceNode(&context->tree, rootId, numNodeId));
            GDERIVATOR_TREE_CHECK(gTree_killSubtree(&context->tree, rootId));
        }
    } else if (node->data.mode == gDerivator_Node_mode_div) {
        GDERIVATOR_ID_CHECK(childId);
        gTree_Node *child = GDERIVATOR_NODE_BY_ID(childId);
        size_t siblingId = child->sibling;
        GDERIVATOR_ID_CHECK(siblingId);
        gTree_Node *sibling = GDERIVATOR_NODE_BY_ID(siblingId);

        if (child->data.mode == gDerivator_Node_mode_num &&
                sibling->data.mode == gDerivator_Node_mode_num) {
            double res = child->data.value / sibling->data.value;
            size_t numNodeId = GDERIVATOR_POOL_ALLOC();
            gTree_Node *numNode = GDERIVATOR_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gDerivator_Node_mode_num;
            numNode->data.value = res;
            GDERIVATOR_TREE_CHECK(gTree_replaceNode(&context->tree, rootId, numNodeId));
            GDERIVATOR_TREE_CHECK(gTree_killSubtree(&context->tree, rootId));
        } else if (sibling->data.mode == gDerivator_Node_mode_num && sibling->data.value == 1) {
            child->sibling = -1;
            GDERIVATOR_TREE_CHECK(gTree_replaceNode(&context->tree, rootId, childId));
            GDERIVATOR_POOL_FREE(rootId);
            GDERIVATOR_POOL_FREE(siblingId);
        }
    }
    return gDerivator_status_OK;
}


static gDerivator_status gDerivator_dumpSubtreeLatex(const gDerivator *context, const size_t rootId, FILE *out)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    GDERIVATOR_ID_CHECK(rootId);

    gTree_Node *node = GDERIVATOR_NODE_BY_ID(rootId);
    const char *op = gDerivator_Node_modeViewLatex[node->data.mode];
    size_t childId   = node->child;
    size_t siblingId = -1;
    gTree_Node *child = NULL;

    fprintf(out, "{");
    switch (node->data.mode) {
        case gDerivator_Node_mode_sum:
        case gDerivator_Node_mode_mul: 
            child = GDERIVATOR_NODE_BY_ID(childId);
            while (child->sibling != -1) {
                gDerivator_dumpSubtreeLatex(context, childId, out);
                fprintf(out, "%s", op);
                childId = child->sibling;
                child = GDERIVATOR_NODE_BY_ID(childId);
            }
            gDerivator_dumpSubtreeLatex(context, childId, out);
            break;

        case gDerivator_Node_mode_sub:
            siblingId = GDERIVATOR_NODE_BY_ID(childId)->sibling;

            gDerivator_dumpSubtreeLatex(context, childId, out);
            fprintf(out, "%s", op);
            gDerivator_dumpSubtreeLatex(context, siblingId, out);
            break;

        case gDerivator_Node_mode_div:
            siblingId = GDERIVATOR_NODE_BY_ID(childId)->sibling;
            fprintf(out, "%s{", op);
            gDerivator_dumpSubtreeLatex(context, childId, out);
            fprintf(out, "}{", op);
            gDerivator_dumpSubtreeLatex(context, siblingId, out);
            fprintf(out, "}", op);
            

        case gDerivator_Node_mode_exp:
            fprintf(out, "{");
            siblingId = GDERIVATOR_NODE_BY_ID(childId)->sibling;

            gDerivator_dumpSubtreeLatex(context, childId, out);
            fprintf(out, "}%s{", op);
            gDerivator_dumpSubtreeLatex(context, siblingId, out);
            fprintf(out, "}", op);
            break;

        case gDerivator_Node_mode_num:
            fprintf(out, " %.1lf ", node->data.value);
            break;

        case gDerivator_Node_mode_var:
            fprintf(out, " %s ", op);
            break;
        
        case gDerivator_Node_mode_func:
            fprintf(out, "\\%s{", gDerivator_Node_funcView[node->data.func]);
            gDerivator_dumpSubtreeLatex(context, childId, out);
            fprintf(out, "}");
            break;

        default:
            GDERIVATOR_ASSERT_LOG(false, gDerivator_status_DecompositionErr);
    }
    fprintf(out, "}");

    return gDerivator_status_OK;
}


static gDerivator_status gDerivator_dumpLatex(const gDerivator *context, FILE *out)
{
    GDERIVATOR_CHECK_SELF_PTR(context);

    fprintf(out, "\\documentclass[a4paper]{article}\n\\begin{document}\n\t\\[");

    size_t realRootId = GDERIVATOR_NODE_BY_ID(context->tree.root)->child;
    GDERIVATOR_IS_OK(gDerivator_dumpSubtreeLatex(context, realRootId, out));
    fprintf(out, "\\]\n\\end{document}\n");
    return gDerivator_status_OK;
}
