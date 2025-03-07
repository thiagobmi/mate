#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdbool.h>
#define MAX_INPUT_SIZE 512
#define BUFFER_SIZE 512
#define ESC '\x1b'
#define BACKSPACE 127

typedef struct
{
    char *op;
    int precedence;
    bool right_associative;
    bool is_unary;
} operator_info;

extern operator_info operators[];
extern const int NUM_OPERATORS;

#endif /* CONSTANTS_H */
