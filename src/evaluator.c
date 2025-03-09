#include "../include/evaluator.h"
#include "../include/eval.h"
#include "../include/parser.h"
#include "../include/token.h"
#include <ffi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double token_to_number(token t) { return atof(t.value); }

double call_extern_function(struct extern_function_info *info,
                            const double *args)
{
    ffi_cif cif;
    ffi_type *arg_types[info->num_args];
    void *arg_values[info->num_args];

    for (int i = 0; i < info->num_args; i++)
    {
        arg_types[i] = &ffi_type_double;
        arg_values[i] = (void *)&args[i];
    }

    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, info->num_args, &ffi_type_double,
                     arg_types) != FFI_OK)
    {
        fprintf(stderr, "ffi_prep_cif failed\n");
        exit(EXIT_FAILURE);
    }

    double result;
    ffi_call(&cif, FFI_FN(info->function), &result, arg_values);
    return result;
}

eval_result evaluate_expression(node *n, dictionary *d)
{
    dictionary *default_dict = get_default_dictionary();
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
        int id_default = search_dict(default_dict, fn->name);

        if (id == -1 && id_default == -1)
        {
            result.value = 0.0;
            result.error = true;
            sprintf(result.error_msg, "function '%s' doesn't exist", fn->name);
            return result;
        }
        else if (id != -1)
        {
            if (d->entries[id].type != FUNCTION &&
                d->entries[id].type != EXTERN_FUNCTION)
            {
                result.error = true;
                result.value = 0.0;
                sprintf(result.error_msg, "'%s' is a variable, not a function",
                        fn->name);
                return result;
            }

            if (d->entries[id].type == FUNCTION)
            {
                struct stored_function_info *stored_func =
                    d->entries[id].function;
                dictionary *arg_dict = stored_func->args;
                
                
                if (fn->num_args != stored_func->num_args)
                {
                    result.error = true;
                    result.value = 0.0;
                    sprintf(result.error_msg,
                            "Wrong number of arguments for %s", fn->name);
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

                eval_result ev =
                    evaluate_expression(stored_func->ast, arg_dict);

                return ev;
            }
            else if (d->entries[id].type == EXTERN_FUNCTION)
            {
                struct extern_function_info *ext =
                    d->entries[id].external_function;

                if (fn->num_args != ext->num_args)
                {
                    result.error = true;
                    result.value = 0.0;
                    sprintf(result.error_msg,
                            "Wrong number of arguments for %s", fn->name);
                    return result;
                }

                double *arguments = malloc(sizeof(double) * ext->num_args);
                for (int i = 0; i < ext->num_args; i++)
                {
                    eval_result res = evaluate_expression(fn->args[i], d);
                    if (!res.error)
                    {
                        arguments[i] = res.value;
                    }
                    else
                    {
                        result.error = true;
                        strcpy(result.error_msg, res.error_msg);
                        result.value = 0.0;
                        return result;
                    }
                }

                double result = call_extern_function(ext, arguments);
                free(arguments);
                return (eval_result){result, false, 0, false, ""};
            }
        }
        else if (id_default != -1)
        {
            if (default_dict->entries[id_default].type != FUNCTION &&
                default_dict->entries[id_default].type != EXTERN_FUNCTION)
            {
                result.error = true;
                result.value = 0.0;
                sprintf(result.error_msg, "'%s' is a variable, not a function",
                        fn->name);
                return result;
            }
            if (default_dict->entries[id_default].type == FUNCTION)
            {
                struct stored_function_info *stored_func =
                    default_dict->entries[id_default].function;
                dictionary *arg_dict = stored_func->args;

                if (fn->num_args != stored_func->num_args)
                {
                    result.error = true;
                    result.value = 0.0;
                    sprintf(result.error_msg,
                            "Wrong number of arguments for %s", fn->name);
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
                        printf("dajsdhd\n");
                        // result.error = true;
                        printf("%s\n", res.error_msg);
                        // strcpy(result.error_msg, res.error_msg);
                        // result.value = 0.0;
                        // return result;
                    }
                }

                eval_result ev =
                    evaluate_expression(stored_func->ast, arg_dict);

                return ev;
            }
        }
    }

    else if (n->type == NUMBER_NODE)
    {
        result.value = n->value;
        return result;
    }
    else if (n->type == IDENTIFIER_NODE)
    {
        // For non function calls, the dict passed as argument and default are
        // the same.
        //  TODO: change this :/

        int id = search_dict(d, n->name);
        int id_default = search_dict(default_dict, n->name);

        if (id == -1 && id_default == -1)
        {
            result.error = true;
            result.value = 0.0;
            sprintf(result.error_msg, "'%s' doesn't exist", n->name);
            return result;
        }
        else if (id != -1)
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
        else if (id_default != -1)
        {
            if (default_dict->entries[id_default].type != VARIABLE)
            {
                result.error = true;
                result.value = 0.0;
                sprintf(result.error_msg, "'%s' is a function, not a variable",
                        n->name);
                return result;
            }
            result.value = default_dict->entries[id_default].value;
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
