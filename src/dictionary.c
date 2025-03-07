#include "../include/dictionary.h"
#include "../include/node.h"
#include "../include/parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

dictionary *new_dict(int max_size)
{
    dictionary *d = malloc(sizeof(dictionary));
    d->entries = malloc(sizeof(entry) * max_size);
    d->max_size = max_size;
    d->len = 0;

    return d;
}
dictionary *add_function(dictionary *d, entry *et)
{

    int index = d->len;

    if (d->len >= d->max_size - 1)
    {
        d = resize_dict(d, d->max_size * 2);
    }

    int id = search_dict(d, et->key);

    if (id != -1)
    {
        free_entry(d->entries[id]);
        index = id;
    }

    d->entries[index].key = et->key;
    d->entries[index].type = FUNCTION;
    d->entries[index].function = et->function;

    if (id == -1)
        d->len++;

    return d;
}

dictionary *resize_dict(dictionary *d, int new_size)
{
    assert(new_size > d->max_size);
    d->entries = realloc(d->entries, (new_size * sizeof(entry)));
    d->max_size = new_size;

    return d;
}

dictionary *add_entry(dictionary *d, char *k, double v)
{

    int index = d->len;
    if (d->len >= d->max_size - 1)
    {
        d = resize_dict(d, d->max_size * 2);
    }

    int id = search_dict(d, k);
    if (id != -1)
    {
        free_entry(d->entries[id]);
        index = id;
    }

    d->entries[index].type = VARIABLE;
    d->entries[index].key = k;
    d->entries[index].value = v;

    if (id == -1)
        d->len++;
    return d;
}

void free_entry(entry et)
{

    free(et.key);
    if (et.type == FUNCTION)
    {
        struct stored_function_info *fn = et.function;
        free_dict(fn->args);
        free_tree(fn->ast);
        free(fn);
    }
}

void free_dict(dictionary *d)
{
    for (int i = 0; i < d->len; i++)
    {
        free_entry(d->entries[i]);
    }

    free(d->entries);
    free(d);
}

int search_dict(dictionary *d, char *s)
{
    for (int i = 0; i < d->len; i++)
    {
        if (strcmp(s, d->entries[i].key) == 0)
        {
            return i;
        }
    }
    return -1;
}
