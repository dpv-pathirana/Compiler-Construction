
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_logger.h"

typedef struct ErrorNode
{
    char *message;
    int line_number;
    struct ErrorNode *next;
} ErrorNode;

static ErrorNode *error_list_head = NULL;
static ErrorNode *error_list_tail = NULL;
static int semantic_error_count = 0;

void log_semantic_error(const char *message, int line)
{
    semantic_error_count++;

    ErrorNode *new_error = (ErrorNode *)malloc(sizeof(ErrorNode));
    new_error->message = strdup(message);
    new_error->line_number = line;
    new_error->next = NULL;

    if (error_list_head == NULL)
    {

        error_list_head = new_error;
        error_list_tail = new_error;
    }
    else
    {

        error_list_tail->next = new_error;
        error_list_tail = new_error;
    }
}

int print_errors_to_file(const char *filename)
{
    if (semantic_error_count == 0)
    {

        return 0;
    }

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open error file %s\n", filename);
        return 1;
    }

    ErrorNode *current = error_list_head;
    while (current != NULL)
    {

        fprintf(file, "Error at line %d: %s\n",
                current->line_number, current->message);

        ErrorNode *to_free = current;
        current = current->next;

        free(to_free->message);
        free(to_free);
    }

    fclose(file);
    printf("Semantic errors written to %s\n", filename);
    return 0;
}

int get_semantic_error_count()
{
    return semantic_error_count;
}