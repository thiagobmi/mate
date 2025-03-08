#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "constants.h"
#include "dictionary.h"
#include "node.h"
#include "token.h"
#include <stdbool.h>

typedef struct
{
    double value;
    bool error;
    int error_at;
    bool is_assignment;
    char error_msg[MAX_INPUT_SIZE];
} eval_result;

double call_extern_function(struct extern_function_info *info, const double *args);
eval_result evaluate_expression(node *n, dictionary *d);
double token_to_number(token t);

#endif /* EVALUATOR_H */
