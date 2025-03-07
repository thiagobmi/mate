#ifndef NODE_H
#define NODE_H

#include <stdbool.h>

enum node_type
{
    NUMBER_NODE,
    OPERATOR_NODE,
    IDENTIFIER_NODE,
    UNARY_OPERATOR_NODE,
    FUNCTION_NODE
};

typedef struct node
{
    char *op;
    double value;
    char *name;
    enum node_type type;
    struct node *left, *right;
    struct parsed_function_info *function;
} node;

struct parsed_function_info
{

    char *name;
    node **args;
    int num_args;
};

node *new_number_node(double v);
node *new_function_node(struct parsed_function_info *f);
node *new_identifier_node(char *s);
node *new_op_node(char *op, node *l, node *r);
node *new_unary_op_node(char *op, node *l);

void free_tree(node *root);

void printAST(node *root, int level);
void pretty_print_AST(node *root);

#endif /* NODE_H */
