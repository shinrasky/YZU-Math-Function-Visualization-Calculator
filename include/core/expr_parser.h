#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H

#include <stdbool.h>

typedef struct {
    char name[16];
    double value;
} Variable;

typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_UNARY,
    NODE_BINARY,
    NODE_FUNCTION
} ExprNodeType;

typedef struct ExprNode {
    ExprNodeType type;
    double number;
    char text[16];
    struct ExprNode *left;
    struct ExprNode *right;
} ExprNode;

ExprNode *expr_parse(const char *expr_str, const char **error_msg);
double expr_eval(const ExprNode *node, const Variable *vars, int var_count, bool *ok);
void expr_free(ExprNode *node);

#endif
