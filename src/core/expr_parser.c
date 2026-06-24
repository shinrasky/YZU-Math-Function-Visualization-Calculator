#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "core/expr_parser.h"

typedef struct {
    const char *s;
    const char *error;
} Parser;

static void skip_spaces(Parser *p) {
    while (*p->s && isspace((unsigned char)*p->s)) {
        p->s++;
    }
}

static ExprNode *new_node(ExprNodeType type) {
    ExprNode *node = (ExprNode *)calloc(1, sizeof(ExprNode));
    if (!node) {
        return NULL;
    }
    node->type = type;
    return node;
}

static int is_ident_char(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static ExprNode *parse_expression(Parser *p);

static ExprNode *parse_primary(Parser *p) {
    skip_spaces(p);

    if (*p->s == '(') {
        p->s++;
        ExprNode *inside = parse_expression(p);
        skip_spaces(p);
        if (*p->s != ')') {
            p->error = "missing ')'";
            expr_free(inside);
            return NULL;
        }
        p->s++;
        return inside;
    }

    if (isdigit((unsigned char)*p->s) || *p->s == '.') {
        char *end_ptr = NULL;
        double value = strtod(p->s, &end_ptr);
        if (end_ptr == p->s) {
            p->error = "invalid number";
            return NULL;
        }
        p->s = end_ptr;
        ExprNode *n = new_node(NODE_NUMBER);
        if (!n) {
            p->error = "out of memory";
            return NULL;
        }
        n->number = value;
        return n;
    }

    if (is_ident_char(*p->s)) {
        char name[16] = {0};
        int i = 0;
        while (is_ident_char(*p->s) && i < (int)sizeof(name) - 1) {
            name[i++] = (char)tolower((unsigned char)*p->s);
            p->s++;
        }

        if (strcmp(name, "pi") == 0 || strcmp(name, "e") == 0) {
            ExprNode *n = new_node(NODE_NUMBER);
            if (!n) {
                p->error = "out of memory";
                return NULL;
            }
            n->number = (strcmp(name, "pi") == 0) ? 3.14159265358979323846 : 2.71828182845904523536;
            return n;
        }

        skip_spaces(p);
        if (*p->s == '(') {
            p->s++;
            ExprNode *arg = parse_expression(p);
            skip_spaces(p);
            if (*p->s != ')') {
                p->error = "missing ')' after function argument";
                expr_free(arg);
                return NULL;
            }
            p->s++;
            ExprNode *n = new_node(NODE_FUNCTION);
            if (!n) {
                p->error = "out of memory";
                expr_free(arg);
                return NULL;
            }
            strncpy(n->text, name, sizeof(n->text) - 1);
            n->left = arg;
            return n;
        }

        ExprNode *n = new_node(NODE_VARIABLE);
        if (!n) {
            p->error = "out of memory";
            return NULL;
        }
        strncpy(n->text, name, sizeof(n->text) - 1);
        return n;
    }

    p->error = "unexpected token";
    return NULL;
}

static ExprNode *parse_unary(Parser *p) {
    skip_spaces(p);
    if (*p->s == '+' || *p->s == '-') {
        char op = *p->s++;
        ExprNode *rhs = parse_unary(p);
        if (!rhs) {
            return NULL;
        }
        if (op == '+') {
            return rhs;
        }
        ExprNode *n = new_node(NODE_UNARY);
        if (!n) {
            p->error = "out of memory";
            expr_free(rhs);
            return NULL;
        }
        n->text[0] = '-';
        n->left = rhs;
        return n;
    }
    return parse_primary(p);
}

static ExprNode *parse_power(Parser *p) {
    ExprNode *left = parse_unary(p);
    if (!left) {
        return NULL;
    }
    skip_spaces(p);
    if (*p->s == '^') {
        p->s++;
        ExprNode *right = parse_power(p);
        if (!right) {
            expr_free(left);
            return NULL;
        }
        ExprNode *n = new_node(NODE_BINARY);
        if (!n) {
            p->error = "out of memory";
            expr_free(left);
            expr_free(right);
            return NULL;
        }
        n->text[0] = '^';
        n->left = left;
        n->right = right;
        return n;
    }
    return left;
}

static ExprNode *parse_term(Parser *p) {
    ExprNode *left = parse_power(p);
    if (!left) {
        return NULL;
    }

    while (1) {
        skip_spaces(p);
        if (*p->s != '*' && *p->s != '/') {
            break;
        }
        char op = *p->s++;
        ExprNode *right = parse_power(p);
        if (!right) {
            expr_free(left);
            return NULL;
        }
        ExprNode *n = new_node(NODE_BINARY);
        if (!n) {
            p->error = "out of memory";
            expr_free(left);
            expr_free(right);
            return NULL;
        }
        n->text[0] = op;
        n->left = left;
        n->right = right;
        left = n;
    }

    return left;
}

static ExprNode *parse_expression(Parser *p) {
    ExprNode *left = parse_term(p);
    if (!left) {
        return NULL;
    }

    while (1) {
        skip_spaces(p);
        if (*p->s != '+' && *p->s != '-') {
            break;
        }
        char op = *p->s++;
        ExprNode *right = parse_term(p);
        if (!right) {
            expr_free(left);
            return NULL;
        }
        ExprNode *n = new_node(NODE_BINARY);
        if (!n) {
            p->error = "out of memory";
            expr_free(left);
            expr_free(right);
            return NULL;
        }
        n->text[0] = op;
        n->left = left;
        n->right = right;
        left = n;
    }

    return left;
}

ExprNode *expr_parse(const char *expr_str, const char **error_msg) {
    Parser p;
    p.s = expr_str;
    p.error = NULL;

    ExprNode *root = parse_expression(&p);
    if (!root) {
        if (error_msg) {
            *error_msg = p.error ? p.error : "parse error";
        }
        return NULL;
    }

    skip_spaces(&p);
    if (*p.s != '\0') {
        expr_free(root);
        if (error_msg) {
            *error_msg = "unexpected trailing characters";
        }
        return NULL;
    }

    if (error_msg) {
        *error_msg = NULL;
    }
    return root;
}

static double lookup_var(const char *name, const Variable *vars, int var_count, int *found) {
    int i;
    for (i = 0; i < var_count; i++) {
        if (strcmp(name, vars[i].name) == 0) {
            *found = 1;
            return vars[i].value;
        }
    }
    *found = 0;
    return 0.0;
}

static double eval_function(const char *name, double v, bool *ok) {
    if (strcmp(name, "sin") == 0) return sin(v);
    if (strcmp(name, "cos") == 0) return cos(v);
    if (strcmp(name, "tan") == 0) return tan(v);
    if (strcmp(name, "asin") == 0) return asin(v);
    if (strcmp(name, "acos") == 0) return acos(v);
    if (strcmp(name, "atan") == 0) return atan(v);
    if (strcmp(name, "sqrt") == 0) return sqrt(v);
    if (strcmp(name, "exp") == 0) return exp(v);
    if (strcmp(name, "abs") == 0) return fabs(v);
    if (strcmp(name, "ln") == 0) return log(v);
    if (strcmp(name, "log") == 0) return log10(v);
    *ok = false;
    return 0.0;
}

double expr_eval(const ExprNode *node, const Variable *vars, int var_count, bool *ok) {
    if (!node) {
        *ok = false;
        return 0.0;
    }

    switch (node->type) {
        case NODE_NUMBER:
            return node->number;
        case NODE_VARIABLE: {
            int found = 0;
            double v = lookup_var(node->text, vars, var_count, &found);
            if (!found) {
                *ok = false;
            }
            return v;
        }
        case NODE_UNARY: {
            double v = expr_eval(node->left, vars, var_count, ok);
            if (!*ok) return 0.0;
            if (node->text[0] == '-') {
                return -v;
            }
            *ok = false;
            return 0.0;
        }
        case NODE_BINARY: {
            double a = expr_eval(node->left, vars, var_count, ok);
            if (!*ok) return 0.0;
            double b = expr_eval(node->right, vars, var_count, ok);
            if (!*ok) return 0.0;
            switch (node->text[0]) {
                case '+': return a + b;
                case '-': return a - b;
                case '*': return a * b;
                case '/':
                    if (fabs(b) < 1e-12) {
                        *ok = false;
                        return 0.0;
                    }
                    return a / b;
                case '^': return pow(a, b);
                default:
                    *ok = false;
                    return 0.0;
            }
        }
        case NODE_FUNCTION: {
            double v = expr_eval(node->left, vars, var_count, ok);
            if (!*ok) return 0.0;
            return eval_function(node->text, v, ok);
        }
        default:
            *ok = false;
            return 0.0;
    }
}

void expr_free(ExprNode *node) {
    if (!node) {
        return;
    }
    expr_free(node->left);
    expr_free(node->right);
    free(node);
}
