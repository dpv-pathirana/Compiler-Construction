
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

typedef enum
{
    KIND_VAR,
    KIND_PARAM,
    KIND_FUNCTION,
    KIND_CLASS,
    KIND_ATTRIBUTE
} SymbolKind;

typedef struct SymbolEntry
{
    char *name;
    char *type;
    SymbolKind kind;
    int line_number;

    struct ASTNode *params;

    struct SymbolEntry *next;
} SymbolEntry;

typedef struct Scope
{
    SymbolEntry *head;
    struct Scope *parent;
    char *scope_name;
    struct Scope *children;
    struct Scope *next_sibling;
} Scope;

typedef struct SymbolTable
{
    Scope *global_scope;
    Scope *current_scope;
} SymbolTable;

SymbolTable *create_symbol_table();

void enter_scope(SymbolTable *st, char *scope_name);

void exit_scope(SymbolTable *st);

void insert_symbol(SymbolTable *st, const char *name, const char *type,
                   SymbolKind kind, int line, struct ASTNode *params);

SymbolEntry *lookup_current_scope(SymbolTable *st, const char *name);

SymbolEntry *lookup_all_scopes(SymbolTable *st, const char *name);

void print_symbol_table_to_file(SymbolTable *st, const char *filename);

void free_symbol_table(SymbolTable *st);

#endif