#include "semantic.h"
#include "error_logger.h"
#include "tokens.h"
#include <stdio.h>
#include <string.h>

static char *get_expression_type(struct ASTNode *node, SymbolTable *st);

void build_symbol_table_pass(struct ASTNode *node, SymbolTable *st)
{
    if (node == NULL)
        return;

    switch (node->type)
    {

    case NODE_FUNC_DEF:
    {
        struct FuncDefNode *func_def = (struct FuncDefNode *)node;
        struct FuncHeadNode *head = (struct FuncHeadNode *)func_def->func_head;

        char *func_type;
        if (head->return_type)
        {
            func_type = ((struct IdentifierNode *)head->return_type)->name;
        }
        else
        {
            func_type = "constructor";
        }

        insert_symbol(st, head->id, func_type, KIND_FUNCTION, head->line_number, head->params);

        enter_scope(st, head->id);
        node->scope = st->current_scope;

        build_symbol_table_pass(head->params, st);
        build_symbol_table_pass(func_def->func_body, st);

        exit_scope(st);
        break;
    }

    case NODE_VAR_DECL:
    {
        struct VarDeclNode *var_decl = (struct VarDeclNode *)node;
        char *type_name = ((struct IdentifierNode *)var_decl->type_node)->name;

        insert_symbol(st, var_decl->id, type_name, KIND_VAR, var_decl->line_number, NULL);

        build_symbol_table_pass(var_decl->array_dims, st);
        break;
    }

    case NODE_STAT_BLOCK:
    {
        enter_scope(st, "stat_block");
        node->scope = st->current_scope;

        build_symbol_table_pass(((struct GenericNode *)node)->child1, st);

        exit_scope(st);
        break;
    }

    case NODE_FUNC_BODY:
    {

        build_symbol_table_pass(((struct GenericNode *)node)->child1, st);
        break;
    }

    case NODE_PROG:
    case NODE_STATEMENT_LIST:
    case NODE_PARAM_LIST:
        build_symbol_table_pass(((struct GenericNode *)node)->child1, st);
        build_symbol_table_pass(((struct GenericNode *)node)->child2, st);
        break;

    case NODE_IF_STMT:
        build_symbol_table_pass(((struct IfNode *)node)->condition, st);
        build_symbol_table_pass(((struct IfNode *)node)->if_body, st);
        build_symbol_table_pass(((struct IfNode *)node)->else_body, st);
        break;

    case NODE_WHILE_STMT:
        build_symbol_table_pass(((struct WhileNode *)node)->condition, st);
        build_symbol_table_pass(((struct WhileNode *)node)->while_body, st);
        break;

    case NODE_ASSIGN_STMT:
        build_symbol_table_pass(((struct AssignNode *)node)->variable, st);
        build_symbol_table_pass(((struct AssignNode *)node)->expression, st);
        break;

    default:
        break;
    }

    build_symbol_table_pass(node->next, st);
}

static char *type_check_function_call(struct ASTNode *node, SymbolTable *st)
{
    struct FuncCallNode *func_call = (struct FuncCallNode *)node;

    SymbolEntry *func_symbol = lookup_all_scopes(st, func_call->id);

    if (func_symbol == NULL)
    {
        char buffer[256];
        sprintf(buffer, "Undeclared function '%s'", func_call->id);
        log_semantic_error(buffer, node->line_number);
        return "error_type";
    }

    if (func_symbol->kind != KIND_FUNCTION)
    {
        char buffer[256];
        sprintf(buffer, "'%s' is not a function", func_call->id);
        log_semantic_error(buffer, node->line_number);
        return "error_type";
    }

    struct ASTNode *current_arg_node = func_call->args;

    struct ASTNode *current_param_node = func_symbol->params;

    while (current_arg_node != NULL && current_param_node != NULL)
    {

        struct ASTNode *arg_expr = current_arg_node;
        char *arg_type = get_expression_type(arg_expr, st);

        struct VarDeclNode *param_decl = (struct VarDeclNode *)current_param_node;
        char *param_type = ((struct IdentifierNode *)param_decl->type_node)->name;

        if (strcmp(arg_type, "error_type") != 0 && strcmp(arg_type, param_type) != 0)
        {

            if (strcmp(param_type, "float") == 0 && strcmp(arg_type, "integer") == 0)
            {
            }
            else
            {
                char buffer[256];
                sprintf(buffer, "Type mismatch in function call '%s': expected '%s' but got '%s'",
                        func_call->id, param_type, arg_type);
                log_semantic_error(buffer, arg_expr->line_number);
            }
        }

        type_check_pass(arg_expr, st);

        current_arg_node = current_arg_node->next;
        current_param_node = current_param_node->next;
    }

    if (current_arg_node != NULL)
    {
        log_semantic_error("Too many arguments to function", node->line_number);
    }
    if (current_param_node != NULL)
    {
        log_semantic_error("Too few arguments to function", node->line_number);
    }

    return func_symbol->type;
}

static char *get_expression_type(struct ASTNode *node, SymbolTable *st)
{
    if (node == NULL)
        return "void";

    switch (node->type)
    {
    case NODE_INT_LIT:
        return "integer";

    case NODE_FLOAT_LIT:
        return "float";

    case NODE_STRING_LIT:
        return "string";

    case NODE_ID:
    {
        char *var_name = ((struct IdentifierNode *)node)->name;
        SymbolEntry *symbol = lookup_all_scopes(st, var_name);

        if (symbol == NULL)
        {
            char buffer[256];
            sprintf(buffer, "Undeclared variable '%s'", var_name);
            log_semantic_error(buffer, node->line_number);
            return "error_type";
        }
        return symbol->type;
    }

    case NODE_VARIABLE:
    {
        struct VarAccessNode *var_node = (struct VarAccessNode *)node;
        char *base_type = get_expression_type(var_node->base, st);

        if (var_node->indices != NULL)
        {
            struct ASTNode *currentIndex = var_node->indices;
            while (currentIndex != NULL)
            {

                char *index_type = get_expression_type(currentIndex, st);

                if (strcmp(index_type, "error_type") != 0 &&
                    strcmp(index_type, "integer") != 0)
                {
                    char buffer[256];
                    sprintf(buffer, "Array index must be an integer, but got '%s'", index_type);
                    log_semantic_error(buffer, currentIndex->line_number);

                    return "error_type";
                }
                currentIndex = currentIndex->next;
            }
        }
        return base_type;
    }

    case NODE_BIN_OP:
    {
        struct BinOpNode *bin_op = (struct BinOpNode *)node;
        char *left_type = get_expression_type(bin_op->left, st);
        char *right_type = get_expression_type(bin_op->right, st);

        if (strcmp(left_type, "error_type") == 0 ||
            strcmp(right_type, "error_type") == 0)
        {
            return "error_type";
        }

        switch (bin_op->op)
        {
        case PLUS_OP:
        case MINUS_OP:
        case MULT_OP:
        case DIV_OP:
            if ((strcmp(left_type, "integer") != 0 && strcmp(left_type, "float") != 0) ||
                (strcmp(right_type, "integer") != 0 && strcmp(right_type, "float") != 0))
            {
                log_semantic_error("Operands for arithmetic op must be numeric", node->line_number);
                return "error_type";
            }
            if (strcmp(left_type, "float") == 0 || strcmp(right_type, "float") == 0)
            {
                return "float";
            }
            return "integer";

        case EQ_OP:
        case NE_OP:
        case LT_OP:
        case GT_OP:
        case LE_OP:
        case GE_OP:
            if (strcmp(left_type, right_type) != 0)
            {
                if (!((strcmp(left_type, "integer") == 0 && strcmp(right_type, "float") == 0) ||
                      (strcmp(left_type, "float") == 0 && strcmp(right_type, "integer") == 0)))
                {
                    log_semantic_error("Incompatible types for comparison", node->line_number);
                }
            }
            return "boolean";

        case KEYWORD:
            if (strcmp(current_lexeme, "and") == 0 || strcmp(current_lexeme, "or") == 0)
            {
                if (strcmp(left_type, "boolean") != 0 || strcmp(right_type, "boolean") != 0)
                {
                    log_semantic_error("Operands for logical op must be boolean", node->line_number);
                    return "error_type";
                }
                return "boolean";
            }
            break;
        }
        break;
    }
    case NODE_FUNC_CALL:
    {
        return type_check_function_call(node, st);
    }
    }
    return "error_type";
}

void type_check_pass(struct ASTNode *node, SymbolTable *st)
{
    if (node == NULL)
        return;

    if (node->scope != NULL)
    {
        st->current_scope = node->scope;
    }

    switch (node->type)
    {
    case NODE_ASSIGN_STMT:
    {
        struct AssignNode *assign = (struct AssignNode *)node;
        char *lhs_type = get_expression_type(assign->variable, st);
        char *rhs_type = get_expression_type(assign->expression, st);

        if (strcmp(lhs_type, "error_type") != 0 && strcmp(rhs_type, "error_type") != 0)
        {

            if (strcmp(lhs_type, rhs_type) != 0)
            {
                if (strcmp(lhs_type, "float") == 0 && strcmp(rhs_type, "integer") == 0)
                {
                }
                else
                {
                    char buffer[256];
                    sprintf(buffer, "Type mismatch: cannot assign type '%s' to variable of type '%s'", rhs_type, lhs_type);
                    log_semantic_error(buffer, node->line_number);
                }
            }
        }
        break;
    }
    case NODE_IF_STMT:
    case NODE_WHILE_STMT:
    {
        struct ASTNode *condition;
        if (node->type == NODE_IF_STMT)
        {
            condition = ((struct IfNode *)node)->condition;
        }
        else
        {
            condition = ((struct WhileNode *)node)->condition;
        }

        char *cond_type = get_expression_type(condition, st);
        if (strcmp(cond_type, "error_type") != 0 &&
            strcmp(cond_type, "boolean") != 0)
        {
            log_semantic_error("Condition expression must be of type boolean", condition->line_number);
        }

        if (node->type == NODE_IF_STMT)
        {
            type_check_pass(((struct IfNode *)node)->if_body, st);
            type_check_pass(((struct IfNode *)node)->else_body, st);
        }
        else
        {
            type_check_pass(((struct WhileNode *)node)->while_body, st);
        }
        break;
    }

    case NODE_PROG:
    case NODE_FUNC_BODY:
    case NODE_STATEMENT_LIST:
    case NODE_STAT_BLOCK:

        type_check_pass(((struct GenericNode *)node)->child1, st);
        type_check_pass(((struct GenericNode *)node)->child2, st);
        break;

    case NODE_FUNC_DEF:
    {

        Scope *old_scope = st->current_scope;
        st->current_scope = node->scope;

        type_check_pass(((struct FuncDefNode *)node)->func_body, st);

        st->current_scope = old_scope;
        break;
    }

    case NODE_WRITE_STMT:

        type_check_pass(((struct GenericNode *)node)->child1, st);
        break;

    case NODE_RETURN_STMT:
    {
        struct ASTNode *return_expr = ((struct GenericNode *)node)->child1;

        type_check_pass(return_expr, st);

        char *actual_return_type = "void";
        if (return_expr != NULL)
        {
            actual_return_type = get_expression_type(return_expr, st);
        }

        SymbolEntry *func_symbol = lookup_all_scopes(st, st->current_scope->scope_name);

        if (func_symbol == NULL)
        {
            log_semantic_error("Compiler Bug: Cannot find symbol for current function", node->line_number);
            break;
        }
        char *expected_return_type = func_symbol->type;

        if (strcmp(actual_return_type, "error_type") != 0 &&
            strcmp(expected_return_type, actual_return_type) != 0)
        {
            if (strcmp(expected_return_type, "float") == 0 && strcmp(actual_return_type, "integer") == 0)
            {
            }
            else
            {

                char buffer[256];
                sprintf(buffer, "Return type mismatch: function expects '%s' but returns '%s'",
                        expected_return_type, actual_return_type);
                log_semantic_error(buffer, node->line_number);
            }
        }
        break;
    }

    case NODE_VAR_DECL:

        break;

    case NODE_FUNC_CALL:
    {
        type_check_function_call(node, st);
        break;
    }

    default:
        break;
    }

    type_check_pass(node->next, st);

    if (node->scope != NULL)
    {
        exit_scope(st);
    }
}