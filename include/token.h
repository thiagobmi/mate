#ifndef TOKEN_H
#define TOKEN_H

typedef enum token_type
{
    LITERAL,
    IDENTIFIER,
    OPERATOR,
    UNARY_OPERATOR,
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
    ASSIGNMENT_OPERATOR,
    COMMA,
    UNDEFINED
} token_type;

typedef struct token
{
    char *value;
    enum token_type type;
} token;

typedef struct token_vec
{
    token *tokens;
    int max_size;
    int len;
    int cur_parsing;
} token_vec;

token get_current_token(token_vec *v);
token_vec *new_token_vec(int max_size);
token_vec *resize_token_vec(token_vec *v, int new_size);
token_vec *add_token(token_vec *v, char *s, token_type t);
void free_token_vec(token_vec *v);
char *get_token_type_name(token_type t);
void print_token(token t);
void print_tokens(token_vec *v);
void print_expression(token_vec *v);

#endif /* TOKEN_H */
