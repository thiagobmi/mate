#ifndef PARSER_H
#define PARSER_H

#include "dictionary.h"
#include "node.h"
#include "token.h"

enum expression_state
{
    EXPR_START,
    EXPR_LITERAL_OR_IDENTIFIER,
    EXPR_OPERATOR,
    EXPR_PARENTHESIS,
    EXPR_FUNCTION_PARENTHESIS,
    EXPR_FUNCTION_COMMA,
    EXPR_FUNCTION_IDENTIFIER,
    EXPR_FUNCTION_CLOSE_PARENTHESIS,
    EXPR_DONE,
    EXPR_ERROR
};

enum func_state
{
    FUNC_START,
    FUNC_NAME,
    FUNC_PAREN,
    FUNC_ARG,
    FUNC_DONE,
    FUNC_ACCEPT,
    FUNC_ERROR

};

struct function_info
{
    char *name;
    token_vec **args;
    int num_args;
};

struct stored_function_info
{
    dictionary *args;
    int num_args;
    node *ast;
};

struct function_info *get_function_arguments(token_vec *v);

entry *parse_function_declaration(token_vec *v);

typedef struct expression_info
{
    bool is_valid;
    int error_at;
    bool is_assignment;
    bool is_function_declaration;

} expression_info;

expression_info valid_expression(token_vec *v);
bool match(token_vec *v, token_type t);

node *compute_expression(token_vec *v);
node *compute_expression_by_prec(token_vec *v, int min_prec);
node *parse_factor(token_vec *v);

void test_expression(const char *s, int expected);
void run_tests(void);

#endif /* PARSER_H */
