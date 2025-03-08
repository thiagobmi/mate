#include "../include/eval.h"
#include "../include/constants.h"
#include "../include/dictionary.h"
#include "../include/evaluator.h"
#include "../include/node.h"
#include "../include/parser.h"
#include "../include/tokenizer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

dictionary *get_default_dictionary()
{
    static dictionary *default_dict = NULL;

    if (default_dict == NULL)
    {
        default_dict = new_dict(1);
    }
    return default_dict;
}

void mate_cleanup()
{
    dictionary *default_dict = get_default_dictionary();

    if (default_dict != NULL)
    {
        free_dict(default_dict);
    }
}

void add_function(char *name, void *function, int num_args)
{

    dictionary *dict = get_default_dictionary();
    entry *et = malloc(sizeof(entry));
    struct extern_function_info *ext= malloc(sizeof(struct extern_function_info));
    ext->function = function;
    ext->num_args = num_args;
    et->external_function = ext;
    et->key = strdup(name);
    et->type = EXTERN_FUNCTION;
    dictionary_add_function(dict,et);
    free(et);

}

eval_result eval(const char *expression)
{

    dictionary *dict = get_default_dictionary();
    token_vec *v = tokenize(expression);
    expression_info e = valid_expression(v);
    int error_ptr = e.error_at;

    if (!e.is_valid)
    {
        free_token_vec(v);
        return (eval_result){0.0, true, error_ptr, false, ""};
    }

    if (e.is_function_declaration)
    {
        entry *et = parse_function_declaration(v);
        dict = dictionary_add_function(dict, et);
        free(et);
        free_token_vec(v);
        return (eval_result){1.0, false, 0, true, ""};
    }

    node *root = compute_expression(v);

    if (root == NULL)
    {
        free_token_vec(v);
        return (eval_result){0.0, true, 0, false,
                             "Error: Failed to parse expression"};
    }
    else
    {
        eval_result result = evaluate_expression(root, dict);
        free_token_vec(v);
        free_tree(root);
        return result;
    }
}
