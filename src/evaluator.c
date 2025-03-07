#include "../include/evaluator.h"
#include "../include/parser.h"
#include "../include/token.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double token_to_number(token t) { return atof(t.value); }

eval_result evaluate_expression(node *n, dictionary *d)
{
    eval_result result = {0.0, false, false, 0, ""};

    if (n == NULL)
    {
        result.error = true;
        strcpy(result.error_msg, "NULL node encountered");
        return result;
    }

    if (n->type == FUNCTION_NODE)
    {
        struct parsed_function_info *fn = n->function;
        int id = search_dict(d, fn->name);

        if (id == -1)
        {
            result.value = 0.0;
            result.error = true;
            sprintf(result.error_msg, "function '%s' doesn't exist", fn->name);
            return result;
        }
        else
        {
            if (d->entries[id].type != FUNCTION)
            {
                result.error = true;
                result.value = 0.0;
                sprintf(result.error_msg, "'%s' is a variable, not a function",
                        fn->name);
                return result;
            }
            struct stored_function_info *stored_func = d->entries[id].function;
            dictionary *arg_dict = stored_func->args;

            if (fn->num_args != stored_func->num_args)
            {
                result.error = true;
                result.value = 0.0;
                sprintf(result.error_msg, "Wrong number of arguments for %s",
                        fn->name);
                return result;
            }

            for (int i = 0; i < arg_dict->len; i++)
            {
                eval_result res = evaluate_expression(fn->args[i], d);
                if (!res.error)
                {
                    arg_dict->entries[i].value = res.value;
                }
                else
                {
                    result.error = true;
                    strcpy(result.error_msg, res.error_msg);
                    result.value = 0.0;
                    return result;
                }
            }

            eval_result ev = evaluate_expression(stored_func->ast, arg_dict);

            if (ev.error && strlen(ev.error_msg) > 0)
                printf("\n Error: %s\n", ev.error_msg);

            return ev;
        }
    }

    else if (n->type == NUMBER_NODE)
    {
        result.value = n->value;
        return result;
    }
    else if (n->type == IDENTIFIER_NODE)
    {
        int id = search_dict(d, n->name);
        if (id == -1)
        {
            result.error = true;
            result.value = 0.0;
            sprintf(result.error_msg, "'%s' doesn't exist", n->name);
            return result;
        }
        else
        {
            if (d->entries[id].type != VARIABLE)
            {
                result.error = true;
                result.value = 0.0;
                sprintf(result.error_msg, "'%s' is a function, not a variable",
                        n->name);
                return result;
            }
            result.value = d->entries[id].value;
            return result;
        }
    }
    else if (n->type == OPERATOR_NODE)
    {
        if (strcmp(n->op, "=") == 0)
        {
            eval_result right_result = evaluate_expression(n->right, d);

            if (right_result.error)
            {
                return right_result;
            }

            if (n->left->type == IDENTIFIER_NODE)
            {
                result.is_assignment = true;
                result.value = right_result.value;

                int id = search_dict(d, n->left->name);
                if (id != -1)
                {
                    d->entries[id].value = right_result.value;
                }
                else
                {
                    d = add_entry(d, strdup(n->left->name), right_result.value);
                }
                return result;
            }
            else
            {
                result.error = true;
                strcpy(result.error_msg,
                       "Left side of assignment must be a variable");
                return result;
            }
        }
        else
        {
            eval_result left_result = evaluate_expression(n->left, d);

            if (left_result.error)
            {
                return left_result;
            }

            eval_result right_result = evaluate_expression(n->right, d);

            if (right_result.error)
            {
                return right_result;
            }

            double left_value = left_result.value;
            double right_value = right_result.value;

            if (strcmp(n->op, "||") == 0)
                result.value = left_value || right_value;
            else if (strcmp(n->op, "&&") == 0)
                result.value = left_value && right_value;
            else if (strcmp(n->op, "&") == 0)
                result.value = (int)left_value & (int)right_value;
            else if (strcmp(n->op, "==") == 0)
                result.value = left_value == right_value;
            else if (strcmp(n->op, "!=") == 0)
                result.value = left_value != right_value;
            else if (strcmp(n->op, "<") == 0)
                result.value = left_value < right_value;
            else if (strcmp(n->op, ">") == 0)
                result.value = left_value > right_value;
            else if (strcmp(n->op, "<=") == 0)
                result.value = left_value <= right_value;
            else if (strcmp(n->op, ">=") == 0)
                result.value = left_value >= right_value;

            // Arithmetic operators
            else if (strcmp(n->op, "-") == 0)
                result.value = left_value - right_value;
            else if (strcmp(n->op, "+") == 0)
                result.value = left_value + right_value;
            else if (strcmp(n->op, "*") == 0)
                result.value = left_value * right_value;
            else if (strcmp(n->op, "/") == 0)
            {
                if (right_value == 0)
                {
                    result.error = true;
                    strcpy(result.error_msg, "Division by zero");
                    return result;
                }
                result.value = left_value / right_value;
            }
            else if (strcmp(n->op, "%") == 0)
            {
                if ((int)right_value == 0)
                {
                    result.error = true;
                    strcpy(result.error_msg, "Modulo by zero");
                    return result;
                }
                result.value = (int)left_value % (int)right_value;
            }
            else if (strcmp(n->op, "^") == 0)
                result.value = pow(left_value, right_value);
            else
            {
                result.error = true;
                sprintf(result.error_msg, "Unknown operator: %s", n->op);
                return result;
            }

            return result;
        }
    }
    else if (n->type == UNARY_OPERATOR_NODE)
    {
        eval_result operand_result = evaluate_expression(n->left, d);

        if (operand_result.error)
        {
            return operand_result;
        }

        double operand_value = operand_result.value;

        if (strcmp(n->op, "-") == 0)
            result.value = -operand_value;
        else if (strcmp(n->op, "!") == 0)
            result.value = !operand_value;
        else
        {
            result.error = true;
            sprintf(result.error_msg, "Unknown unary operator: %s", n->op);
            return result;
        }

        return result;
    }

    result.error = true;
    strcpy(result.error_msg, "Invalid node type");
    return result;
}
