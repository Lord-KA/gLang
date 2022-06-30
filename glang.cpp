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
    else if (data.mode == gLang_Node_mode_var) {
        if (data.var == NULL)
            fprintf(out, "| {\'%s\' | NULL} ", data.varName);
        else
            fprintf(out, "| {\'%s\' | %s | %zu} ", data.varName, REGISTER_MSG[data.var->reg], data.var->offset);
    }
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
        for (size_t i = 0; i < ctx->varTablesLen; ++i)
            varPool_delete(ctx->varTables[i]);
        free(ctx->varTables);
        ctx->varTables = NULL;
    }

    if (gPtrValid(ctx->commands))
        gArr_delete_c(ctx->commands);

    if (gPtrValid(ctx->bin))
        gArr_delete_b(ctx->bin);

    free(ctx->labelFixup);

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
            fprintf(ctx->logStream, "\tcur = #%s#\n", cur);
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
            gLang_Node *prev = NULL;
            if (*cur == '-') {
                if (ctx->LexemeIds.len < 2) {
                    goto forEnd;
                }
                prev = &GLANG_NODE_BY_ID(ctx->LexemeIds.data[ctx->LexemeIds.len - 2])->data;
                if (prev->mode != gLang_Node_mode_num && prev->mode != gLang_Node_mode_var)
                    goto forEnd;
            }
            if (*gLang_Node_modeView[i] == *cur) {
                node->mode = (gLang_Node_mode)i;
                foundLit = true;
                ++cur;
                goto forEnd;
            }
        }
forEnd:
        if (foundLit) {
            #ifdef EXTRA_VERBOSE
                fprintf(ctx->logStream, "\t\tFound lit (%zu)!\n", ctx->LexemeIds.len);
                fprintf(ctx->logStream, "\t\tcur = #%s#\n", cur);
            #endif
            continue;
        }

        char *litEnd = cur + 1;
        while (!GLANG_IS_DELIM(litEnd))
            ++litEnd;
        char literal[GLANG_MAX_LIT_LEN] = "";
        if (litEnd - cur >= GLANG_MAX_LIT_LEN) {
            fprintf(ctx->logStream, "ERROR: literal is too long (MAX_LIT_LEN = %lu)\n", GLANG_MAX_LIT_LEN);
            GLANG_THROW_ERR(gLang_status_ParsingErr_UnknownLex);
        }
        strncpy(literal, cur, litEnd - cur);
        if (*cur == '-') {
            char *iter = cur + 1;
            while (iter != litEnd && isdigit(*iter))
                ++iter;
            if (iter == litEnd) {
                node->mode = gLang_Node_mode_num;
                node->value = strtol(literal, NULL, 0);
                cur = litEnd;
                continue;
            }
        }

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
                GLANG_THROW_ERR(gLang_status_ParsingErr_UnknownLex);
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
                    GLANG_THROW_ERR(gLang_status_ParsingErr_UnknownLex);
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
        GLANG_THROW_ERR(gLang_status_ParsingErr_NoSemicolon);
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

static gLang_status gLang_fillVarTable_internal_(gLang *ctx, size_t rootId)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(rootId);
    GLANG_ASSERT_LOG(gPtrValid(ctx->varTables), gLang_status_BadPtr);

    varPool *p = ctx->varTables[ctx->varTablesCur];
    gLang_Node *node = &GLANG_NODE_BY_ID(rootId)->data;
    if (node->mode == gLang_Node_mode_var) {
        size_t lim = REG_CNT_ + p->inMemCnt;
        for (size_t i = 1; i <= lim; ++i) {
            if (i == lim) {
                node->var = varPool_alloc(p, rootId);
                GLANG_ASSERT_LOG(node->var != NULL, gLang_status_AllocErr);
            }
            size_t varId = p->inReg[i].nodeId;
            if (p->inReg[i].allocated && !strcmp(node->varName, GLANG_NODE_BY_ID(varId)->data.varName)) {
                node->var = GLANG_NODE_BY_ID(varId)->data.var;
                break;
            }
        }
    }

    size_t childId = GLANG_NODE_BY_ID(rootId)->child;
    while (childId != -1) {
        GLANG_IS_OK(gLang_fillVarTable_internal_(ctx, childId));
        childId = GLANG_NODE_BY_ID(childId)->sibling;
    }
    return gLang_status_OK;
}

static gLang_status gLang_fillVarTable(gLang *ctx, size_t rootId)
{
    fprintf(stderr, "rootId = %zu\n", rootId);
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(rootId);
    GLANG_ASSERT_LOG(gPtrValid(ctx->varTables), gLang_status_BadPtr);

    varPool *p = ctx->varTables[ctx->varTablesCur];
    size_t argId = GLANG_NODE_BY_ID(rootId)->child;
    gTree_Node *node = GLANG_NODE_BY_ID(argId);
    argId = node->child;
    fprintf(stderr, "argId = %zu\n", argId);

    size_t i = 0;
    while (argId != -1 && i < 6) {          // 6 == number of args passed via registers
        gTree_Node *node = GLANG_NODE_BY_ID(argId);
        node->data.var = varPool_alloc(p, argId);
        GLANG_ASSERT_LOG(node->data.var != NULL, gLang_status_AllocErr);
        argId = node->sibling;
        ++i;
    }

    while (argId != -1) {
        gTree_Node *node = GLANG_NODE_BY_ID(argId);
        node->data.var = varPool_allocInMem(p, argId);
        GLANG_ASSERT_LOG(node->data.var != NULL, gLang_status_AllocErr);
        argId = node->sibling;
        ++i;
    }
    fprintf(stderr, "i = %zu\n", i);

    GLANG_IS_OK(gLang_fillVarTable_internal_(ctx, rootId));

    return gLang_status_OK;
}

static gLang_status gLang_compileExpr(gLang *ctx, size_t rootId, Var **res_out)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(rootId);

    varPool *p = ctx->varTables[ctx->varTablesCur];
    gLang_Node *node = &GLANG_NODE_BY_ID(rootId)->data;
    FILE *out = ctx->asmOut;
    size_t childId = GLANG_NODE_BY_ID(rootId)->child;
    Var *tmpRes = NULL;
    Var *res = NULL;

    if (node->mode == gLang_Node_mode_num) {
        *res_out = varPool_alloc(p, -1);
        GLANG_ASSERT_LOG(*res_out != NULL, gLang_status_AllocErr);

        Command c = {};
        c.opcode = MOV;
        c.first = **res_out;
        c.second.reg = REG_NONE_;
        c.second.offset = -1;
        c.second.num = node->value;
        PUSH_COMMAND(c);

    } else if (node->mode == gLang_Node_mode_var) {
        *res_out = node->var;
        GLANG_ASSERT_LOG(*res_out != NULL, gLang_status_AllocErr);

    } else if (node->mode == gLang_Node_mode_add || node->mode == gLang_Node_mode_mul) {
        GLANG_IS_OK(gLang_compileExpr(ctx, childId, res_out));
        if ((*res_out)->temp) {
            res = *res_out;
        } else {
            res = varPool_alloc(p, -1);
            GLANG_ASSERT_LOG(res != NULL, gLang_status_AllocErr);

            Command c = {};
            c.opcode = MOV;
            c.first  = *res;
            c.second = **res_out;
            PUSH_COMMAND(c);
        }
        childId = GLANG_NODE_BY_ID(childId)->sibling;
        while (childId != -1) {
            GLANG_IS_OK(gLang_compileExpr(ctx, childId, &tmpRes));

            Command c = {};
            if (node->mode == gLang_Node_mode_add)
                c.opcode = ADD;
            else
                c.opcode = IMUL;
            c.first  = *res;
            c.second = *tmpRes;
            PUSH_COMMAND(c);
            if (tmpRes->temp)
                varPool_free(p, tmpRes);
            childId = GLANG_NODE_BY_ID(childId)->sibling;
        }
        *res_out = res;

    } else if (node->mode == gLang_Node_mode_sub || node->mode == gLang_Node_mode_exp) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId, res_out));
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId, &tmpRes));

        if ((*res_out)->temp) {
            res = *res_out;
        } else {
            res = varPool_alloc(p, -1);
            GLANG_ASSERT_LOG(res != NULL, gLang_status_AllocErr);

            Command c = {};
            c.opcode = MOV;
            c.first  = *res;
            c.second = **res_out;
            PUSH_COMMAND(c);
        }

        Command c = {};
        if (node->mode == gLang_Node_mode_sub)
            c.opcode = SUB;
        else
            c.opcode = POW;
        c.first  = *res;
        c.second = *tmpRes;
        PUSH_COMMAND(c);
        if (tmpRes->temp)
            varPool_free(p, tmpRes);
        *res_out = res;

    } else if (node->mode == gLang_Node_mode_div) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId, res_out));
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId, &tmpRes));

        if ((*res_out)->reg != RAX) {
            Command c = {};
            c.opcode = MOV;
            c.first.reg = RAX;
            c.second = **res_out;
            PUSH_COMMAND(c);
        }
        Var *tmp = NULL;
        if (p->inReg[RDX].allocated) {
            tmp = varPool_alloc(p, -1);
            GLANG_ASSERT_LOG(tmp != NULL, gLang_status_AllocErr);

            Command c = {};
            c.opcode = MOV;
            c.first = *tmp;
            c.second.reg = RDX;
            PUSH_COMMAND(c);

            c.first.reg = RDX;
            c.second.reg = REG_NONE_;
            c.second.offset = -1;
            c.second.num = 0x00;
            PUSH_COMMAND(c);
        }

        Command c = {};
        c.opcode = IDIV;
        if (tmpRes->reg != RDX)
            c.first = *tmpRes;
        else
            c.first = *tmp;
        PUSH_COMMAND(c);
        if (tmpRes->temp)
            varPool_free(p, tmpRes);
        if ((*res_out)->reg != RAX) {
            Command c = {};
            c.opcode = MOV;
            c.first = **res_out;
            c.second.reg = RAX;
            PUSH_COMMAND(c);
        }
        if (tmp != NULL) {
            Command c = {};
            c.opcode = MOV;
            c.first.reg = RDX;
            c.second = *tmp;
            PUSH_COMMAND(c);

            varPool_free(p, tmp);
        }


    } else if (node->mode == gLang_Node_mode_func) {
        childId = GLANG_NODE_BY_ID(childId)->child;
        size_t i = 1;
        bool inUse[7] = {};
        while (childId != -1 && i <= 6) {                    // 6 == number of args passed via registers
            GLANG_IS_OK(gLang_compileExpr(ctx, childId, res_out));
            if (p->inReg[i].allocated && !(*res_out)->temp) {
                inUse[i] = true;
                Command c = {};
                c.opcode = PUSH;
                c.first = p->inReg[i];
                PUSH_COMMAND(c);
            }
            if ((*res_out)->reg != p->inReg[i].reg) {
                Command c = {};
                c.opcode = MOV;
                c.first = p->inReg[i];
                c.second = **res_out;
                PUSH_COMMAND(c);
                p->inReg[i].allocated = true;
                p->inReg[i].temp = true;
                if ((*res_out)->temp)
                    varPool_free(p, *res_out);
            }

            childId = GLANG_NODE_BY_ID(childId)->sibling;
            ++i;
        }
        size_t passedInMem = 0;
        while (childId != -1) {
            GLANG_IS_OK(gLang_compileExpr(ctx, childId, res_out));

            Command c = {};
            c.opcode = PUSH;
            c.first = **res_out;
            PUSH_COMMAND(c);
            if ((*res_out)->temp)
                varPool_free(p, *res_out);

            childId = GLANG_NODE_BY_ID(childId)->sibling;
            ++passedInMem;
        }

        Command c = {};
        c.opcode = CALL;
        strncpy(c.name, node->funcName, GLANG_MAX_LIT_LEN);
        c.labelId = -1;
        PUSH_COMMAND(c);

        assert(i != 0);
        for (size_t j = i - 1; j > 0; --j) {
            if (inUse[j]) {
                Command c = {};
                c.opcode = POP;
                c.first = p->inReg[j];
                PUSH_COMMAND(c);
            } else {
                p->inReg[j].allocated = false;
            }
        }

        /*
        c.opcode = ADD;                                         // TODO recheck the convention; sub rsp with the same num at the end of each func
        c.first.reg = RSP;
        c.second.reg = REG_NONE_;
        c.second.offset = -1;
        c.second.num = -1;
        */

        if ((*res_out)->temp)
            varPool_free(p, *res_out);
        *res_out = p->inReg + RAX;

    } else if (node->mode == gLang_Node_mode_less || node->mode == gLang_Node_mode_great) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId, res_out));
        GLANG_IS_OK(gLang_compileExpr(ctx, siblingId, &tmpRes));

        Command c = {};
        c.opcode = CMP;
        c.first = **res_out;
        c.second = *tmpRes;
        PUSH_COMMAND(c);

        if ((*res_out)->temp) {
            res = *res_out;
            if (tmpRes->temp)
                varPool_free(p, tmpRes);
        } else if (tmpRes->temp) {
            res = tmpRes;
        } else {
            res = varPool_alloc(p, -1);
            GLANG_ASSERT_LOG(res != NULL, gLang_status_AllocErr);
        }

        c.opcode = MOV;
        c.first = *res;
        c.second.reg = REG_NONE_;
        c.second.offset = -1;
        c.second.num = 0;
        PUSH_COMMAND(c);

        if (node->mode == gLang_Node_mode_less)
            c.opcode = CMOVL;
        else
            c.opcode = CMOVG;
        c.first = *res;
        c.second.reg = REG_NONE_;
        c.second.offset = -1;
        c.second.num = 1;
        PUSH_COMMAND(c);
        *res_out = res;

    } else if (node->mode == gLang_Node_mode_keyword &&
                   node->keyword >= gLang_Node_keyword_sqrt &&
                   node->keyword < gLang_Node_keyword_CNT) {
        assert(!"This feature is considered too sweet and left till better days");      //TODO
        GLANG_IS_OK(gLang_compileExpr(ctx, childId));

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
    varPool *p = ctx->varTables[ctx->varTablesCur];

    if (node->mode == gLang_Node_mode_keyword &&
            node->keyword == gLang_Node_keyword_if) {
        size_t siblingId = GLANG_NODE_BY_ID(childId)->sibling;
        Var *res = NULL;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId, &res));
        Command c = {};
        c.opcode = TEST;
        c.first  = *res;
        c.second = *res;
        PUSH_COMMAND(c);
        if (res->temp)
            varPool_free(p, res);

        c.opcode = JE;
        c.labelId = ctx->labelCnt;
        PUSH_COMMAND(c);

        GLANG_IS_OK(gLang_compileBlk(ctx, siblingId));

        c.opcode = LABLE;
        c.labelId = ctx->labelCnt;
        PUSH_COMMAND(c);
        ++ctx->labelCnt;

    } else if (node->mode == gLang_Node_mode_keyword &&
            node->keyword == gLang_Node_keyword_while) {
        size_t blkId = GLANG_NODE_BY_ID(childId)->sibling;

        Command c = {};
        c.opcode = LABLE;
        c.labelId = ctx->labelCnt;
        PUSH_COMMAND(c);
        Var *res = NULL;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId, &res));

        c.opcode = TEST;
        c.first  = *res;
        c.second = *res;
        PUSH_COMMAND(c);

        c.opcode = JE;
        c.labelId = ctx->labelCnt + 1;
        PUSH_COMMAND(c);

        GLANG_IS_OK(gLang_compileBlk(ctx, blkId));

        c.opcode = JMP;
        c.labelId = ctx->labelCnt;
        PUSH_COMMAND(c);

        c.opcode = LABLE;
        c.labelId = ctx->labelCnt + 1;
        PUSH_COMMAND(c);
        ctx->labelCnt += 2;

    } else if (node->mode == gLang_Node_mode_assign) {
        size_t exprId = GLANG_NODE_BY_ID(childId)->sibling;
        Var *res = NULL;
        GLANG_IS_OK(gLang_compileExpr(ctx, exprId, &res));
        gLang_Node *node = &GLANG_NODE_BY_ID(childId)->data;

        Command c = {};
        c.opcode = MOV;
        c.first  = *node->var;
        c.second = *res;
        PUSH_COMMAND(c);

    } else if (node->mode == gLang_Node_mode_keyword &&
            node->keyword == gLang_Node_keyword_return) {
        Var *res = NULL;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId, &res));
        if (res->reg != RAX) {
            Command c = {};
            c.opcode = MOV;
            c.first.reg = RAX;
            c.second = *res;
            PUSH_COMMAND(c);
            if (res->temp)
                varPool_free(p, res);
        }

        size_t offset = ctx->varTables[ctx->varTablesCur]->inMemCnt;
        Command c = {};
        c.opcode = SUB;
        c.first.reg = RSP;
        c.second.reg = REG_NONE_;
        c.second.offset = -1;
        c.second.num = -1;
        PUSH_COMMAND(c);

        c.opcode = MOV;
        c.first.reg = RSP;
        c.second.reg = RBP;
        PUSH_COMMAND(c);

        c.opcode = POP;
        c.first.reg = RBP;
        PUSH_COMMAND(c);

        c.opcode = RET;
        PUSH_COMMAND(c);

    } else if (node->mode == gLang_Node_mode_keyword &&
            node->keyword == gLang_Node_keyword_print) {
        Var *res = NULL;
        GLANG_IS_OK(gLang_compileExpr(ctx, childId, &res));
        Command c = {};
        c.opcode = OUT;
        c.first = *res;
        PUSH_COMMAND(c);
        if (res->temp)
            varPool_free(p, res);

    } else {
        Var *res = NULL;
        GLANG_IS_OK(gLang_compileExpr(ctx, rootId, &res));
    }
    return gLang_status_OK;
}

static gLang_status gLang_getArgs(gLang *ctx, size_t siblingId, size_t num, size_t *cnt)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ID_CHECK(siblingId);
    ++num;
    ++(*cnt);

    gTree_Node *node = GLANG_NODE_BY_ID(siblingId);
    if (node->sibling != -1)
        GLANG_IS_OK(gLang_getArgs(ctx, node->sibling, num, cnt));

    if (num >= 6) {
        Command c = {};
        c.opcode = POP;
        c.first = *node->data.var;
        PUSH_COMMAND(c);
    }
    return gLang_status_OK;
}

gLang_status gLang_compile(gLang *ctx)
{
    GLANG_CHECK_SELF_PTR(ctx);

    if (ctx->funcFixup != NULL) {
        fprintf(ctx->logStream, "WARNING: function fixup table isn't empty!\n");
        gHT_delete(ctx->funcFixup);
    }
    ctx->funcFixup = gHT_new();
    GLANG_ASSERT_LOG(ctx->funcFixup != NULL, gLang_status_AllocErr);

    size_t funcRootId = GLANG_NODE_BY_ID(ctx->tree.root)->child;
    if (funcRootId == -1) {
        fprintf(ctx->logStream, "There is nothing to compile, have you run the parser?\n");
        return gLang_status_NothingToDo;
    }

    size_t cnt = 0;
    while (funcRootId != -1) {
        ++cnt;
        funcRootId = GLANG_NODE_BY_ID(funcRootId)->sibling;
    }

    ctx->labelCnt = 1;
    ctx->varTablesLen = cnt;
    ctx->varTables = (varPool**)calloc(cnt, sizeof(varPool*));
    GLANG_ASSERT_LOG(ctx->varTables != NULL, gLang_status_AllocErr);

    ctx->commands = gArr_new_c(100);
    GLANG_ASSERT_LOG(ctx->commands != NULL, gLang_status_AllocErr);

    Command c = {};
    c.opcode = CALL;
    strcpy(c.name, "main");
    c.labelId = -1;
    PUSH_COMMAND(c);

    c.opcode = EXIT;
    PUSH_COMMAND(c);
    funcRootId = GLANG_NODE_BY_ID(ctx->tree.root)->child;
    ctx->varTablesCur = 0;
    while (funcRootId != -1) {
        ctx->varTables[ctx->varTablesCur] = varPool_new();
        GLANG_ASSERT_LOG(ctx->varTables[ctx->varTablesCur] != NULL, gLang_status_AllocErr);
        gLang_fillVarTable(ctx, funcRootId);

        gTree_Node *node = GLANG_NODE_BY_ID(funcRootId);
        size_t offset = ctx->varTables[ctx->varTablesCur]->inMemCnt;
        #ifdef EXTRA_VERBOSE
            fprintf(ctx->logStream, "for func %s varTable len is %lu | offset is %zu\n", node->data.funcName, ctx->varTables[ctx->varTablesCur]->overall, offset);
            varPool_dump(ctx->varTables[ctx->varTablesCur], ctx->logStream);
        #endif

        c.opcode = LABLE;
        c.labelId = ctx->labelCnt;
        strncpy(c.name, node->data.funcName, GLANG_MAX_LIT_LEN);
        ++ctx->labelCnt;
        PUSH_COMMAND(c);

        gHT_insert(ctx->funcFixup, c.name, c.labelId, false);

        size_t argId = node->child;
        node = GLANG_NODE_BY_ID(argId);
        argId = node->child;
        size_t blkId = node->sibling;
        if (argId != -1) {
            c.opcode = POP;
            c.first.reg = R10;
            PUSH_COMMAND(c);

            size_t cnt = 0;
            GLANG_IS_OK(gLang_getArgs(ctx, argId, 0, &cnt));
            if (cnt < 7) {
                assert(ctx->commands->len > 0);
                --ctx->commands->len;
            } else {
                c.opcode = PUSH;
                c.first.reg = R10;
                PUSH_COMMAND(c);
            }
        }

        c.opcode = PUSH;
        c.first.reg = RBP;
        PUSH_COMMAND(c);                    //TODO push all registers that are to be preserved

        c.opcode = MOV;
        c.first.reg = RBP;
        c.second.reg = RSP;
        PUSH_COMMAND(c);

        c.opcode = ADD;
        c.first.reg = RSP;
        c.second.reg = REG_NONE_;
        c.second.offset = -1;
        c.second.num = -1;
        PUSH_COMMAND(c);

        GLANG_IS_OK(gLang_compileBlk(ctx, blkId));

        ++ctx->varTablesCur;
        funcRootId = GLANG_NODE_BY_ID(funcRootId)->sibling;
    }

    return gLang_status_OK;
}

static gLang_status gLang_translate_movNum(gLang *ctx, REGISTER_ reg, long num)
{
    if (REG_TYPE[reg] == BASIC)
        PUSH_BYTE(0x48);
    else
        PUSH_BYTE(0x49);
    PUSH_BYTE(0xb0 | 0b1000 | REG_CODE[reg]);

    long n = num;
    bool isNegative = num < 0;
    char *c = (char*)&num;
    size_t i = 0;
    for (i = 0; i < 8 && n != (isNegative ? -1 : 0); ++i, ++c) {
        PUSH_BYTE(*c);
        n >>= 8;
    }
    for (size_t j = 0; j < 8 - i; ++j) {
        if (isNegative)
            PUSH_BYTE(0xff);
        else
            PUSH_BYTE(0x00);
    }
    return gLang_status_OK;
}

gLang_status gLang_translate(gLang *ctx, bool fixupRun)
{
    GLANG_CHECK_SELF_PTR(ctx);

    if (ctx->commands == NULL || ctx->commands->len == 0) {
        fprintf(ctx->logStream, "There is nothing to translate, have you run the compiler?\n");
        return gLang_status_NothingToDo;
    }
    if (ctx->funcFixup == NULL) {
        fprintf(ctx->logStream, "ERROR: no function fixup table found! Have you run the compiler?\n");
        return gLang_status_NothingToDo;
    }

    if (ctx->bin != NULL) {
        fprintf(ctx->logStream, "WARNING: bin buffer is not empty!\n");
        ctx->bin = gArr_delete_b(ctx->bin);
    }
    ctx->bin = gArr_new_b(1000);
    GLANG_ASSERT_LOG(ctx->bin != NULL, gLang_status_AllocErr);

    Command *iter = ctx->commands->data;
    size_t offset = 0;
    if (fixupRun) {
        #ifdef EXTRA_VERBOSE
            gHT_dump(ctx->funcFixup, ctx->logStream);
        #endif

        if (ctx->labelFixup != NULL) {
            fprintf(ctx->logStream, "WARNING: label fixup table isn't empty!\n");
            free(ctx->labelFixup);
        }
        ctx->labelFixup = (size_t*)calloc(sizeof(size_t), GLANG_MAX_LABEL_CNT);
        GLANG_ASSERT_LOG(ctx->labelFixup != NULL, gLang_status_AllocErr);
    }
    if (ctx->labelFixup == NULL) {
        fprintf(ctx->logStream, "ERROR: label fixup table is empty!\n");
        GLANG_THROW_ERR(gLang_status_AllocErr);
    }

    size_t funcCnt = -1;
    for (size_t i = 0; i < ctx->commands->len; ++i, ++iter) {
        REGISTER_ f = iter->first.reg;
        REGISTER_ s = iter->second.reg;
        long jlen = -1;
        switch (iter->opcode) {
        case PUSH:
            assert(f != REG_NONE_ && f < REG_CNT_);
            if (REG_TYPE[f] == BASIC) {
                PUSH_BYTE(0x50 | REG_CODE[f]);
            } else if (REG_TYPE[f] == EXTENDED) {
                PUSH_BYTE(0x41);
                PUSH_BYTE(0x50 | REG_CODE[f]);
            }
            break;

        case POP:
            assert(f != REG_NONE_ && f < REG_CNT_);
            if (REG_TYPE[f] == BASIC) {
                PUSH_BYTE(0x50 | (REG_CODE[f] | 0b1000));
            } else if (REG_TYPE[f] == EXTENDED) {
                PUSH_BYTE(0x41);
                PUSH_BYTE(0x50 | (REG_CODE[f] | 0b1000));
            }
            break;

        case MOV:
            if (s == REG_NONE_) {
                assert(f != REG_NONE_ && f < REG_CNT_);
                GLANG_IS_OK(gLang_translate_movNum(ctx, f, iter->second.num));
                break;
            }
        case ADD:
        case SUB:
            if (iter->opcode != MOV && s == REG_NONE_ && iter->second.offset == -1) {
                if (f == RSP && iter->second.num == -1) {
                    assert(funcCnt != -1);
                    iter->second.num = ctx->varTables[funcCnt]->inMemCnt * 8;
                }
                if (iter->second.num == 0)
                    break;
            }
        case CMP:
        case TEST:
            assert(f != REG_NONE_ && f < REG_CNT_);
            if (s == REG_NONE_) {
                assert(iter->second.offset == -1);
                GLANG_IS_OK(gLang_translate_movNum(ctx, R10, iter->second.num));
                s = R10;
            }
            assert(s != REG_NONE_ && s < REG_CNT_);
            if (     REG_TYPE[f] == BASIC    && REG_TYPE[s] == BASIC)
                PUSH_BYTE(0x48);
            else if (REG_TYPE[f] == EXTENDED && REG_TYPE[s] == BASIC)
                PUSH_BYTE(0x49);
            else if (REG_TYPE[f] == BASIC    && REG_TYPE[s] == EXTENDED)
                PUSH_BYTE(0x4c);
            else if (REG_TYPE[f] == EXTENDED && REG_TYPE[s] == EXTENDED)
                PUSH_BYTE(0x4d);
            else
                assert(false);

            if (iter->opcode == ADD) {
                PUSH_BYTE(0x01);
            } else if (iter->opcode == SUB) {
                PUSH_BYTE(0x29);
            } else if (iter->opcode == CMP) {
                PUSH_BYTE(0x39);
            } else if (iter->opcode == MOV) {
                PUSH_BYTE(0x89);
            } else if (iter->opcode == TEST) {
                PUSH_BYTE(0x85);
            } else {
                assert(false);
            }
            PUSH_BYTE(0b11000000 | REG_CODE[s] << 3 | REG_CODE[f]);
            break;

        case CALL:
            if (iter->labelId == -1 && iter->name[0] != '\0') {
                iter->labelId = gHT_find(ctx->funcFixup, iter->name);
            }
        case JMP:
            if (f == REG_NONE_ && iter->labelId != -1) {
                long offset = GLANG_START_OFFSET + ctx->labelFixup[iter->labelId];
                GLANG_IS_OK(gLang_translate_movNum(ctx, R10, offset));
                f = R10;
            } else if (f == REG_NONE_ && iter->first.offset == -1) {
                GLANG_IS_OK(gLang_translate_movNum(ctx, R10, iter->first.num));
                f = R10;
            }
            if (REG_TYPE[f] == EXTENDED)
                PUSH_BYTE(0x41);
            PUSH_BYTE(0xff);
            if (iter->opcode == CALL)
                PUSH_BYTE(0xd0 | REG_CODE[f]);
            else
                PUSH_BYTE(0xe0 | REG_CODE[f]);
            break;

        case IMUL:
            if (s == REG_NONE_ && iter->second.offset == -1 && iter->second.num == 1)
                break;
        case CMOVL:
        case CMOVG:
            assert(f != REG_NONE_ && f < REG_CNT_);
            if (s == REG_NONE_) {
                assert(iter->second.offset == -1);
                GLANG_IS_OK(gLang_translate_movNum(ctx, R10, iter->second.num));
                s = R10;
            }
            assert(s != REG_NONE_ && s < REG_CNT_);
            if (     REG_TYPE[f] == BASIC    && REG_TYPE[s] == BASIC)
                PUSH_BYTE(0x48);
            else if (REG_TYPE[f] == EXTENDED && REG_TYPE[s] == BASIC)
                PUSH_BYTE(0x4c);
            else if (REG_TYPE[f] == BASIC    && REG_TYPE[s] == EXTENDED)
                PUSH_BYTE(0x49);
            else if (REG_TYPE[f] == EXTENDED && REG_TYPE[s] == EXTENDED)
                PUSH_BYTE(0x4d);
            else
                assert(false);

            PUSH_BYTE(0x0f);

            if (iter->opcode == IMUL) {
                PUSH_BYTE(0xaf);
            } else if (iter->opcode == CMOVL) {
                PUSH_BYTE(0x4c);
            } else if (iter->opcode == CMOVG) {
                PUSH_BYTE(0x4f);
            } else {
                assert(false);
            }
            PUSH_BYTE(0b11000000 | REG_CODE[f] << 3 | REG_CODE[s]);
            break;

        case RET:
            PUSH_BYTE(0xc3);
            break;

        case JE:
            assert(iter->labelId != -1);
            assert(ctx->labelFixup[iter->labelId] != -1);
            assert(fixupRun || ctx->labelFixup[iter->labelId] >= ctx->bin->len);
            jlen = ctx->labelFixup[iter->labelId] - ctx->bin->len;
            fprintf(stderr, "jlen = %ld | pos = %zu | id = %zu\n", jlen, ctx->bin->len, iter->labelId);
            assert(fixupRun || (0 <= jlen < 250l && "Not implemented yet!"));

            PUSH_BYTE(0x74);
            PUSH_BYTE(jlen - 2);
            break;

        case IDIV:
            assert(f != REG_NONE_ && f < REG_CNT_);
            if (s == REG_NONE_ && iter->second.offset == -1 && iter->second.num == 1)
                break;
            if (REG_TYPE[f] == EXTENDED)
                PUSH_BYTE(0x49);
            else if (REG_TYPE[f] == BASIC)
                PUSH_BYTE(0x48);
            else
                assert(!"Not implemented yet!");
            PUSH_BYTE(0xf7);
            PUSH_BYTE(0xf0 | 0b1000 | REG_CODE[f]);
            break;

        case LABLE:
            if (fixupRun) {
                if (iter->name[0] != '\0')
                    ++funcCnt;
                if (iter->labelId == -1 && iter->name[0] != '\0') {
                    iter->labelId = gHT_find(ctx->funcFixup, iter->name);
                }
                assert(iter->labelId != -1);
                ctx->labelFixup[iter->labelId] = ctx->bin->len;
            }
            break;
        }
    }
    return gLang_status_OK;
}

gLang_status gLang_dumpBytes(gLang *ctx, FILE *out)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ASSERT_LOG(gPtrValid(out), gLang_status_BadPtr);
    GLANG_ASSERT_LOG(gPtrValid(ctx->bin), gLang_status_NothingToDo);

    GLANG_ASSERT_LOG(fwrite(ctx->bin->data, sizeof(uint8_t), ctx->bin->len, out) == ctx->bin->len, gLang_status_FileErr);

    return gLang_status_OK;
}

gLang_status gLang_writeBin(gLang *ctx, FILE *out)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ASSERT_LOG(gPtrValid(out), gLang_status_BadPtr);
    GLANG_ASSERT_LOG(gPtrValid(ctx->bin), gLang_status_NothingToDo);

    Elf64_Ehdr ehdr = elf_hdr_setup();
    Elf64_Phdr phdr = progr_hdr_setup(ctx->bin->len);
    GLANG_ASSERT_LOG(fwrite(&ehdr, sizeof(ehdr), 1, out) == 1, gLang_status_FileErr);
    GLANG_ASSERT_LOG(fwrite(&phdr, sizeof(phdr), 1, out) == 1, gLang_status_FileErr);
    const size_t offset_len = CODE_OFFSET_ - sizeof(ehdr) - sizeof(phdr);
    uint8_t offset[offset_len] = {};
    GLANG_ASSERT_LOG(fwrite(offset,  sizeof(uint8_t), offset_len,  out) == offset_len, gLang_status_FileErr);
    GLANG_ASSERT_LOG(fwrite(ctx->bin->data, sizeof(uint8_t), ctx->bin->len, out) == ctx->bin->len, gLang_status_FileErr);

    return gLang_status_OK;
}


gLang_status gLang_commandsDump(gLang *ctx, FILE *out)
{
    GLANG_CHECK_SELF_PTR(ctx);
    GLANG_ASSERT_LOG(gPtrValid(out), gLang_status_BadPtr);

    if (ctx->commands == NULL || ctx->commands->len == 0 || ctx->commands->len == -1) {
        fprintf(ctx->logStream, "There are no commands, have you run the compiler?\n");
        return gLang_status_NothingToDo;
    }

    for (size_t i = 0; i < ctx->commands->len; ++i) {
        Command c = ctx->commands->data[i];
        fprintf(out, "%s", OPCODE_MSG[c.opcode]);
        if (c.opcode == LABLE || c.opcode == CALL) {
            if (*c.name != '\0')
                fprintf(out, " | %s", c.name);
            fprintf(out, " | %zu", c.labelId);
            goto end;
        }
        if (c.opcode == JMP | c.opcode == JE) {
            fprintf(out, " | %zu", c.labelId);
            goto end;
        }
        if (OPCODE_ARGS[c.opcode] > 0) {
            Var v = c.first;

            if (v.reg != REG_NONE_) {
                fprintf(out, " %s", REGISTER_MSG[v.reg]);
            } else if (v.offset != -1) {
                fprintf(out, " [RBP - %zu]", v.offset);
            } else {
                fprintf(out, " %ld", v.num);
            }
        }
        if (OPCODE_ARGS[c.opcode] > 1) {
            fprintf(out, ",");
            Var v = c.second;

            if (v.reg != REG_NONE_) {
                fprintf(out, " %s", REGISTER_MSG[v.reg]);
            } else if (v.offset != -1) {
                fprintf(out, " [RBP - %zu]", v.offset);
            } else {
                fprintf(out, " %ld", v.num);
            }
        }
    end:
        fprintf(out, "\t| (%zu)\n", c.line);
    }
    return gLang_status_OK;
}

#include "undefs.h"
