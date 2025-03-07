#ifndef EVAL_H
#define EVAL_H
#include "evaluator.h"

void mate_cleanup();
dictionary *get_default_dictionary();
eval_result eval(const char *expression);
#endif /* NODE_H */
