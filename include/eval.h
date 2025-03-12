#ifndef EVAL_H
#define EVAL_H
#include "evaluator.h"

void mate_cleanup();
dictionary *get_default_dictionary();
void add_function(char *name, void *function, int num_args);
eval_result eval(const char *expression);
#endif /* EVAL_H */
