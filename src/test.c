#include "../include/tokenizer.h"
#include "../include/parser.h"
#include <assert.h>
#include <stdio.h>

void test_expression(const char *s, int expected)
{

    token_vec *v = tokenize(s);
    assert(v != NULL);
    int valid = valid_expression(v).is_valid;

    //printf("[%s]: %s\n", valid ? "VALID" : "INVALID", s);

    assert(valid == expected);
    free_token_vec(v);
}

void run_tests()
{
    test_expression("(-237 + -3)", 1);
    test_expression("5 + x", 1);
    test_expression("(a + b) * c", 1);
    test_expression("y = 3 * (x + 2)", 1);
    test_expression("x / (y - 5) + 7", 1);
    test_expression("-x / (-y - 5) + vari_av - el", 1);
    test_expression("--x / (y - 5) + 7", 1);
    test_expression("a = pow((a+b)/(4*17),a,7)", 1);
    test_expression("pow(a,b,p) = a^p + a", 1);
    test_expression("10 + pow(2+5,a/7+12)*pow(2+5,a/7+12)+4", 1);

    test_expression("5 + ", 0); // Operator without operand
    test_expression("-*x / (y - 5) + 7", 0);
    test_expression("(2 + 3", 0);  // Unbalanced parentheses
    test_expression("a b + 3", 0); // Misplaced variable
    test_expression("*/2", 0);     // Invalid operator sequence
    test_expression("", 0);
}

