#include "../include/parser.h"
#include "../include/dictionary.h"
#include "../include/eval.h"
#include "../include/evaluator.h"
#include "../include/tokenizer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool match(token_vec *v, token_type t)
{

    if (v->cur_parsing >= v->len)
        return false;

    token tk = get_current_token(v);
    if (tk.type == t)
    {
        // print_token(tk);
        v->cur_parsing++;
        return true;
    }

    return false;
}

bool vec_contains_assignment(token_vec *v)
{
    for (int i = 0; i < v->len; i++)
    {
        if (v->tokens[i].type == ASSIGNMENT_OPERATOR)
            return true;
    }
    return false;
}

expression_info valid_expression(token_vec *v)
{
    int count_brackets = 0;
    bool assignment = false;
    bool is_function_declaration = false;
    bool can_be_assignment = true;
    bool is_function_call = false;
    bool declared_function = false;
    char *name_pointer = NULL;
    char *function_name = NULL;
    enum expression_state state = EXPR_START;
    expression_info e;
    dictionary *d = new_dict(1);
    dictionary *default_dict = get_default_dictionary();

    // TODO: Add support for nested functions in arguments;
    while (true)
    {
        token current_token = get_current_token(v);
        switch (state)
        {
        case EXPR_FUNCTION_IDENTIFIER:
            if (match(v, COMMA))
            {
                state = EXPR_FUNCTION_COMMA;
            }
            else if (match(v, CLOSE_PARENTHESIS))
            {
                state = EXPR_FUNCTION_CLOSE_PARENTHESIS;
            }
            else
            {
                state = EXPR_ERROR;
                continue;
            }
            break;

        case EXPR_FUNCTION_COMMA:
            if (match(v, IDENTIFIER))
            {
                int result = search_dict(d, current_token.value);
                if (result != -1)
                {
                    state = EXPR_ERROR;
                    continue;
                }
                else
                {
                    d = add_entry(d, strdup(current_token.value), 0.0);
                    state = EXPR_FUNCTION_IDENTIFIER;
                }
            }
            else
            {
                state = EXPR_ERROR;
                continue;
            }
            break;

        case EXPR_FUNCTION_PARENTHESIS:
            is_function_declaration =
                vec_contains_assignment(v) && !assignment ? true : false;
            is_function_call = (!is_function_declaration) || declared_function;
            is_function_declaration = !is_function_call;
            if (is_function_declaration)
                function_name = name_pointer;

            if (match(v, IDENTIFIER))
            {
                if (is_function_declaration)
                {
                    d = add_entry(d, strdup(current_token.value), 0.0);
                    state = EXPR_FUNCTION_IDENTIFIER;
                }
                else
                {
                    state = EXPR_LITERAL_OR_IDENTIFIER;
                }
                continue;
            }
            if (match(v, LITERAL))
            {
                if (is_function_call)
                {
                    state = EXPR_LITERAL_OR_IDENTIFIER;
                }
                else
                {
                    state = EXPR_ERROR;
                }
                continue;
            }
            else if (match(v, UNARY_OPERATOR))
            {
                if (is_function_call)
                {
                    state = EXPR_START;
                }
            }
            else if (match(v, OPEN_PARENTHESIS))
            {
                if (is_function_call)
                {
                    state = EXPR_PARENTHESIS;
                }
            }
            else if (match(v, CLOSE_PARENTHESIS))
            {
                state = EXPR_FUNCTION_CLOSE_PARENTHESIS;
            }
            else
            {
                state = EXPR_ERROR;
                continue;
            }
            break;

        case EXPR_FUNCTION_CLOSE_PARENTHESIS:
            if (match(v, ASSIGNMENT_OPERATOR))
            {
                if (is_function_declaration)
                {
                    declared_function = true;
                    state = EXPR_START;
                }
                else
                    state = EXPR_ERROR;
                continue;
            }
            else
            {
                if (is_function_call)
                    state = EXPR_DONE;
                else
                    state = EXPR_ERROR;
                continue;
            }
            break;

        case EXPR_START:
            if (match(v, IDENTIFIER))
            {
                if (is_function_declaration)
                {
                    int result = search_dict(d, current_token.value);
                    int result_default =
                        search_dict(default_dict, current_token.value);

                    if (result == -1 && result_default == -1)
                    {
                        state = EXPR_ERROR;
                        continue;
                    }
                    else
                    {
                        if (strcmp(current_token.value, function_name) == 0)
                            state = EXPR_ERROR;
                        else
                            state = EXPR_LITERAL_OR_IDENTIFIER;
                    }
                }
                else
                {
                    name_pointer = current_token.value;
                    state = EXPR_LITERAL_OR_IDENTIFIER;
                }
            }
            else if (match(v, LITERAL))
            {
                state = EXPR_LITERAL_OR_IDENTIFIER;
            }
            else if (match(v, OPEN_PARENTHESIS))
            {
                state = EXPR_PARENTHESIS;
            }
            else if (match(v, UNARY_OPERATOR))
            {
                state = EXPR_START;
            }
            else
            {
                state = EXPR_ERROR;
                continue;
            }
            break;

        case EXPR_LITERAL_OR_IDENTIFIER:
            if (match(v, OPERATOR))
            {
                can_be_assignment = false;
                state = EXPR_OPERATOR;
            }
            else if (match(v, COMMA))
            {
                if (count_brackets == 0)
                    state = EXPR_START;
                else
                    state = EXPR_ERROR;
            }
            else if (match(v, ASSIGNMENT_OPERATOR))
            {
                if (assignment || !can_be_assignment)
                {
                    state = EXPR_ERROR;
                    continue;
                }
                else
                {
                    assignment = true;
                    state = EXPR_START;
                }
            }
            else if (match(v, CLOSE_PARENTHESIS))
            {
                if (count_brackets == 0)
                {
                    if (is_function_call)
                    {
                        if (v->cur_parsing >= v->len)
                        {
                            state = EXPR_DONE;
                        }
                        else
                        {
                            is_function_call = false;
                            state = EXPR_LITERAL_OR_IDENTIFIER;
                        }
                    }
                    continue;
                }
                else
                {
                    count_brackets--;
                    state = EXPR_LITERAL_OR_IDENTIFIER;
                }
            }
            else if (match(v, OPEN_PARENTHESIS))
            {
                state = EXPR_FUNCTION_PARENTHESIS;
            }
            else if (v->cur_parsing >= v->len)
            {
                if (count_brackets == 0 && !is_function_call)
                {
                    state = EXPR_DONE;
                    break;
                }
                else
                {
                    state = EXPR_ERROR;
                    continue;
                }
            }
            else
            {
                state = EXPR_ERROR;
                continue;
            }
            break;

        case EXPR_OPERATOR:
            if (match(v, IDENTIFIER))
            {
                if (is_function_declaration)
                {
                    int result = search_dict(d, current_token.value);
                    int result_default =
                        search_dict(default_dict, current_token.value);
                    if (result == -1 && result_default == -1)
                    {
                        state = EXPR_ERROR;
                        continue;
                    }
                    else
                    {
                        if (strcmp(current_token.value, function_name) == 0)
                            state = EXPR_ERROR;
                        else
                            state = EXPR_LITERAL_OR_IDENTIFIER;
                    }
                }
                else
                    state = EXPR_LITERAL_OR_IDENTIFIER;
            }
            else if (match(v, LITERAL))
            {
                state = EXPR_LITERAL_OR_IDENTIFIER;
            }
            else if (match(v, OPEN_PARENTHESIS))
            {
                state = EXPR_PARENTHESIS;
            }
            else if (match(v, UNARY_OPERATOR))
            {
                state = EXPR_START;
            }
            else
            {
                state = EXPR_ERROR;
                continue;
            }
            break;

        case EXPR_PARENTHESIS:
            count_brackets++;
            state = EXPR_START;
            break;

        case EXPR_DONE:
            free_dict(d);
            v->cur_parsing = 0;
            e.is_valid = true;
            e.is_assignment = assignment;
            e.is_function_declaration =
                is_function_declaration || declared_function;
            return e;
            break;

        case EXPR_ERROR:
            free_dict(d);
            e.is_valid = false;
            e.error_at = v->cur_parsing;
            v->cur_parsing = 0;
            e.is_assignment = assignment;
            e.is_function_declaration =
                is_function_declaration || declared_function;
            return e;
            break;
        }
    }
    return (expression_info){0, 0, 0, 0};
}

node *compute_expression_by_prec(token_vec *v, int min_prec)
{
    node *left = parse_factor(v);
    if (left == NULL)
        return NULL;

    while (true)
    {
        if (v->cur_parsing >= v->len)
            break;

        token current_token = get_current_token(v);

        if (current_token.type != OPERATOR &&
            current_token.type != ASSIGNMENT_OPERATOR)
            break;

        int precedence = get_precedence(current_token.value);
        bool associative = is_right_associative(current_token.value);

        if (precedence < min_prec)
            break;

        v->cur_parsing++;

        int next_min_precedence = associative ? precedence : precedence + 1;
        node *right = compute_expression_by_prec(v, next_min_precedence);

        char *operator= strdup(current_token.value);
        left = new_op_node(operator, left, right);
    }

    return left;
}

node *compute_expression(token_vec *v)
{
    return compute_expression_by_prec(v, 0);
}

bool is_function(token_vec *v)
{
    int index = v->cur_parsing;
    token c = v->tokens[index];
    if (c.type == IDENTIFIER)
    {
        index++;
        if (index < v->len)
        {
            token c = v->tokens[index];
            if (c.type == OPEN_PARENTHESIS)
            {
                return true;
            }
        }
    }
    return false;
}

token_vec *get_function(token_vec *v)
{
    token_vec *func = new_token_vec(5);
    int count_brackets = 0;

    while (true)
    {
        if (v->cur_parsing > v->len)
            break;

        token tk = get_current_token(v);

        add_token(func, strdup(tk.value), tk.type);

        if (match(v, OPEN_PARENTHESIS))
        {
            count_brackets++;
        }
        else if (match(v, CLOSE_PARENTHESIS))
        {
            if (count_brackets > 1)
            {
                count_brackets--;
            }
            else
            {
                return func;
            }
        }
        else
        {
            v->cur_parsing++;
        }
    }

    return NULL;
}

entry *parse_function_declaration(token_vec *v)
{
    token_vec *e = new_token_vec(1);
    v->cur_parsing = 0;
    int count_args = 0;
    dictionary *d = new_dict(1);

    token tk = get_current_token(v);

    if (!match(v, IDENTIFIER))
        return NULL;
    char *name = strdup(tk.value);

    if (!match(v, OPEN_PARENTHESIS))
        return NULL;

    tk = get_current_token(v);

    while (match(v, IDENTIFIER))
    {
        add_entry(d, strdup(tk.value), 0.0);
        count_args++;
        match(v, COMMA);
        tk = get_current_token(v);
    }

    if (!match(v, CLOSE_PARENTHESIS))
        return NULL;
    if (!match(v, ASSIGNMENT_OPERATOR))
        return NULL;

    while (v->cur_parsing < v->len)
    {
        token tk = get_current_token(v);

        add_token(e, strdup(tk.value), tk.type);
        v->cur_parsing++;
    }

    entry *et = malloc(sizeof(entry));
    struct stored_function_info *func =
        malloc(sizeof(struct stored_function_info));
    
    node *root = compute_expression(e);
    free_token_vec(e);

    func->args = d;
    func->num_args = count_args;
    func->ast = root;

    et->type = FUNCTION;
    et->key = name;
    et->function = func;

    return et;
}

struct parsed_function_info *parse_function(struct function_info *f)
{
    struct parsed_function_info *p =
        malloc(sizeof(struct parsed_function_info));
    p->num_args = f->num_args;
    p->name = strdup(f->name);
    p->args = malloc(sizeof(node *) * p->num_args);

    for (int i = 0; i < f->num_args; i++)
    {
        node *root = compute_expression(f->args[i]);
        p->args[i] = root;
    }

    return p;
}

node *parse_factor(token_vec *v)
{
    if (v->cur_parsing >= v->len)
    {
        printf("\nErro: Tentativa de acessar token além do limite");
        return NULL;
    }

    token current_token = get_current_token(v);
    node *n = NULL;

    if (is_function(v))
    {
        token_vec *func = get_function(v);
        struct function_info *f = get_function_arguments(func);
        struct parsed_function_info *p = parse_function(f);

        if (func != NULL)
        {
            free_token_vec(func);
        }

        if (f != NULL)
        {
            if (f->name != NULL)
            {
                free(f->name);
            }
            if (f->num_args > 0)
            {
                for (int i = 0; i < f->num_args; i++)
                {
                    if (f->args[i] != NULL)
                    {
                        free_token_vec(f->args[i]);
                    }
                }
                free(f->args);
            }
            free(f);
        }
        n = new_function_node(p);
    }

    else if (match(v, LITERAL))
    {
        n = new_number_node(token_to_number(current_token));
    }
    else if (match(v, IDENTIFIER))
    {
        n = new_identifier_node(strdup(current_token.value));
    }
    else if (match(v, OPEN_PARENTHESIS))
    {
        n = compute_expression(v);
        if (n == NULL || !match(v, CLOSE_PARENTHESIS))
        {
            printf("\nErro: Expressão inválida ou falta parêntese de "
                   "fechamento\n");
            if (n != NULL)
            {
                free_tree(n);
            }
            return NULL;
        }
    }
    else if (match(v, UNARY_OPERATOR))
    {
        node *left = parse_factor(v);
        if (left == NULL)
        {
            return NULL;
        }
        n = new_unary_op_node(strdup(current_token.value), left);
    }
    else
    {
        printf("\nErro: Token inesperado\n");
        print_token(current_token);
        return NULL;
    }

    return n;
}

struct function_info *get_function_arguments(token_vec *v)
{
    enum func_state state = FUNC_START;
    int count_brackets = 0;

    struct function_info *f = malloc(sizeof(struct function_info));
    f->args = NULL;
    f->num_args = 0;
    token_vec *a = NULL;

    while (v->cur_parsing < v->len)
    {
        token tk = get_current_token(v);

        switch (state)
        {
        case FUNC_START:
            if (match(v, IDENTIFIER))
            {
                f->name = strdup(tk.value);
                state = FUNC_NAME;
                continue;
            }
            else
                state = FUNC_ERROR;
            break;

        case FUNC_NAME:
            if (match(v, OPEN_PARENTHESIS))
            {
                state = FUNC_PAREN;
            }
            else
                state = FUNC_ERROR;
            break;

        case FUNC_PAREN:
            if (match(v, CLOSE_PARENTHESIS))
            {
                v->cur_parsing = 0;
                f->num_args = 0;
                state = FUNC_DONE;
            }
            else
            {
                state = FUNC_ARG;
            }
            break;

        case FUNC_ARG:
            if (f->num_args == 0)
            {
                a = new_token_vec(1);
                f->args = malloc(sizeof(token_vec *));
                f->args[0] = a;
                f->num_args++;
            }
            else
            {
                a = new_token_vec(1);
                f->num_args++;
                f->args = realloc(f->args, sizeof(token_vec *) * f->num_args);
                f->args[f->num_args - 1] = a;
            }
            state = FUNC_ACCEPT;
            break;

        case FUNC_ACCEPT:
            if (match(v, IDENTIFIER) || match(v, UNARY_OPERATOR) ||
                match(v, OPERATOR) || match(v, LITERAL))
            {
                a = add_token(a, strdup(tk.value), tk.type);
            }
            else if (match(v, COMMA))
            {
                state = FUNC_ARG;
            }
            else if (match(v, OPEN_PARENTHESIS))
            {
                count_brackets++;
                a = add_token(a, strdup(tk.value), tk.type);
            }
            else if (match(v, CLOSE_PARENTHESIS))
            {
                if (count_brackets == 0)
                {
                    v->cur_parsing = 0;
                    state = FUNC_DONE;
                    return f;
                }
                else
                {
                    count_brackets--;
                    a = add_token(a, strdup(tk.value), tk.type);
                }
            }
            break;

        case FUNC_DONE:
            v->cur_parsing = 0;
            return f;
            break;

        case FUNC_ERROR:
            // TODO: free arguments in case of error.
            v->cur_parsing = 0;
            return NULL;
            break;
        }
    }

    return NULL;
}
