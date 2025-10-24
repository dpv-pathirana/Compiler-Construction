
#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>
#include "tokens.h"

extern int yylineno;
extern char current_lexeme[];

typedef enum
{
    NODE_PROG,
    NODE_CLASS_LIST,
    NODE_CLASS_DECL,
    NODE_IMPL_DEF,
    NODE_FUNC_DEF,
    NODE_MEMBER_LIST,
    NODE_VAR_DECL,
    NODE_ATTRIBUTE_DECL,
    NODE_FUNC_HEAD,
    NODE_TYPE,
    NODE_RETURN_TYPE,
    NODE_FUNC_BODY,
    NODE_STATEMENT_LIST,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_READ_STMT,
    NODE_WRITE_STMT,
    NODE_RETURN_STMT,
    NODE_ASSIGN_STMT,
    NODE_STAT_BLOCK,
    NODE_EXPR,
    NODE_BIN_OP,
    NODE_UNARY_OP,
    NODE_VARIABLE,
    NODE_FUNC_CALL,
    NODE_INT_LIT,
    NODE_FLOAT_LIT,
    NODE_STRING_LIT,
    NODE_PARAM_LIST,
    NODE_ARG_LIST,
    NODE_FUNC_DECL,
    NODE_PUBLIC,
    NODE_PRIVATE,
    NODE_ID,
    NODE_VISIBILITY,
    NODE_OP
} NodeType;

struct ASTNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;
};

struct GenericNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    struct ASTNode *child1;
    struct ASTNode *child2;
    struct ASTNode *child3;
    struct ASTNode *child4;
};

struct LiteralNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    union
    {
        int int_value;
        float float_value;
        char *string_value;
    } value;
};

struct IdentifierNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    char *name;
};

struct BinOpNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    int op;
    struct ASTNode *left;
    struct ASTNode *right;
};

struct UnaryOpNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    int op;
    struct ASTNode *operand;
};

struct VarDeclNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    char *id;
    struct ASTNode *type_node;
    struct ASTNode *array_dims;
};

struct FuncHeadNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    int is_constructor;
    char *id;
    struct ASTNode *params;
    struct ASTNode *return_type;
};

struct FuncDefNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    struct ASTNode *func_head;
    struct ASTNode *func_body;
};

struct ClassDeclNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    char *id;
    struct ASTNode *isa_list;
    struct ASTNode *inheritance_list;
    struct ASTNode *members;
};

struct ImplDefNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    char *id;
    struct ASTNode *func_defs;
};

struct IfNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    struct ASTNode *condition;
    struct ASTNode *if_body;
    struct ASTNode *else_body;
};

struct WhileNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    struct ASTNode *condition;
    struct ASTNode *while_body;
};

struct AssignNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    struct ASTNode *variable;
    struct ASTNode *expression;
};

struct VarAccessNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    struct ASTNode *base;
    struct ASTNode *indices;
    struct ASTNode *members;
};

struct FuncCallNode
{
    NodeType type;
    int line_number;
    struct ASTNode *next;
    struct Scope *scope;

    char *id;
    struct ASTNode *id_nest;
    struct ASTNode *args;
};

static inline struct ASTNode *create_node(NodeType type, struct ASTNode *c1, struct ASTNode *c2)
{
    struct GenericNode *node = (struct GenericNode *)malloc(sizeof(struct GenericNode));
    node->type = type;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->child1 = c1;
    node->child2 = c2;
    node->child3 = NULL;
    node->child4 = NULL;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_id_node(char *name)
{
    struct IdentifierNode *node = (struct IdentifierNode *)malloc(sizeof(struct IdentifierNode));
    node->type = NODE_ID;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->name = strdup(name);
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_int_lit(int value)
{
    struct LiteralNode *node = (struct LiteralNode *)malloc(sizeof(struct LiteralNode));
    node->type = NODE_INT_LIT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->value.int_value = value;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_float_lit(float value)
{
    struct LiteralNode *node = (struct LiteralNode *)malloc(sizeof(struct LiteralNode));
    node->type = NODE_FLOAT_LIT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->value.float_value = value;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_string_lit(char *value)
{
    struct LiteralNode *node = (struct LiteralNode *)malloc(sizeof(struct LiteralNode));
    node->type = NODE_STRING_LIT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->value.string_value = strdup(value);
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_bin_op(int op, struct ASTNode *left, struct ASTNode *right)
{
    struct BinOpNode *node = (struct BinOpNode *)malloc(sizeof(struct BinOpNode));
    node->type = NODE_BIN_OP;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->op = op;
    node->left = left;
    node->right = right;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_unary_op(int op, struct ASTNode *operand)
{
    struct UnaryOpNode *node = (struct UnaryOpNode *)malloc(sizeof(struct UnaryOpNode));
    node->type = NODE_UNARY_OP;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->op = op;
    node->operand = operand;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_op_node(int op)
{
    struct UnaryOpNode *node = (struct UnaryOpNode *)malloc(sizeof(struct UnaryOpNode));
    node->type = NODE_OP;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->op = op;
    node->operand = NULL;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_class_decl(char *id, struct ASTNode *isa, struct ASTNode *inherit, struct ASTNode *members)
{
    struct ClassDeclNode *node = (struct ClassDeclNode *)malloc(sizeof(struct ClassDeclNode));
    node->type = NODE_CLASS_DECL;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->id = id;
    node->isa_list = isa;
    node->inheritance_list = inherit;
    node->members = members;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_visibility_node(char *visibility)
{
    struct IdentifierNode *node = (struct IdentifierNode *)malloc(sizeof(struct IdentifierNode));
    node->type = (strcmp(visibility, "public") == 0) ? NODE_PUBLIC : NODE_PRIVATE;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->name = strdup(visibility);
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_impl_def(char *id, struct ASTNode *func_list)
{
    struct ImplDefNode *node = (struct ImplDefNode *)malloc(sizeof(struct ImplDefNode));
    node->type = NODE_IMPL_DEF;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->id = id;
    node->func_defs = func_list;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_func_def(struct ASTNode *head, struct ASTNode *body)
{
    struct FuncDefNode *node = (struct FuncDefNode *)malloc(sizeof(struct FuncDefNode));
    node->type = NODE_FUNC_DEF;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->func_head = head;
    node->func_body = body;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_func_head(int is_ctor, char *id, struct ASTNode *params, struct ASTNode *ret_type)
{
    struct FuncHeadNode *node = (struct FuncHeadNode *)malloc(sizeof(struct FuncHeadNode));
    node->type = NODE_FUNC_HEAD;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->is_constructor = is_ctor;
    node->id = id;
    node->params = params;
    node->return_type = ret_type;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_type_node(char *type_name)
{
    struct IdentifierNode *node = (struct IdentifierNode *)malloc(sizeof(struct IdentifierNode));
    node->type = NODE_TYPE;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->name = strdup(type_name);
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_var_decl(char *id, struct ASTNode *type_node, struct ASTNode *dims)
{
    struct VarDeclNode *node = (struct VarDeclNode *)malloc(sizeof(struct VarDeclNode));
    node->type = NODE_VAR_DECL;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->id = id;
    node->type_node = type_node;
    node->array_dims = dims;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_if_node(struct ASTNode *cond, struct ASTNode *if_body, struct ASTNode *else_body)
{
    struct IfNode *node = (struct IfNode *)malloc(sizeof(struct IfNode));
    node->type = NODE_IF_STMT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->condition = cond;
    node->if_body = if_body;
    node->else_body = else_body;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_while_node(struct ASTNode *cond, struct ASTNode *body)
{
    struct WhileNode *node = (struct WhileNode *)malloc(sizeof(struct WhileNode));
    node->type = NODE_WHILE_STMT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->condition = cond;
    node->while_body = body;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_read_node(struct ASTNode *var)
{
    struct GenericNode *node = (struct GenericNode *)malloc(sizeof(struct GenericNode));
    node->type = NODE_READ_STMT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->child1 = var;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_write_node(struct ASTNode *expr)
{
    struct GenericNode *node = (struct GenericNode *)malloc(sizeof(struct GenericNode));
    node->type = NODE_WRITE_STMT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->child1 = expr;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_return_node(struct ASTNode *expr)
{
    struct GenericNode *node = (struct GenericNode *)malloc(sizeof(struct GenericNode));
    node->type = NODE_RETURN_STMT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->child1 = expr;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_assign_node(struct ASTNode *var, struct ASTNode *expr)
{
    struct AssignNode *node = (struct AssignNode *)malloc(sizeof(struct AssignNode));
    node->type = NODE_ASSIGN_STMT;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->variable = var;
    node->expression = expr;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_var_node(struct ASTNode *base, struct ASTNode *indices, struct ASTNode *members)
{
    struct VarAccessNode *node = (struct VarAccessNode *)malloc(sizeof(struct VarAccessNode));
    node->type = NODE_VARIABLE;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->base = base;
    node->indices = indices;
    node->members = members;
    return (struct ASTNode *)node;
}

static inline struct ASTNode *create_func_call(char *id, struct ASTNode *idnest, struct ASTNode *args)
{
    struct FuncCallNode *node = (struct FuncCallNode *)malloc(sizeof(struct FuncCallNode));
    node->type = NODE_FUNC_CALL;
    node->line_number = yylineno;
    node->next = NULL;
    node->scope = NULL;
    node->id = id;
    node->id_nest = idnest;
    node->args = args;
    return (struct ASTNode *)node;
}

#endif