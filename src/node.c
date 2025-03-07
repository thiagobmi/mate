#include "../include/node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_tree(node *root)
{
    if (root == NULL)
        return;

    free_tree(root->left);
    free_tree(root->right);

    if ((root->type == OPERATOR_NODE || root->type == UNARY_OPERATOR_NODE) &&
        root->op != NULL)
        free(root->op);
    if (root->type == IDENTIFIER_NODE && root->name != NULL)
        free(root->name);
    if (root->type == FUNCTION_NODE && root->function != NULL)
    {
        struct parsed_function_info *fn = root->function;
        free(fn->name);
        for (int i = 0; i < fn->num_args; i++)
            free_tree(fn->args[i]);
        free(fn->args);
        free(fn);
    }

    free(root);
}

node *new_unary_op_node(char *op, node *l)
{
    node *n = malloc(sizeof(node));

    if (n == NULL)
        return NULL;

    n->type = UNARY_OPERATOR_NODE;
    n->op = op;
    n->name = NULL;
    n->left = l;
    n->right = NULL;

    return n;
}

node *new_op_node(char *op, node *l, node *r)
{

    node *n = malloc(sizeof(node));

    if (n == NULL)
        return NULL;

    n->type = OPERATOR_NODE;
    n->op = op;
    n->name = NULL;
    n->left = l;
    n->right = r;

    return n;
}

node *new_function_node(struct parsed_function_info *f)
{

    node *n = malloc(sizeof(node));

    if (n == NULL)
        return NULL;

    n->type = FUNCTION_NODE;
    n->function = f;
    n->op = NULL;
    n->name = NULL;
    n->left = NULL;
    n->right = NULL;

    return n;
}

node *new_number_node(double v)
{
    node *n = malloc(sizeof(node));

    if (n == NULL)
        return NULL;

    n->type = NUMBER_NODE;
    n->value = v;
    n->op = NULL;
    n->name = NULL;
    n->left = NULL;
    n->right = NULL;

    return n;
}

node *new_identifier_node(char *s)
{
    node *n = malloc(sizeof(node));

    if (n == NULL)
        return NULL;

    n->type = IDENTIFIER_NODE;
    n->name = s;
    n->op = NULL;
    n->left = NULL;
    n->right = NULL;

    return n;
}

void printAST(node *root, int level)
{
    if (root == NULL)
        return;

    for (int i = 0; i < level; i++)
    {
        printf("│   ");
    }

    if (level > 0)
    {
        printf("├── ");
    }

    if (root->type == NUMBER_NODE)
    {
        printf("%.2f\n", root->value);
    }
    else if (root->type == OPERATOR_NODE)
    {
        printf("%s\n", root->op);
    }
    else if (root->type == IDENTIFIER_NODE)
    {
        printf("%s\n", root->name);
    }
    else if (root->type == FUNCTION_NODE)
    {
        printf("%s()\n", root->function->name);
    }

    printAST(root->left, level + 1);
    printAST(root->right, level + 1);
}

void pretty_print_AST(node *root)
{

    printf("\n");
    printAST(root, 0);
    printf("\n");
}
