#include "../include/constants.h"
#include "../include/dictionary.h"
#include "../include/eval.h"
#include "../include/evaluator.h"
#include "../include/extern.h"
#include "../include/parser.h"
#include "../include/tokenizer.h"
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 1024
struct termios orig_termios;

void disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void print_function(entry et)
{
    if (et.type == FUNCTION)
    {
        printf("\t%s(", et.key);

        struct stored_function_info *fn = et.function;
        for (int i = 0; i < et.function->num_args; i++)
        {
            printf("%s%s", fn->args->entries[i].key,
                   i == fn->num_args - 1 ? "" : ",");
        }
        printf(") = ");
        print_expression(fn->expr);
        printf("\n");
    }
    else if (et.type == EXTERN_FUNCTION)
    {
        printf("\t%s({%d}) EXTERNAL\n", et.key, et.external_function->num_args);
    }
}
void print_functions(dictionary *d)
{
    for (int i = 0; i < d->len; i++)
    {
        entry ent = d->entries[i];
        if (ent.type == FUNCTION || ent.type == EXTERN_FUNCTION)
            print_function(ent);
    }
}

void print_result(double result)
{
    char str[51];
    snprintf(str, 50, "%.15lf", result);

    int index = strlen(str) - 1;

    while (str[index] == '0')
        index--;

    if (str[index] == '.')
        index--;

    for (int i = 0; i <= index; i++)
        printf("%c", str[i]);
}

void print_variables(dictionary *d)
{
    for (int i = 0; i < d->len; i++)
    {
        entry ent = d->entries[i];
        if (ent.type == VARIABLE)
        {

            printf("\t%s = ", ent.key);
            print_result(ent.value);
            printf("\n");
        }
    }
}

void redrawLine(const char *buffer, int cursor_pos)
{
    printf("\r%% %s", buffer);
    printf("\x1b[K");

    printf("\r%% ");
    for (int i = 0; i < cursor_pos; i++)
    {
        printf("\x1b[C");
    }
    fflush(stdout);
}

char **history_alloc(int num)
{
    char **history = malloc(sizeof(char *) * num);
    for (int i = 0; i < num; i++)
        history[i] = malloc(sizeof(char) * BUFFER_SIZE);
    return history;
}

char **history_realloc(char **history, int old_num, int new_num)
{
    assert(new_num > old_num);
    history = realloc(history, sizeof(char *) * new_num);

    for (int i = old_num; i < new_num; i++)
        history[i] = malloc(sizeof(char) * BUFFER_SIZE);
    return history;
}

void free_history(char **history, int len)
{

    for (int i = 0; i < len; i++)
        free(history[i]);
    free(history);
}

int start_calculator()
{
    enableRawMode();

    char buffer[BUFFER_SIZE] = {0};
    int cursor_pos = 0;
    int buffer_len = 0;
    int history_pos = 0;
    int history_cursor = 0;
    int history_max = 1;

    char **history = history_alloc(history_max);

    printf("MATE (Mathematical Analysis Tool and Evaluator) | ESC to exit\n");
    printf("%% ");
    fflush(stdout);

    dictionary *d = get_default_dictionary();
    while (1)
    {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1)
        {
            if (c == ESC)
            {
                char seq[3];
                if (read(STDIN_FILENO, &seq[0], 1) != 1)
                    break;
                if (read(STDIN_FILENO, &seq[1], 1) != 1)
                    break;

                if (seq[0] == '[')
                {
                    switch (seq[1])
                    {
                    case 'D':
                        if (cursor_pos > 0)
                        {
                            cursor_pos--;
                            redrawLine(buffer, cursor_pos);
                        }
                        break;
                    case 'C':
                        if (cursor_pos < buffer_len)
                        {
                            cursor_pos++;
                            redrawLine(buffer, cursor_pos);
                        }
                        break;
                    case 'A':
                        if (history_cursor > 0)
                        {
                            history_cursor--;
                            strcpy(buffer, history[history_cursor]);
                            cursor_pos = strlen(buffer);
                            buffer_len = cursor_pos;
                            redrawLine(buffer, cursor_pos);
                        }
                        break;
                    case 'B':
                        if (history_cursor < history_pos - 1)
                        {
                            history_cursor++;
                            strcpy(buffer, history[history_cursor]);
                            cursor_pos = strlen(buffer);
                            buffer_len = cursor_pos;
                            redrawLine(buffer, cursor_pos);
                        }
                        else if (history_cursor == history_pos - 1)
                        {
                            buffer[0] = '\0';
                            cursor_pos = 0;
                            redrawLine(buffer, cursor_pos);
                            history_cursor++;
                        }
                        break;

                    case 'H': // Home key (ESC [ H)
                    case '1': // ESC [ 1 ~ (Alternative Home key)
                        if (read(STDIN_FILENO, &seq[2], 1) == 1 &&
                            seq[2] == '~')
                        {
                            cursor_pos = 0;
                            redrawLine(buffer, cursor_pos);
                        }
                        else
                        {
                            cursor_pos = 0;
                            redrawLine(buffer, cursor_pos);
                        }
                        break;
                    case 'F': // End key (ESC [ F)
                    case '4': // ESC [ 4 ~ (Alternative End key)
                        if (read(STDIN_FILENO, &seq[2], 1) == 1 &&
                            seq[2] == '~')
                        {
                            cursor_pos = buffer_len;
                            redrawLine(buffer, cursor_pos);
                        }
                        else
                        {
                            cursor_pos = buffer_len;
                            redrawLine(buffer, cursor_pos);
                        }
                        break;
                    }
                }
                else
                {
                    printf("\n Exiting...\n");
                    free_history(history, history_max);
                    free_dict(d);
                    return 0;
                }
            }
            else if (c == BACKSPACE || c == 8)
            {
                if (cursor_pos > 0)
                {
                    memmove(&buffer[cursor_pos - 1], &buffer[cursor_pos],
                            buffer_len - cursor_pos + 1);
                    cursor_pos--;
                    buffer_len--;
                    redrawLine(buffer, cursor_pos);
                }
            }
            else if (c == '\r' || c == '\n')
            {
                buffer[buffer_len] = '\0';

                if (strcmp(buffer, "exit") == 0)
                {
                    free_history(history, history_max);
                    free_dict(d);
                    return 0;
                }
                if (strlen(buffer) == 0)
                {
                    printf("\n");
                    redrawLine(buffer, cursor_pos);
                    continue;
                }

                if (history_pos >= history_max - 1)
                {
                    history =
                        history_realloc(history, history_max, history_max * 2);
                    history_max *= 2;
                }

                if (history_pos == 0 ||
                    (history_pos > 0 &&
                     strcmp(buffer, history[history_pos - 1]) != 0))
                {
                    strcpy(history[history_pos++], buffer);
                    history_cursor = history_pos;
                }

                if (strcmp(buffer, "functions") == 0)
                {
                    printf("\n");
                    print_functions(d);
                    buffer[0] = '\0';
                    cursor_pos = 0;
                    redrawLine(buffer, cursor_pos);
                    continue;
                }
                else if (strcmp(buffer, "variables") == 0)
                {
                    printf("\n");
                    print_variables(d);
                    buffer[0] = '\0';
                    cursor_pos = 0;
                    redrawLine(buffer, cursor_pos);
                    continue;
                }
                else if (strncmp(buffer, "showtree", 8) == 0)
                {
                    char *token = strtok(buffer, " ");
                    token = strtok(NULL, " ");

                    if (token != NULL)
                    {
                        int id = search_dict(d, token);
                        if (id != -1 && d->entries[id].type == FUNCTION)
                        {
                            printf("\n");
                            pretty_print_AST(d->entries[id].function->ast);
                        }
                        else
                        {
                            printf(
                                " Error: %s not found or is not a function\n",
                                token);
                        }
                        memset(buffer, 0, BUFFER_SIZE);
                        cursor_pos = 0;
                        buffer_len = 0;
                        redrawLine(buffer, cursor_pos);
                        continue;
                    }
                    else
                    {
                        memset(buffer, 0, BUFFER_SIZE);
                        cursor_pos = 0;
                        buffer_len = 0;
                        redrawLine(buffer, cursor_pos);
                        printf("\n Missing second word.\n");
                        continue;
                    }
                }

                token_vec *v = tokenize(buffer);
                expression_info e = valid_expression(v);

                if (!e.is_valid)
                {
                    printf("\n Error: Invalid expression\n");
                    fflush(stdout);
                    free_token_vec(v);
                    memset(buffer, 0, BUFFER_SIZE);
                    cursor_pos = 0;
                    buffer_len = 0;
                    redrawLine(buffer, cursor_pos);
                    continue;
                }
                else if (e.is_function_declaration)
                {
                    entry *et = parse_function_declaration(v);
                    d = dictionary_add_function(d, et);
                    free(et);
                    memset(buffer, 0, BUFFER_SIZE);
                    cursor_pos = 0;
                    buffer_len = 0;
                    printf("\n%% ");
                    fflush(stdout);
                    free_token_vec(v);
                    continue;
                }

                node *root = compute_expression(v);

                if (root == NULL)
                {
                    printf(" Error: Failed to parse expression\n");
                }
                else
                {
                    eval_result result = evaluate_expression(root, d);

                    if (strlen(result.error_msg) > 0)
                    {
                        printf("\n Error: %s\n", result.error_msg);
                    }
                    else if (result.is_assignment)
                    {
                        printf("\n");
                    }
                    else
                    {
                        printf("\n ");
                        print_result(result.value);
                        printf("\n");
                    }
                    free_tree(root);
                }

                free_token_vec(v);

                memset(buffer, 0, BUFFER_SIZE);
                cursor_pos = 0;
                buffer_len = 0;
                printf("%% ");
                fflush(stdout);
            }
            else if (isprint(c))
            {
                if (buffer_len < BUFFER_SIZE - 1)
                {
                    memmove(&buffer[cursor_pos + 1], &buffer[cursor_pos],
                            buffer_len - cursor_pos + 1);
                    buffer[cursor_pos] = c;
                    cursor_pos++;
                    buffer_len++;
                    redrawLine(buffer, cursor_pos);
                }
            }
        }
    }

    free_history(history, history_max);
    free_dict(d);
    return 0;
}
char *trimwhitespace(char *str)
{
    char *end;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    end[1] = '\0';

    return str;
}

int count_tokens(const char *str, const char *delim)
{
    if (str == NULL)
        return 0;

    int count = 0;

    while (*str)
    {
        str += strspn(str, delim);

        if (*str)
        {
            count++;
            str += strcspn(str, delim);
        }
    }

    return count;
}

bool load_mateconfig(char *filename)
{
    FILE *file;
    char line[MAX_LINE_LENGTH];

    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file %s\n", filename);
        return 1;
    }

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        eval_result ex = eval(line);

        if (ex.error)
        {
            // printf("%s\n",line);
            /*printf("Error loading %s\n",filename);*/
        }
    }

    fclose(file);
    return 0;
}

void print_usage(char *program_name)
{
    printf("\n");
    printf("MATE (Mathematical Analysis Tool and Evaluator)\n");
    printf("Uso: %s [opções] [expressão]\n", program_name);
    printf("     %s [opções]\n", program_name);
    printf("Opções:\n");
    printf("  -c, --config ARQUIVO    Usa ARQUIVO como configuração (padrão: "
           ".mateconfig)\n");
    printf("  -s, --simple-print      Força o modo de impressão simplificada "
           "para todas as expressões\n");
    printf("  -h, --help              Mostra esta mensagem de ajuda\n");
    printf("\n");
}

int main(int argc, char **argv)
{
    // run_tests();

    char *token;
    char *config_file = ".mateconfig";
    bool force_simple_output = false;
    int c;

    static struct option long_options[] = {
        {"config", required_argument, 0, 'c'},
        {"simple-output", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "c:sh", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'c':
            config_file = optarg;
            break;
        case 's':
            force_simple_output = true;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        case '?':
            return 1;
        default:
            abort();
        }
    }

    add_extern_functions();
    load_mateconfig(config_file);

    if (optind < argc)
    {
        bool simple_output =
            force_simple_output || (count_tokens(argv[optind], ";") < 2);
        token = strtok(argv[optind], ";");
        do
        {
            eval_result ex = eval(token);

            if (ex.error)
            {
                if (strlen(ex.error_msg) > 0)
                {
                    printf("%s\n", ex.error_msg);
                }
                else
                {
                    printf("\nError at: %d\n", ex.error_at);
                }
                mate_cleanup();
                return 1;
            }
            else if (!ex.is_assignment)
            {
                if (!simple_output)
                {
                    printf("%-3s", "");
                    char *trimmed = trimwhitespace(token);
                    printf("%s ", trimmed);
                    int padding = 10 - strlen(trimmed);
                    if (padding < 0)
                        padding = 1;
                    for (int i = 0; i < padding; i++)
                    {
                        printf(" ");
                    }
                    printf("= ");
                    print_result(ex.value);
                    printf("\n");
                }
                else
                {
                    printf("%-5s", "");
                    print_result(ex.value);
                    printf("\n");
                }
            }
        } while ((token = strtok(NULL, ";")));
        mate_cleanup();
    }
    else
    {
        start_calculator();
    }

    return 0;
}
