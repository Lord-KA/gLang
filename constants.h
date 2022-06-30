#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <stdlib.h>

static const size_t MAX_LINE_LEN = 1000;
static const size_t GLANG_MAX_LIT_LEN = 100;
static const size_t GLANG_LEX_LIM = 1000;
static const size_t GLANG_MAX_LABEL_CNT = 10000;
static const size_t GLANG_MAX_BIN_LEN   = 1000000;

static const double GLANG_EPS = 1e-3;

static const char GLANG_DELIMS_LIST[] = " +-*/^(){};,<>=";

#endif
