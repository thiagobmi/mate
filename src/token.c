#include "../include/token.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_token_type_name(token_type t)
{
    switch (t)
    {
    case LITERAL:
        return "LITERAL";
    case IDENTIFIER:
        return "IDENTIFIER";
    case OPERATOR:
        return "OPERATOR";
    case ASSIGNMENT_OPERATOR:
        return "ASSIGNMENT";
    case UNARY_OPERATOR:
        return "UNARY_OPERATOR";
    case COMMA:
        return "COMMA";
    case OPEN_PARENTHESIS:
        return "OPEN_PARENTHESIS";
    case CLOSE_PARENTHESIS:
        return "CLOSE_PARENTHESIS";
    case UNDEFINED:
        return "UNDEFINED";
    }

    return "UNDEFINED";
}

token_vec *new_token_vec(int max_size)
{
    token_vec *t = malloc(sizeof(token_vec));
    t->tokens = malloc(sizeof(token) * max_size);
    t->len = 0;
    t->cur_parsing = 0;
    t->max_size = max_size;
    return t;
}

token get_current_token(token_vec *v)
{
    if (v->cur_parsing > v->len)
        return (token){"", UNDEFINED};

    return v->tokens[v->cur_parsing];
}

token_vec *resize_token_vec(token_vec *v, int new_size)
{
    assert(new_size > v->max_size);
    v->tokens = realloc(v->tokens, (new_size * sizeof(token)));
    v->max_size = new_size;
    return v;
}

token_vec *add_token(token_vec *v, char *s, token_type t)
{
    if (v->len >= v->max_size - 1)
    {
        v = resize_token_vec(v, v->max_size * 2);
    }

    int len = v->len;
    v->tokens[len].value = s;
    v->tokens[len].type = t;
    v->len++;

    return v;
}

void print_token(token t)
{
    printf("\n%s %s", t.value, get_token_type_name(t.type));
}

void print_tokens(token_vec *v)
{

    for (int i = 0; i < v->len; i++)
    {
        printf("\n[%d] %s %s", i, v->tokens[i].value,
               get_token_type_name(v->tokens[i].type));
    }
}

void print_expression(token_vec *v)
{
    for (int i = 0; i < v->len; i++)
    {
        printf("%s", v->tokens[i].value);
    }
}

void free_token_vec(token_vec *v)
{
    for (int i = 0; i < v->len; i++)
    {
        free(v->tokens[i].value);
    }
    free(v->tokens);
    free(v);
}
