#include "../include/tokenizer.h"
#include "../include/constants.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

operator_info operators[] = {
    {"=", 0, false, false},
    {"||", 10, false, false}, // Logical OR (lowest precedence)
    {"&&", 20, false, false}, // Logical AND
    {"&", 50, false, false},  // Bitwise AND
    {"==", 60, false, false}, // Equality
    {"!=", 60, false, false}, // Inequality
    {"<", 70, false, false},  // Less than
    {">", 70, false, false},  // Greater than
    {"<=", 70, false, false}, // Less than or equal
    {">=", 70, false, false}, // Greater than or equal
    {"+", 80, false, false},  // Addition
    {"-", 90, false, false},  // Subtraction
    {"*", 100, false, false}, // Multiplication
    {"/", 100, false, false}, // Division
    {"%", 100, false, false}, // Modulo
    {"^", 110, true,
     false}, // Exponentiation (high precedence, right-associative)
    {"-", 110, false,
     true}, // Unary minus (higher precedence than binary operators)
    {"!", 110, false, true}, // Logical NOT
};
const int NUM_OPERATORS = sizeof(operators) / sizeof(operator_info);

int get_precedence(char *operator)
{
    for (int i = 0; i < NUM_OPERATORS; i++)
        if (strcmp(operator, operators[i].op) == 0)
            return operators[i].precedence;
    return false;
}

bool is_right_associative(char *operator)
{
    for (int i = 0; i < NUM_OPERATORS; i++)
        if (strcmp(operator, operators[i].op) == 0)
            return operators[i].right_associative;
    return false;
}

bool can_be_unary(char c)
{
    bool is_unary = false;

    for (int i = 0; i < NUM_OPERATORS; i++)
    {
        if (strlen(operators[i].op) == 1 && c == operators[i].op[0] &&
            operators[i].is_unary)
            is_unary = true;
    }
    return is_unary;
}
bool is_operator(char c)
{
    for (int i = 0; i < NUM_OPERATORS; i++)
    {
        if (strlen(operators[i].op) == 1 && operators[i].op[0] == c)
        {
            return true;
        }
    }
    return false;
}

bool is_parenthesis(char c) { return c == '(' || c == ')'; }

bool is_open_parenthesis(char c) { return c == '('; }

bool is_close_parenthesis(char c) { return c == ')'; }

bool is_assignment_operator(char c) { return c == '='; }

bool is_comma(char c) { return c == ','; }

bool is_multi_char_operator(char c1, char c2)
{
    for (int i = 0; i < NUM_OPERATORS; i++)
    {
        if (strlen(operators[i].op) == 2)
        {
            char a1 = operators[i].op[0];
            char a2 = operators[i].op[1];
            if (a1 == c1 && a2 == c2)
                return true;
        }
    }
    return false;
}

// IDENTIFIER: ; Padrão de formação: letra(letra+dígito)*
bool is_identifier(const char *s)
{
    if (isdigit(s[0])) // identifiers can't start with digits
        return 0;

    if (s[strlen(s) - 1] == '-')
        return 0;

    for (size_t i = 0; i < strlen(s); i++)
    {
        if (i == 0 && s[i] == '-')
            continue;

        if (!isdigit(s[i]) && !isalpha(s[i]) && s[i] != '_')
            return 0;
    }

    return 1;
}

bool is_integer(const char *s)
{

    if (s[strlen(s) - 1] == '-')
        return 0;

    for (size_t i = 0; i < strlen(s); i++)
    {
        if (i == 0 && s[i] == '-')
            continue;

        if (!isdigit(s[i]))
            return 0;
    }

    return 1;
}

bool is_float(const char *s)
{
    int just_one_dot = 0;
    int len = strlen(s);

    if (s[0] == '.' || s[len - 1] == '-' || s[len - 1] == '.')
        return 0;

    for (size_t i = 0; i < strlen(s); i++)
    {
        if (i == 0 && s[i] == '-')
            continue;

        if (s[i] == '.')
        {
            if (just_one_dot == 0)
                just_one_dot = 1;
            else
                return 0;
        }

        else if (!isdigit(s[i]))
            return 0;
    }

    return 1;
}

bool is_literal(const char *s) { return is_integer(s) || is_float(s); }

token_type get_token_type(const char *s)
{
    if (is_literal(s))
        return LITERAL;
    else if (strlen(s) == 1 && is_comma(s[0]))
        return COMMA;
    else if (is_identifier(s))
        return IDENTIFIER;
    else if (strlen(s) == 1 && is_assignment_operator(s[0]))
        return ASSIGNMENT_OPERATOR;
    else if (strlen(s) == 1 && is_operator(s[0]))
        return OPERATOR;
    else if (strlen(s) == 2 && is_multi_char_operator(s[0], s[1]))
        return OPERATOR;
    else if (strlen(s) == 1 && is_open_parenthesis(s[0]))
        return OPEN_PARENTHESIS;
    else if (strlen(s) == 1 && is_close_parenthesis(s[0]))
        return CLOSE_PARENTHESIS;
    else
        return UNDEFINED;
}

token_vec *tokenize(const char *s)
{
    assert(s != NULL);
    assert(strlen(s) <= MAX_INPUT_SIZE);

    char buffer[MAX_INPUT_SIZE];
    int buffer_len = 0;
    token_vec *v = new_token_vec(10);
    int s_len = strlen(s);
    enum token_type tp = UNDEFINED;

    for (int i = 0; i < s_len; i++)
    {
        char a = s[i];

        if (a == ' ')
        {
            if (buffer_len > 0)
            {
                buffer[buffer_len] = '\0';
                char *n = strdup(buffer);
                tp = get_token_type(n);
                add_token(v, n, tp);
                buffer_len = 0;
            }
        }
        else if (i + 1 < s_len && is_multi_char_operator(s[i], s[i + 1]))
        {
            if (buffer_len > 0)
            {
                buffer[buffer_len] = '\0';
                char *n = strdup(buffer);
                tp = get_token_type(n);
                add_token(v, n, tp);
                buffer_len = 0;
            }

            buffer[buffer_len++] = s[i];
            buffer[buffer_len++] = s[i + 1];
            buffer[buffer_len] = '\0';
            char *n = strdup(buffer);
            tp = get_token_type(n);
            add_token(v, n, tp);
            buffer_len = 0;
            i++;
        }
        else if (can_be_unary(a) &&
                 (tp == OPERATOR || tp == UNDEFINED || tp == UNARY_OPERATOR ||
                  tp == OPEN_PARENTHESIS || tp == ASSIGNMENT_OPERATOR ||
                  tp == COMMA))
        {
            if (buffer_len > 0)
            {
                buffer[buffer_len] = '\0';
                char *n = strdup(buffer);
                tp = get_token_type(n);
                add_token(v, n, tp);
                buffer_len = 0;

                i--;
                continue;
            }

            buffer[buffer_len++] = a;
            buffer[buffer_len] = '\0';
            char *n = strdup(buffer);
            tp = UNARY_OPERATOR;
            add_token(v, n, tp);
            buffer_len = 0;
        }
        else if (is_operator(a) || is_parenthesis(a) || is_comma(a))
        {
            if (buffer_len > 0)
            {
                buffer[buffer_len] = '\0';
                char *n = strdup(buffer);
                tp = get_token_type(n);
                add_token(v, n, tp);
                buffer_len = 0;
            }

            buffer[buffer_len++] = a;
            buffer[buffer_len] = '\0';
            char *n = strdup(buffer);
            tp = get_token_type(n);
            add_token(v, n, tp);
            buffer_len = 0;
        }
        else
        {
            buffer[buffer_len++] = a;
        }
    }

    if (buffer_len > 0)
    {
        buffer[buffer_len] = '\0';
        char *n = strdup(buffer);
        tp = get_token_type(n);
        add_token(v, n, tp);
        buffer_len = 0;
    }

    return v;
}
