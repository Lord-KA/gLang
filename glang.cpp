#include "glang.h"


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
    if (data.mode >= gLang_Node_mode_add && data.mode <= gLang_Node_mode_assign)
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


gLang_status gLang_ctor(gLang *ctx, FILE *newLogStream)
{
    if (!gPtrValid(ctx)) {
        FILE *out;
        if (!gPtrValid(newLogStream))
            out = stderr;
        else
            out = newLogStream;
        fprintf(out, "ERROR: bad structure ptr provided to derivator ctor!\n");
        return gLang_status_BadStructPtr;
    }

    ctx->varTables = NULL;
    ctx->buffer    = NULL;
    ctx->logStream = stderr;

    if (gPtrValid(newLogStream))
        ctx->logStream = newLogStream;

    gTree_status status = gTree_ctor(&ctx->tree, ctx->logStream);
    GLANG_ASSERT_LOG(status == gTree_status_OK, gLang_status_TreeErr);

    GENERIC(stack_ctor)(&ctx->LexemeIds);

    return gLang_status_OK;
}

gLang_status gLang_dtor(gLang *ctx)
{
    GLANG_CHECK_SELF_PTR(ctx);

    GLANG_TREE_CHECK(gTree_dtor(&ctx->tree));

    GENERIC(stack_dtor)(&ctx->LexemeIds);

    if (gPtrValid(ctx->varTables)) {
        for (size_t i = 0; i < ctx->varTablesLen; ++i) {
            GENERIC(stack_dtor)(&ctx->varTables[i]);
        }
        free(ctx->varTables);
        ctx->varTables = NULL;
    }

    return gLang_status_OK;
}

gLang_status gLang_lexer(gLang *ctx, const char *buffer)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ASSERT_LOG(gPtrValid(buffer), gLang_status_BadPtr);

    ctx->buffer = buffer;
    char *cur = (char*)buffer;
    size_t id = -1;
    gLang_Node *node = NULL;
    GENERIC(stack_clear)(&ctx->LexemeIds);

    while (*cur != '\0') {
        #ifdef EXTRA_VERBOSE
            fprintf(ctx->logStream, "cur = %s\n", cur);
        #endif
        if (isspace(*cur)) {
            ++cur;
            continue;
        }
        id   = GLANG_POOL_ALLOC();
        node = &(GLANG_NODE_BY_ID(id)->data);
        GENERIC(stack_push)(&ctx->LexemeIds, id);
        node->position = cur - buffer;

        bool foundLit = false;
        for (size_t i = gLang_Node_mode_add; i < gLang_Node_mode_var; ++i) {
            /* one-symbol literals case */
            if (*cur == '-') {
                if (ctx->LexemeIds.len <= 1)
                    break;
                gLang_Node *prev = &GLANG_NODE_BY_ID(ctx->LexemeIds.data[ctx->LexemeIds.len - 2])->data;
                if (prev->mode == gLang_Node_mode_keyword ||
                        (prev->mode >= gLang_Node_mode_add && prev->mode < gLang_Node_mode_var &&
                         prev->mode != gLang_Node_mode_clBrack))
                    break;


            }
            if (*gLang_Node_modeView[i] == *cur) {
                node->mode = (gLang_Node_mode)i;
                foundLit = true;
                ++cur;
                break;
            }
        }
        if (foundLit) {
            #ifdef EXTRA_VERBOSE
                fprintf(ctx->logStream, "Found lit!\n");
                fprintf(ctx->logStream, "\tcur = #%s#\n", cur);
            #endif
            continue;
        }

        char *litEnd = cur;
        while (!GLANG_IS_DELIM(litEnd))
            ++litEnd;
        char literal[GLANG_MAX_LIT_LEN] = "";
        if (litEnd - cur >= GLANG_MAX_LIT_LEN) {
            fprintf(ctx->logStream, "ERROR: literal is too long (MAX_LIT_LEN = %lu)\n", GLANG_MAX_LIT_LEN);
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
                fprintf(ctx->logStream, "ERROR: bad function/variable name; it should not start with a digit!\n");
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
                    fprintf(ctx->logStream, "ERROR: bad function/variable name; it should only contain letters, digits and '_'!\n");
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
        fprintf(ctx->logStream, "LexemeIds = {");
        for (size_t i = 0; i < ctx->LexemeIds.len; ++i)
            fprintf(ctx->logStream, "%lu, ", ctx->LexemeIds.data[i]);
        fprintf(ctx->logStream, "}\n");
    #endif

    return gLang_status_OK;
}

gLang_status gLang_parser(gLang *ctx)
{
    GLANG_CHECK_SELF_PTR(ctx);
    size_t len = ctx->LexemeIds.len;
    ctx->lexemeCur = 0;

    GLANG_ASSERT_LOG(len != 0 && len < GLANG_LEX_LIM, gLang_status_EmptyLexer);
    GLANG_IS_OK(gLang_parser_funcDef(ctx, ctx->tree.root));

    return gLang_status_OK;
}

static gLang_status gLang_parser_funcDef(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    size_t lastChildId = -1;
    while (ctx->lexemeCur < ctx->LexemeIds.len) {
        GLANG_IS_OK(gLang_parser_func(ctx, rootId));
        if (lastChildId == -1)
            lastChildId = GLANG_NODE_BY_ID(rootId)->child;
        else
            lastChildId = GLANG_NODE_BY_ID(lastChildId)->sibling;

        GLANG_IS_OK(gLang_parser_blk(ctx, lastChildId));
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_gram(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_status status = gLang_status_OK;
    while (status != gLang_status_NothingToDo) {
        status = gLang_parser_stmnt(ctx, rootId);
        GLANG_ASSERT_LOG(status == gLang_status_OK || status == gLang_status_NothingToDo, status);
    }
    return gLang_status_OK;
}


static gLang_status gLang_parser_stmnt(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    size_t *data = ctx->LexemeIds.data;
    gLang_Node *node = GLANG_CUR_NODE();

    gLang_status status = gLang_parser_if(ctx, rootId);
    if (status == gLang_status_OK)
        return gLang_status_OK;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_while(ctx, rootId);
    if (status == gLang_status_OK)
        return gLang_status_OK;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_retrn(ctx, rootId);
    if (status == gLang_status_OK)
        goto finish;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_print(ctx, rootId);
    if (status == gLang_status_OK)
        goto finish;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_assig(ctx, rootId);
    if (status == gLang_status_OK)
        goto finish;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_blk(ctx, rootId);
    if (status == gLang_status_OK)
        return gLang_status_OK;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    status = gLang_parser_expr(ctx, rootId);
    if (status == gLang_status_OK)
        goto finish;
    else
        GLANG_ASSERT_LOG(status == gLang_status_NothingToDo, status);

    return gLang_status_NothingToDo;

finish:
    node = GLANG_CUR_NODE();
    if (node->mode == gLang_Node_mode_semicolon) {
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        ++ctx->lexemeCur;
    } else {
        fprintf(ctx->logStream, "ERROR: no semicolon at the end of the line!\n");   //TODO add position outp
        GLANG_ASSERT_LOG(false, gLang_status_ParsingErr_NoSemicolon);
    }
    return gLang_status_OK;
}


static gLang_status gLang_parser_assig(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    if (ctx->lexemeCur + 2 >= ctx->LexemeIds.len)
        return gLang_status_NothingToDo;

    size_t varId = GLANG_CUR_NODE_ID();
    gLang_Node *node = GLANG_CUR_NODE();

    if (node->mode != gLang_Node_mode_var)
        return gLang_status_NothingToDo;

    ctx->lexemeCur += 1;
    size_t assignId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    if (node == NULL || node->mode != gLang_Node_mode_assign) {
        ctx->lexemeCur -= 1;
        return gLang_status_NothingToDo;
    }

    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId,   assignId));
    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, assignId, varId));
    ctx->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(ctx, assignId));
    return gLang_status_OK;
}

static gLang_status gLang_parser_if(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    if (ctx->lexemeCur + 3 >= ctx->LexemeIds.len)
        return gLang_status_NothingToDo;

    size_t ifId = GLANG_CUR_NODE_ID();
    gLang_Node *node = GLANG_CUR_NODE();

    if (node->mode != gLang_Node_mode_keyword || node->keyword != gLang_Node_keyword_if)
        return gLang_status_NothingToDo;

    ctx->lexemeCur += 1;
    size_t opBrackId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_opBrack, gLang_status_ParsingErr_NoBrack);

    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, ifId));
    ctx->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(ctx, ifId));

    size_t clBrackId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
    ctx->lexemeCur += 1;

    GLANG_IS_OK(gLang_parser_stmnt(ctx, ifId));

    GLANG_POOL_FREE(opBrackId);
    GLANG_POOL_FREE(clBrackId);
    return gLang_status_OK;
}


static gLang_status gLang_parser_expr(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();

    size_t tmpId = GLANG_POOL_ALLOC();

    gLang_status status = gLang_parser_cmp(ctx, tmpId);
    if (status == gLang_status_NothingToDo) {
        GLANG_POOL_FREE(tmpId);
        return status;
    } else {
        GLANG_IS_OK(status);
    }

    if (ctx->lexemeCur < ctx->LexemeIds.len &&
            GLANG_CUR_NODE()->mode >= gLang_Node_mode_less && GLANG_CUR_NODE()->mode < gLang_Node_mode_assign) {
        size_t cmpId = GLANG_CUR_NODE_ID();
        ctx->lexemeCur += 1;

        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, cmpId));
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, cmpId,  GLANG_NODE_BY_ID(tmpId)->child));
        GLANG_IS_OK(gLang_parser_cmp(ctx, cmpId));
    } else {
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, GLANG_NODE_BY_ID(tmpId)->child));
    }
    GLANG_POOL_FREE(tmpId);
    return gLang_status_OK;
}

static gLang_status gLang_parser_cmp(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();

    size_t addId = GLANG_POOL_ALLOC();
    GLANG_NODE_BY_ID(addId)->data.mode = gLang_Node_mode_add;

    gLang_status status = gLang_parser_term(ctx, addId);
    if (status == gLang_status_NothingToDo) {
        GLANG_POOL_FREE(addId);
        return status;
    } else {
        GLANG_IS_OK(status);
    }

    gLang_Node *node = GLANG_CUR_NODE();
    while (node != NULL && (node->mode == gLang_Node_mode_add || node->mode == gLang_Node_mode_sub)) {
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        ctx->lexemeCur += 1;
        if (node->mode == gLang_Node_mode_add) {
            GLANG_IS_OK(gLang_parser_term(ctx, addId));
        } else {
            size_t mulId = GLANG_POOL_ALLOC();
            size_t oneId = GLANG_POOL_ALLOC();
            gLang_Node *mulNode = &GLANG_NODE_BY_ID(mulId)->data;
            gLang_Node *oneNode = &GLANG_NODE_BY_ID(oneId)->data;
            mulNode->mode = gLang_Node_mode_mul;
            oneNode->mode = gLang_Node_mode_num;
            oneNode->value = -1;

            GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, mulId, oneId));
            GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, addId, mulId));
            GLANG_IS_OK(gLang_parser_term(ctx, mulId));
        }
        node = GLANG_CUR_NODE();
    }
    size_t childId = GLANG_NODE_BY_ID(addId)->child;
    if (GLANG_NODE_BY_ID(childId)->sibling == -1) {
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, childId));
        GLANG_POOL_FREE(addId);
    } else {
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, addId));
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_term(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();

    size_t mulId = GLANG_POOL_ALLOC();
    GLANG_NODE_BY_ID(mulId)->data.mode = gLang_Node_mode_mul;

    gLang_status status = gLang_parser_expn(ctx, mulId);
    if (status == gLang_status_NothingToDo) {
        GLANG_POOL_FREE(mulId);
        return status;
    } else {
        GLANG_IS_OK(status);
    }

    gLang_Node *node = GLANG_CUR_NODE();
    while (node != NULL && (node->mode == gLang_Node_mode_mul || node->mode == gLang_Node_mode_div)) {
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        ctx->lexemeCur += 1;
        if (node->mode == gLang_Node_mode_mul) {
            GLANG_IS_OK(gLang_parser_expn(ctx, mulId));
        } else {
            size_t expId = GLANG_POOL_ALLOC();
            size_t oneId = GLANG_POOL_ALLOC();
            gLang_Node *expNode = &GLANG_NODE_BY_ID(expId)->data;
            gLang_Node *oneNode = &GLANG_NODE_BY_ID(oneId)->data;
            expNode->mode = gLang_Node_mode_exp;
            oneNode->mode = gLang_Node_mode_num;
            oneNode->value = -1;

            GLANG_IS_OK(gLang_parser_expn(ctx, expId));

            GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, expId, oneId));
            GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, mulId, expId));
        }
        node = GLANG_CUR_NODE();
    }
    size_t childId = GLANG_NODE_BY_ID(mulId)->child;
    if (GLANG_NODE_BY_ID(childId)->sibling == -1) {
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, childId));
        GLANG_POOL_FREE(mulId);
    } else {
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, mulId));
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_expn(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();

    size_t expId = GLANG_POOL_ALLOC();
    GLANG_NODE_BY_ID(expId)->data.mode = gLang_Node_mode_exp;

    gLang_status status = gLang_parser_prior(ctx, expId);
    if (status == gLang_status_NothingToDo) {
        GLANG_POOL_FREE(expId);
        return status;
    } else {
        GLANG_IS_OK(status);
    }

    gLang_Node *node = GLANG_CUR_NODE();
    if (node != NULL && node->mode == gLang_Node_mode_exp) {
        ctx->lexemeCur += 1;

        GLANG_IS_OK(gLang_parser_prior(ctx, expId));
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, expId));
    } else {
        size_t childId = GLANG_NODE_BY_ID(expId)->child;
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, childId));
        GLANG_POOL_FREE(expId);
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_prior(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    if (node->mode == gLang_Node_mode_opBrack) {
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        ctx->lexemeCur += 1;
        GLANG_IS_OK(gLang_parser_expr(ctx, rootId));
        node = GLANG_CUR_NODE();
        GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);
        GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
        GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
        ctx->lexemeCur += 1;
    } else if (node->mode == gLang_Node_mode_num || node->mode == gLang_Node_mode_var) {
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, GLANG_CUR_NODE_ID()));
        ctx->lexemeCur += 1;
    } else if (node->mode == gLang_Node_mode_func) {
        GLANG_IS_OK(gLang_parser_func(ctx, rootId));

    } else if (node->mode == gLang_Node_mode_keyword &&
                   node->keyword >= gLang_Node_keyword_sqrt &&
                   node->keyword < gLang_Node_keyword_CNT) {
        size_t funcId = GLANG_CUR_NODE_ID();
        GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, funcId));
        ctx->lexemeCur += 1;
        GLANG_ASSERT_LOG(GLANG_CUR_NODE()->mode == gLang_Node_mode_opBrack, gLang_status_ParsingErr_NoBrack);
        ctx->lexemeCur += 1;
        GLANG_IS_OK(gLang_parser_expr(ctx, funcId));
        GLANG_ASSERT_LOG(GLANG_CUR_NODE()->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
        ctx->lexemeCur += 1;

    } else {
        return gLang_status_NothingToDo;
    }
    return gLang_status_OK;
}

static gLang_status gLang_parser_func(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    size_t funcId = GLANG_CUR_NODE_ID();

    if (node->mode != gLang_Node_mode_func)
        return gLang_status_NothingToDo;

    ++ctx->lexemeCur;
    size_t opBrackId = GLANG_CUR_NODE_ID();
    node = &GLANG_NODE_BY_ID(opBrackId)->data;
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_opBrack, gLang_status_ParsingErr_UnknownLex);
    GLANG_POOL_FREE(GLANG_CUR_NODE_ID());

    size_t argsId = GLANG_POOL_ALLOC();
    GLANG_NODE_BY_ID(argsId)->data.mode = gLang_Node_mode_func_args;
    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, funcId, argsId));

    ++ctx->lexemeCur;
    node = GLANG_CUR_NODE();

    while (node != NULL && node->mode != gLang_Node_mode_clBrack) {
        if (node->mode != gLang_Node_mode_comma) {
            GLANG_IS_OK(gLang_parser_expr(ctx, argsId));
        } else {
            GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
            ++ctx->lexemeCur;
        }

        node = GLANG_CUR_NODE();
    }
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
    GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
    ctx->lexemeCur += 1;

    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, funcId));

    return gLang_status_OK;
}

static gLang_status gLang_parser_blk  (gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    size_t funcId = GLANG_CUR_NODE_ID();

    if (node->mode != gLang_Node_mode_opFigBrack)
        return gLang_status_NothingToDo;

    GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
    ++ctx->lexemeCur;

    GLANG_IS_OK(gLang_parser_gram(ctx, rootId));

    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node != NULL && node->mode == gLang_Node_mode_clFigBrack, gLang_status_ParsingErr_NoBrack);
    GLANG_POOL_FREE(GLANG_CUR_NODE_ID());
    ++ctx->lexemeCur;

    return gLang_status_OK;
}


static gLang_status gLang_parser_retrn(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    size_t retId = GLANG_CUR_NODE_ID();

    if (node->mode != gLang_Node_mode_keyword || node->keyword != gLang_Node_keyword_return)
        return gLang_status_NothingToDo;

    ctx->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(ctx, retId));

    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, retId));

    return gLang_status_OK;
}

static gLang_status gLang_parser_print(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    gLang_Node *node = GLANG_CUR_NODE();

    GLANG_ASSERT_LOG(node != NULL, gLang_status_ParsingErr_EmptyOutp);

    size_t retId = GLANG_CUR_NODE_ID();

    if (node->mode != gLang_Node_mode_keyword || node->keyword != gLang_Node_keyword_print)
        return gLang_status_NothingToDo;

    ctx->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(ctx, retId));

    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, retId));

    return gLang_status_OK;
}

static gLang_status gLang_parser_while(gLang *ctx, size_t rootId)
{
    GLANG_PARSER_CHECK();
    if (ctx->lexemeCur + 3 >= ctx->LexemeIds.len)
        return gLang_status_NothingToDo;

    size_t whileId = GLANG_CUR_NODE_ID();
    gLang_Node *node = GLANG_CUR_NODE();

    if (node->mode != gLang_Node_mode_keyword || node->keyword != gLang_Node_keyword_while)
        return gLang_status_NothingToDo;

    ctx->lexemeCur += 1;
    size_t opBrackId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_opBrack, gLang_status_ParsingErr_NoBrack);

    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, whileId));
    ctx->lexemeCur += 1;
    GLANG_IS_OK(gLang_parser_expr(ctx, whileId));

    size_t clBrackId = GLANG_CUR_NODE_ID();
    node = GLANG_CUR_NODE();
    GLANG_ASSERT_LOG(node->mode == gLang_Node_mode_clBrack, gLang_status_ParsingErr_NoBrack);
    ctx->lexemeCur += 1;

    GLANG_IS_OK(gLang_parser_stmnt(ctx, whileId));

    GLANG_POOL_FREE(opBrackId);
    GLANG_POOL_FREE(clBrackId);
    return gLang_status_OK;
}

gLang_status gLang_optimize(gLang *ctx, const size_t rootId)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(rootId);

    size_t childId = GLANG_NODE_BY_ID(rootId)->child;

    while (childId != -1) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        gLang_optimize(ctx, childId);
        childId = siblingId;
    }

    gTree_Node *node = GLANG_NODE_BY_ID(rootId);
    gTree_Node *child = NULL;
    childId = GLANG_NODE_BY_ID(rootId)->child;

    /* calculating nums in mul */
    if (node->data.mode == gLang_Node_mode_mul) {
        double mulNum = 1;
        bool calculable = true;
        while (childId != -1) {
            child = GLANG_NODE_BY_ID(childId);
            size_t siblingId = child->sibling;
            if (child->data.mode == gLang_Node_mode_num) {
                mulNum *= child->data.value;
                GLANG_TREE_CHECK(gTree_delSubtree(&ctx->tree, childId));
            } else {
                calculable = false;
            }
            childId = siblingId;
        }

        if (calculable || fabs(mulNum) < GLANG_EPS) {
            size_t numNodeId = GLANG_POOL_ALLOC();
            gTree_Node *numNode = GLANG_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gLang_Node_mode_num;
            numNode->data.value = mulNum;
            GLANG_TREE_CHECK(gTree_replaceNode(&ctx->tree, rootId, numNodeId));
            GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, rootId));
            return gLang_status_OK;

        } else if ((fabs(mulNum - 1) > GLANG_EPS)) {
            size_t numNodeId = GLANG_POOL_ALLOC();
            gTree_Node *numNode = GLANG_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gLang_Node_mode_num;
            numNode->data.value = mulNum;
            GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, numNodeId));
        }

        /* optimizing smth * smth ^ -1  =>  smth / smth */
        node = GLANG_NODE_BY_ID(rootId);
        childId = GLANG_NODE_BY_ID(rootId)->child;
        child = GLANG_NODE_BY_ID(childId);
        size_t siblingId = child->sibling;
        if (siblingId != -1) {
            gTree_Node *sibling = GLANG_NODE_BY_ID(siblingId);
            if (sibling->sibling == -1) {
                if (sibling->data.mode == gLang_Node_mode_exp) {
                    size_t subChildId = sibling->child;
                    gTree_Node *subChild = GLANG_NODE_BY_ID(subChildId);
                    size_t subSiblingId = subChild->sibling;
                    if (subSiblingId != -1) {
                        gTree_Node *subSibling = GLANG_NODE_BY_ID(subSiblingId);
                        if (subSibling->sibling == -1) {
                           if (subSibling->data.mode == gLang_Node_mode_num && fabs(subSibling->data.value + 1) < GLANG_EPS) {
                                node->data.mode = gLang_Node_mode_div;
                                subChild->sibling = -1;
                                child->sibling = -1;
                                sibling->child = -1;
                                GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, subSiblingId));
                                GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, siblingId));
                                GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, subChildId));
                            }
                        }
                    }
                }
            }
        }



    /* calculating nums in add */
    } else if (node->data.mode == gLang_Node_mode_add) {
        double addNum = 0;
        bool calculable = true;
        while (childId != -1) {
            child = GLANG_NODE_BY_ID(childId);
            size_t siblingId = child->sibling;
            if (child->data.mode == gLang_Node_mode_num) {
                addNum += child->data.value;
                GLANG_TREE_CHECK(gTree_delSubtree(&ctx->tree, childId));
            } else {
                calculable = false;
            }
            childId = siblingId;
        }
        if (calculable) {
            size_t numNodeId = GLANG_POOL_ALLOC();
            gTree_Node *numNode = GLANG_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gLang_Node_mode_num;
            numNode->data.value = addNum;
            GLANG_TREE_CHECK(gTree_replaceNode(&ctx->tree, rootId, numNodeId));
            GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, rootId));
            return gLang_status_OK;

        } else if (fabs(addNum) > GLANG_EPS) {
            size_t numNodeId = GLANG_POOL_ALLOC();
            gTree_Node *numNode = GLANG_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gLang_Node_mode_num;
            numNode->data.value = addNum;
            GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, numNodeId));
        }

        /* optimizing smth + -1 * smth  =>  smth - smth */
        node = GLANG_NODE_BY_ID(rootId);
        childId = GLANG_NODE_BY_ID(rootId)->child;
        child = GLANG_NODE_BY_ID(childId);
        size_t siblingId = child->sibling;
        if (siblingId != -1) {
            gTree_Node *sibling = GLANG_NODE_BY_ID(siblingId);
            if (sibling->sibling == -1) {
                if (sibling->data.mode == gLang_Node_mode_mul) {
                    size_t subChildId = sibling->child;
                    gTree_Node *subChild = GLANG_NODE_BY_ID(subChildId);
                    size_t subSiblingId = subChild->sibling;
                    if (subSiblingId != -1) {
                        gTree_Node *subSibling = GLANG_NODE_BY_ID(subSiblingId);
                        if (subSibling->sibling == -1) {
                            if (subChild->data.mode == gLang_Node_mode_num && fabs(subChild->data.value + 1) < GLANG_EPS) {
                                node->data.mode = gLang_Node_mode_sub;
                                subChild->sibling = -1;
                                child->sibling = -1;
                                GLANG_TREE_CHECK(gTree_delSubtree(&ctx->tree, siblingId));
                                GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, subSiblingId));
                            } else if (subSibling->data.mode == gLang_Node_mode_num && fabs(subSibling->data.value + 1) < GLANG_EPS) {
                                node->data.mode = gLang_Node_mode_sub;
                                subChild->sibling = -1;
                                child->sibling = -1;
                                sibling->child = -1;
                                GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, subSiblingId));
                                GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, siblingId));
                                GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, subChildId));
                            }
                        }
                    }
                }
            }
        }

    /* optimizing num - num */
    } else if (node->data.mode == gLang_Node_mode_sub) {
        GLANG_ID_CHECK(childId);
        gTree_Node *child = GLANG_NODE_BY_ID(childId);
        size_t siblingId = child->sibling;
        GLANG_ID_CHECK(siblingId);
        gTree_Node *sibling = GLANG_NODE_BY_ID(siblingId);

        if (child->data.mode == gLang_Node_mode_num &&
                sibling->data.mode == gLang_Node_mode_num) {
            double res = child->data.value - sibling->data.value;
            size_t numNodeId = GLANG_POOL_ALLOC();
            gTree_Node *numNode = GLANG_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gLang_Node_mode_num;
            numNode->data.value = res;
            GLANG_TREE_CHECK(gTree_replaceNode(&ctx->tree, rootId, numNodeId));
            GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, rootId));
            return gLang_status_OK;
        }
    /* optimizing num / num  and  smth / 1 */
    } else if (node->data.mode == gLang_Node_mode_div) {
        GLANG_ID_CHECK(childId);
        gTree_Node *child = GLANG_NODE_BY_ID(childId);
        size_t siblingId = child->sibling;
        GLANG_ID_CHECK(siblingId);
        gTree_Node *sibling = GLANG_NODE_BY_ID(siblingId);

        if (child->data.mode == gLang_Node_mode_num &&
                sibling->data.mode == gLang_Node_mode_num) {
            double res = child->data.value / sibling->data.value;
            size_t numNodeId = GLANG_POOL_ALLOC();
            gTree_Node *numNode = GLANG_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gLang_Node_mode_num;
            numNode->data.value = res;
            GLANG_TREE_CHECK(gTree_replaceNode(&ctx->tree, rootId, numNodeId));
            GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, rootId));
            return gLang_status_OK;

        } else if (sibling->data.mode == gLang_Node_mode_num && sibling->data.value == 1) {
            child->sibling = -1;
            GLANG_TREE_CHECK(gTree_replaceNode(&ctx->tree, rootId, childId));
            GLANG_POOL_FREE(rootId);
            GLANG_POOL_FREE(siblingId);
            return gLang_status_OK;
        }
    /* optimizing num ^ num  and  smth ^ 1 */
    } else if (node->data.mode == gLang_Node_mode_exp) {
        GLANG_ID_CHECK(childId);
        gTree_Node *child = GLANG_NODE_BY_ID(childId);
        size_t siblingId = child->sibling;
        GLANG_ID_CHECK(siblingId);
        gTree_Node *sibling = GLANG_NODE_BY_ID(siblingId);

        if (child->data.mode == gLang_Node_mode_num &&
                sibling->data.mode == gLang_Node_mode_num) {
            double res = pow(child->data.value, sibling->data.value);
            size_t numNodeId = GLANG_POOL_ALLOC();
            gTree_Node *numNode = GLANG_NODE_BY_ID(numNodeId);
            numNode->data.mode  = gLang_Node_mode_num;
            numNode->data.value = res;
            GLANG_TREE_CHECK(gTree_replaceNode(&ctx->tree, rootId, numNodeId));
            GLANG_TREE_CHECK(gTree_killSubtree(&ctx->tree, rootId));
            return gLang_status_OK;

        } else if (sibling->data.mode == gLang_Node_mode_num && sibling->data.value == 1) {
            child->sibling = -1;
            GLANG_TREE_CHECK(gTree_replaceNode(&ctx->tree, rootId, childId));
            GLANG_POOL_FREE(rootId);
            GLANG_POOL_FREE(siblingId);
            return gLang_status_OK;
        }
    }

    assert(gObjPool_idValid(&ctx->tree.pool, rootId));

    /* optimizing  smth [+*] (smth [+*] smth)  =>  smth [+*] smth [+*] smth */
    node = GLANG_NODE_BY_ID(rootId);
    if ((node->data.mode == gLang_Node_mode_mul) || (node->data.mode == gLang_Node_mode_add)) {
        const gLang_Node_mode mode = node->data.mode;
        childId = node->child;
        size_t siblingId = -1;
        while (childId != -1) {
            gTree_Node *child = GLANG_NODE_BY_ID(childId);
            siblingId = child->sibling;
            if (child->data.mode == mode) {
                size_t subChildId = child->child;
                while (subChildId != -1) {
                    size_t subSiblingId = GLANG_NODE_BY_ID(subChildId)->sibling;
                    GLANG_TREE_CHECK(gTree_addExistChild(&ctx->tree, rootId, subChildId));
                    subChildId = subSiblingId;
                }
                child = GLANG_NODE_BY_ID(childId);
                child->child = -1;
                GLANG_TREE_CHECK(gTree_delSubtree(&ctx->tree, childId));
            }
            childId = siblingId;
        }
    }
    return gLang_status_OK;
}

static gLang_status gLang_fillVarTable(gLang *ctx, size_t rootId)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(rootId);
    GLANG_ASSERT_LOG(gPtrValid(ctx->varTables), gLang_status_BadPtr);

    gArr *vars = &(ctx->varTables[ctx->varTablesCur]);
    GLANG_ASSERT_LOG(gPtrValid(vars), gLang_status_BadPtr);

    gLang_Node *node = &GLANG_NODE_BY_ID(rootId)->data;
    if (node->mode == gLang_Node_mode_var) {
        size_t i = 0;
        for (i = 0; i < vars->len; ++i) {
            size_t varId = vars->data[i].varId;
            if (!strcmp(node->varName, GLANG_NODE_BY_ID(varId)->data.varName))
                break;
        }
        if (i == vars->len) {
            Var v =                                                                     //TODO there is a need to rework whole varTable stuff, cause we need a different varPool for each func and maybe it should include whole varTable in it?
            gArr_push(vars, rootId);
        }
        node->varId = i;
    }

    size_t childId = GLANG_NODE_BY_ID(rootId)->child;
    while (childId != -1) {
        gLang_fillVarTable(ctx, childId);
        childId = GLANG_NODE_BY_ID(childId)->sibling;
    }
    return gLang_status_OK;
}

static gLang_status gLang_compileExpr(gLang *ctx, size_t rootId)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(rootId);

    gLang_Node *node = &GLANG_NODE_BY_ID(rootId)->data;
    FILE *out = ctx->asmOut;
    size_t childId = GLANG_NODE_BY_ID(rootId)->child;

    if (node->mode == gLang_Node_mode_num) {
        if (node->value < 0)
            fprintf(out, "push 0%g\n", node->value);    //TODO check value type
        else
            fprintf(out, "push %g\n", node->value);

    } else if (node->mode == gLang_Node_mode_var) {
        fprintf(out, "push [(fx - %lu) * 8] ; (%s)\n", node->varId, node->varName);

    } else if (node->mode == gLang_Node_mode_add) {
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        childId = GLANG_NODE_BY_ID(childId)->sibling;
        while (childId != -1) {
            GLANG_IS_OK(gLang_compileExpr(ctx, childId));
            fprintf(out, "add\n");
            childId = GLANG_NODE_BY_ID(childId)->sibling;
        }

    } else if (node->mode == gLang_Node_mode_sub) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId));
        fprintf(out, "sub\n");

    } else if (node->mode == gLang_Node_mode_mul) {
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        childId = GLANG_NODE_BY_ID(childId)->sibling;
        while (childId != -1) {
            GLANG_IS_OK(gLang_compileExpr(ctx, childId));
            fprintf(out, "mul\n");
            childId = GLANG_NODE_BY_ID(childId)->sibling;
        }

    } else if (node->mode == gLang_Node_mode_div) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId));
        fprintf(out, "div\n");

    } else if (node->mode == gLang_Node_mode_exp) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId));
        fprintf(out, "pow\n");

    } else if (node->mode == gLang_Node_mode_func) {
        childId = GLANG_NODE_BY_ID(childId)->child;
        while (childId != -1) {
            GLANG_IS_OK(gLang_compileExpr(ctx, childId));
            childId = GLANG_NODE_BY_ID(childId)->sibling;
        }
        fprintf(out, "call func_%s;\n", node->funcName);

    } else if (node->mode == gLang_Node_mode_less) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId));
        fprintf(out, "cmp\n");
        fprintf(out, "mov ax, 0\n");
        fprintf(out, "cmovl ax, 1\n");
        fprintf(out, "push ax\n");

    } else if (node->mode == gLang_Node_mode_great) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId));
        fprintf(out, "cmp\n");
        fprintf(out, "mov ax, 0\n");
        fprintf(out, "cmovg ax, 1\n");
        fprintf(out, "push ax\n");

    } else if (node->mode == gLang_Node_mode_keyword &&
                   node->keyword >= gLang_Node_keyword_sqrt &&
                   node->keyword < gLang_Node_keyword_CNT) {

        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        fprintf(out, "%s\n", gLang_Node_keywordView[node->keyword]);

    } else {
        assert(!"This should never happen, PANIC!!1!");
    }
    return gLang_status_OK;
}

static gLang_status gLang_compileBlk(gLang *ctx, size_t siblingId)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(siblingId);

    while (siblingId != -1) {
        GLANG_IS_OK(gLang_compileStmnt(ctx, siblingId));
        siblingId = GLANG_NODE_BY_ID(siblingId)->sibling;
    }
    return gLang_status_OK;
}

static gLang_status gLang_compileStmnt(gLang *ctx, size_t rootId)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(rootId);

    gLang_Node *node = &GLANG_NODE_BY_ID(rootId)->data;
    FILE *out = ctx->asmOut;
    size_t childId = GLANG_NODE_BY_ID(rootId)->child;

    if (node->mode == gLang_Node_mode_keyword &&
            node->keyword == gLang_Node_keyword_if) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        fprintf(out, "; IF started\n");
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        size_t ifLabel = ctx->labelCnt;
        fprintf(out, "pop ax\n");
        fprintf(out, "cmp ax, 0\n");
        fprintf(out, "jeq label_%lu\n", ifLabel);
        GLANG_IS_OK(gLang_compileBlk(ctx, siblingId));
        fprintf(out, "label_%lu:\n", ifLabel);
        ++ctx->labelCnt;
    } else if (node->mode == gLang_Node_mode_keyword &&
            node->keyword == gLang_Node_keyword_while) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        fprintf(out, "label_beg_%lu:\n", ctx->labelCnt);
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        fprintf(out, "pop ax\n");
        fprintf(out, "cmp ax, 0\n");
        fprintf(out, "jeq label_end_%lu\n", ctx->labelCnt + 1);
        GLANG_IS_OK(gLang_compileBlk(ctx, siblingId));
        fprintf(out, "jmp label_beg_%lu\n", ctx->labelCnt);
        fprintf(out, "label_end_%lu:\n", ctx->labelCnt + 1);
        ctx->labelCnt += 2;
    } else if (node->mode == gLang_Node_mode_assign) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId));
        gLang_Node *sibling = &GLANG_NODE_BY_ID(childId)->data;
        fprintf(out, "pop [(fx - %lu) * 8] ; (%s)\n", sibling->varId, sibling->varName);

    } else if (node->mode == gLang_Node_mode_keyword &&
            node->keyword == gLang_Node_keyword_return) {
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        size_t offset = ctx->varTables[ctx->varTablesCur].len;
        fprintf(out, "sub fx, %lu\n", offset);
        fprintf(out, "pop bx\n");
        fprintf(out, "pop ax\n");
        fprintf(out, "push bx\n");
        fprintf(out, "push ax\n");
        fprintf(out, "ret\n");

    } else if (node->mode == gLang_Node_mode_keyword &&
            node->keyword == gLang_Node_keyword_print) {
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));
        fprintf(out, "out\n");

    } else {
        GLANG_IS_OK(gLang_compileExpr(ctx, rootId));
    }
    return gLang_status_OK;
}

static gLang_status gLang_getArg(gLang *ctx, size_t siblingId)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(siblingId);

    gTree_Node *node = GLANG_NODE_BY_ID(siblingId);
    if (node->sibling != -1) {
        GLANG_IS_OK(gLang_getArg(ctx, node->sibling));
    }
    fprintf(ctx->asmOut, "pop [(fx - %lu) * 8]; (%s)\n", node->data.varId, node->data.varName);
    return gLang_status_OK;
}

gLang_status gLang_compile(gLang *ctx, FILE *out)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ASSERT_LOG(gPtrValid(out), gLang_status_BadPtr);

    size_t funcRootId = GLANG_NODE_BY_ID(ctx->tree.root)->child;
    if (funcRootId == -1) {
        fprintf(ctx->logStream, "There is nothing to compile, have you ran the parser?\n");
        return gLang_status_NothingToDo;
    }

    size_t cnt = 0;
    while (funcRootId != -1) {
        ++cnt;
        funcRootId = GLANG_NODE_BY_ID(funcRootId)->sibling;
    }

    ctx->asmOut = out;
    ctx->labelCnt = 0;
    ctx->varTablesLen = cnt;
    ctx->varTables = (gArr*)calloc(cnt, sizeof(gArr));                //TODO define MAX_VARS
    GLANG_ASSERT_LOG(ctx->varTables != NULL, gLang_status_AllocErr);

    fprintf(out, "main:\n");
    fprintf(out, "  call func_main\n");
    fprintf(out, "  out\n");
    fprintf(out, "  ret\n");
    funcRootId = GLANG_NODE_BY_ID(ctx->tree.root)->child;
    ctx->varTablesCur = 0;
    while (funcRootId != -1) {
        ctx->varTables[ctx->varTablesCur] = gArr_new(10);
        gLang_fillVarTable(ctx, funcRootId);

        gTree_Node *node = GLANG_NODE_BY_ID(funcRootId);
        size_t len = ctx->varTables[ctx->varTablesCur].len;
        #ifdef EXTRA_VERBOSE
            fprintf(ctx->logStream, "for func %s varTable len is %lu\n", node->data.funcName, len);
        #endif

        fprintf(out, "func_%s:\n", node->data.funcName);
        fprintf(out, "add fx, %lu\n", len);
        size_t argId = node->child;
        node = GLANG_NODE_BY_ID(argId);
        size_t blkId = node->sibling;
        argId = node->child;

        fprintf(out, "pop ex\n");
        if (argId != -1)
            GLANG_IS_OK(gLang_getArg(ctx, argId));
        fprintf(out, "push ex\n");

        GLANG_IS_OK(gLang_compileBlk(ctx, blkId));

        ++ctx->varTablesCur;
        funcRootId = GLANG_NODE_BY_ID(funcRootId)->sibling;
    }

    return gLang_status_OK;
}

#include "undefs.h"
