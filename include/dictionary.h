#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "node.h"

typedef enum entry_type
{
    VARIABLE,
    FUNCTION
} entry_type;

typedef struct entry
{
    char *key;
    double value;
    entry_type type;
    struct stored_function_info *function;

} entry;

typedef struct dict
{
    int max_size;
    int len;
    entry *entries;
} dictionary;

void free_entry(entry et);
dictionary *add_function(dictionary *d, entry *et);
dictionary *new_dict(int max_size);
dictionary *resize_dict(dictionary *d, int new_size);
dictionary *add_entry(dictionary *d, char *k, double v);
void free_dict(dictionary *d);
int search_dict(dictionary *d, char *s);

#endif /* DICTIONARY_H */
