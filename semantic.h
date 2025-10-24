
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

void build_symbol_table_pass(struct ASTNode *node, SymbolTable *st);

void type_check_pass(struct ASTNode *node, SymbolTable *st);

#endif