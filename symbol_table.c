#include "symbol_table.h"
#include "error_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_scope_recursive(FILE *file, Scope *scope, int indent_level);
static void free_scope_recursive(Scope *scope);

static Scope *create_scope(Scope *parent, char *scope_name)
{
    Scope *scope = (Scope *)malloc(sizeof(Scope));
    scope->head = NULL;
    scope->parent = parent;
    scope->scope_name = strdup(scope_name);

    scope->children = NULL;
    scope->next_sibling = NULL;

    if (parent != NULL)
    {

        scope->next_sibling = parent->children;
        parent->children = scope;
    }

    return scope;
}

SymbolTable *create_symbol_table()
{
    SymbolTable *st = (SymbolTable *)malloc(sizeof(SymbolTable));
    st->global_scope = create_scope(NULL, "global");
    st->current_scope = st->global_scope;
    return st;
}

void enter_scope(SymbolTable *st, char *scope_name)
{

    Scope *new_scope = create_scope(st->current_scope, scope_name);
    st->current_scope = new_scope;
}

void exit_scope(SymbolTable *st)
{

    if (st->current_scope != st->global_scope)
    {
        st->current_scope = st->current_scope->parent;
    }
}

void insert_symbol(SymbolTable *st, const char *name, const char *type,
                   SymbolKind kind, int line, struct ASTNode *params)
{

    if (lookup_current_scope(st, name) != NULL)
    {
        char buffer[256];
        sprintf(buffer, "Symbol '%s' already declared in this scope", name);
        log_semantic_error(buffer, line);
        return;
    }

    SymbolEntry *new_entry = (SymbolEntry *)malloc(sizeof(SymbolEntry));
    new_entry->name = strdup(name);
    new_entry->type = strdup(type);
    new_entry->kind = kind;
    new_entry->line_number = line;
    new_entry->params = params;

    new_entry->next = st->current_scope->head;
    st->current_scope->head = new_entry;
}

SymbolEntry *lookup_current_scope(SymbolTable *st, const char *name)
{
    SymbolEntry *current = st->current_scope->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

SymbolEntry *lookup_all_scopes(SymbolTable *st, const char *name)
{
    Scope *scope = st->current_scope;
    while (scope != NULL)
    {
        SymbolEntry *entry = scope->head;
        while (entry != NULL)
        {
            if (strcmp(entry->name, name) == 0)
            {
                return entry;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

static const char *kind_to_string(SymbolKind kind)
{
    switch (kind)
    {
    case KIND_VAR:
        return "Variable";
    case KIND_PARAM:
        return "Parameter";
    case KIND_FUNCTION:
        return "Function";
    case KIND_CLASS:
        return "Class";
    case KIND_ATTRIBUTE:
        return "Attribute";
    default:
        return "Unknown";
    }
}

#define WIDTH_NAME 18
#define WIDTH_TYPE 13
#define WIDTH_KIND 13
#define WIDTH_LINE 5
#define WIDTH_OTHER 36

static void print_scope(FILE *file, Scope *scope, int indent_level)
{
    char indent[40] = {0};
    for (int i = 0; i < indent_level * 2; i++)
    {
        if (i < 39)
            indent[i] = ' ';
    }

    static const char *dashes = "----------------------------------------------------------------------------------------------------";

    int inner_width = WIDTH_NAME + WIDTH_TYPE + WIDTH_KIND + WIDTH_OTHER + 3;

    char scope_title[100];
    snprintf(scope_title, sizeof(scope_title), " Scope: %s", scope->scope_name);
    fprintf(file, "%s|%-*s|\n", indent, inner_width, scope_title);

    fprintf(file, "%s+", indent);
    fprintf(file, "%.*s", WIDTH_NAME, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_TYPE, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_KIND, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_OTHER, dashes);
    fprintf(file, "+\n");

    fprintf(file, "%s|%-*s|%-*s|%-*s|%-*s|\n",
            indent,
            WIDTH_NAME, " Name",
            WIDTH_TYPE, " Type",
            WIDTH_KIND, " Kind",
            WIDTH_OTHER, " Other");

    fprintf(file, "%s+", indent);
    fprintf(file, "%.*s", WIDTH_NAME, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_TYPE, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_KIND, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_OTHER, dashes);
    fprintf(file, "+\n");

    SymbolEntry *entry = scope->head;
    if (entry == NULL)
    {

        fprintf(file, "%s|%-*s|%-*s|%-*s|%-*s|\n",
                indent,
                WIDTH_NAME, " (empty scope)",
                WIDTH_TYPE, "",
                WIDTH_KIND, "",
                WIDTH_OTHER, "");
    }

    while (entry)
    {
        char f_name[WIDTH_NAME + 5];
        char f_type[WIDTH_TYPE + 5];
        char f_kind[WIDTH_KIND + 5];
        char f_other[WIDTH_OTHER + 5];

        char other_info_content[WIDTH_OTHER + 1] = "";
        if (entry->kind == KIND_FUNCTION && entry->params != NULL)
        {
            strncpy(other_info_content, "(has params)", WIDTH_OTHER);
        }

        snprintf(f_name, sizeof(f_name), " %s", entry->name);
        snprintf(f_type, sizeof(f_type), " %s", entry->type);
        snprintf(f_kind, sizeof(f_kind), " %s", kind_to_string(entry->kind));
        snprintf(f_other, sizeof(f_other), " %s", other_info_content);

        fprintf(file, "%s|%-*s|%-*s|%-*s|%-*s|\n",
                indent,
                WIDTH_NAME, f_name,
                WIDTH_TYPE, f_type,
                WIDTH_KIND, f_kind,
                WIDTH_OTHER, f_other);

        entry = entry->next;
    }

    fprintf(file, "%s+", indent);
    fprintf(file, "%.*s", WIDTH_NAME, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_TYPE, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_KIND, dashes);
    fprintf(file, "+");
    fprintf(file, "%.*s", WIDTH_OTHER, dashes);
    fprintf(file, "+\n");
}

static void print_scope_recursive(FILE *file, Scope *scope, int indent_level)
{
    if (scope == NULL)
        return;

    print_scope(file, scope, indent_level);
    fprintf(file, "\n");

    print_scope_recursive(file, scope->children, indent_level + 1);

    print_scope_recursive(file, scope->next_sibling, indent_level);
}

void print_symbol_table_to_file(SymbolTable *st, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open symbol table file %s\n", filename);
        return;
    }

    const char *title = " Symbol Table ";
    int title_len = (int)strlen(title);

    int total_width = WIDTH_NAME + WIDTH_TYPE + WIDTH_KIND + WIDTH_OTHER + 5;

    int padding_total = total_width - 2 - title_len;
    int padding_left = padding_total / 2;
    int padding_right = padding_total - padding_left;

    const char *dashes = "----------------------------------------------------------------------------------------------------";

    fprintf(file, "+%.*s%s%.*s+\n\n", padding_left, dashes, title, padding_right, dashes);

    print_scope_recursive(file, st->global_scope, 0);

    fclose(file);
    printf("Symbol table written to %s\n", filename);
}

static void free_scope_data(Scope *scope)
{
    if (scope == NULL)
        return;

    SymbolEntry *entry = scope->head;
    while (entry != NULL)
    {
        SymbolEntry *temp = entry;
        entry = entry->next;
        free(temp->name);
        free(temp->type);
        free(temp);
    }
    free(scope->scope_name);
    free(scope);
}

static void free_scope_recursive(Scope *scope)
{
    if (scope == NULL)
        return;

    free_scope_recursive(scope->children);
    free_scope_recursive(scope->next_sibling);

    free_scope_data(scope);
}

void free_symbol_table(SymbolTable *st)
{
    if (st == NULL)
        return;

    free_scope_recursive(st->global_scope);

    st->global_scope = NULL;
    st->current_scope = NULL;

    free(st);
}