#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"
#include <stdbool.h>

token_vec *tokenize(const char *s);
token_type get_token_type(const char *s);

bool is_literal(const char *s);
bool is_identifier(const char *s);
bool is_integer(const char *s);
bool is_float(const char *s);

bool is_operator(char c);
bool is_parenthesis(char c);
bool is_open_parenthesis(char c);
bool is_close_parenthesis(char c);
bool is_comma(char c);
bool is_assignment_operator(char c);
bool is_multi_char_operator(char c1, char c2);
bool can_be_unary(char c);

bool is_right_associative(char *operator);
int get_precedence(char *operator);

#endif /* TOKENIZER_H */
