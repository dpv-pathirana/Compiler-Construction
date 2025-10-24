#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

#include "ast.h"
#include "symbol_table.h"
#include "semantic.h"
#include "error_logger.h"

extern int yylex();
extern char *yytext;
extern int yylineno;
extern FILE *yyin;
extern int error_count;

int lookahead;
char current_lexeme[100];

void match(int expected);
void advance();
void error(const char *msg);

struct ASTNode *parse_prog();
struct ASTNode *parse_classOrImplOrFuncList();
struct ASTNode *parse_classOrImplOrFunc();

struct ASTNode *parse_classDecl();
struct ASTNode *parse_isaOpt();
struct ASTNode *parse_inheritanceList();
struct ASTNode *parse_visibilityMemberDeclList();
struct ASTNode *parse_visibility();
struct ASTNode *parse_memberDeclList();
struct ASTNode *parse_memberDecl();
struct ASTNode *parse_funcDecl();
struct ASTNode *parse_attributeDecl();

struct ASTNode *parse_implDef();
struct ASTNode *parse_funcDefList();

struct ASTNode *parse_funcDef();
struct ASTNode *parse_funcHead();
struct ASTNode *parse_returnType();
struct ASTNode *parse_type();
struct ASTNode *parse_funcBody();
struct ASTNode *parse_VarDeclOrStmtList();
struct ASTNode *parse_VarDeclOrStmt();
struct ASTNode *parse_localVarDecl();
struct ASTNode *parse_varDecl();
struct ASTNode *parse_arraySizeList();
struct ASTNode *parse_arraySize();

struct ASTNode *parse_statement();
struct ASTNode *parse_assignStat();
struct ASTNode *parse_statBlock();
struct ASTNode *parse_statementList();

struct ASTNode *parse_expr();
struct ASTNode *parse_exprRest(struct ASTNode *left_arith);
void parse_relOp();

struct ASTNode *parse_arithExpr();
struct ASTNode *parse_arithExprPrime(struct ASTNode *left_term);
void parse_addOp();

struct ASTNode *parse_term();
struct ASTNode *parse_termPrime(struct ASTNode *left_factor);
void parse_multOp();

struct ASTNode *parse_factor();
struct ASTNode *parse_sign();

struct ASTNode *parse_variable();
struct ASTNode *parse_idnestList();
struct ASTNode *parse_indiceList();
struct ASTNode *parse_indice();

struct ASTNode *parse_functionCall();
struct ASTNode *parse_idOrSelf();

struct ASTNode *parse_fParams();
struct ASTNode *parse_fParamsTailList();
struct ASTNode *parse_fParamsTail();

struct ASTNode *parse_aParams();
struct ASTNode *parse_aParamsTailList();
struct ASTNode *parse_aParamsTail();

void advance()
{
    lookahead = yylex();
    if (lookahead != 0)
    {
        strcpy(current_lexeme, yytext);
    }
}

void error(const char *msg)
{
    fprintf(stderr, "Syntax error at line %d: %s. Found token: %d (%s)\n", yylineno, msg, lookahead, current_lexeme);
    error_count++;
}

void match(int expected)
{
    if (lookahead == expected)
    {
        advance();
    }
    else
    {
        char msg[100];
        sprintf(msg, "expected %d, found %d", expected, lookahead);
        error(msg);

        advance();
    }
}

int is_keyword_str(const char *word)
{
    char *keywords[] = {
        "class", "isa", "implement", "public", "private", "attribute",
        "func", "constructor", "void", "integer", "float", "local",
        "if", "then", "else", "while", "read", "write", "return",
        "self", "not", "or", "and", NULL};

    for (int i = 0; keywords[i] != NULL; i++)
    {
        if (strcmp(word, keywords[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file)
    {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        return 1;
    }
    yyin = input_file;

    printf("--- Starting Parse (Building AST) ---\n");
    advance();
    struct ASTNode *ast_root = parse_prog();

    if (lookahead != 0)
    {
        error("Unexpected tokens at end of input");
    }

    if (error_count > 0)
    {
        printf("\nTotal syntax errors found: %d. Semantic analysis aborted.\n", error_count);
        fclose(input_file);
        return 1;
    }

    printf("--- Parse successful. Starting Semantic Analysis... ---\n");

    SymbolTable *table = create_symbol_table();

    printf("--- Running Pass 1: Building Symbol Table ---\n");
    build_symbol_table_pass(ast_root, table);

    printf("--- Running Pass 2: Type Checking ---\n");
    type_check_pass(ast_root, table);

    print_symbol_table_to_file(table, "symbol_table.txt");
    print_errors_to_file("semantic_errors.txt");

    int semantic_errors = get_semantic_error_count();
    if (semantic_errors > 0)
    {
        printf("\nSemantic analysis found %d errors.\n", semantic_errors);
    }
    else
    {
        printf("\nSemantic analysis completed with no errors.\n");
    }

    fclose(input_file);

    free_symbol_table(table);

    return (semantic_errors > 0 || error_count > 0) ? 1 : 0;
}

struct ASTNode *parse_prog()
{

    struct ASTNode *list = parse_classOrImplOrFuncList();

    return create_node(NODE_PROG, list, NULL);
}

struct ASTNode *parse_classOrImplOrFuncList()
{
    if (lookahead == KEYWORD &&
        (strcmp(current_lexeme, "class") == 0 ||
         strcmp(current_lexeme, "implement") == 0 ||
         strcmp(current_lexeme, "func") == 0 ||
         strcmp(current_lexeme, "constructor") == 0))
    {

        struct ASTNode *head = parse_classOrImplOrFunc();
        if (head)
        {
            head->next = parse_classOrImplOrFuncList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_classOrImplOrFunc()
{
    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "class") == 0)
        {
            return parse_classDecl();
        }
        else if (strcmp(current_lexeme, "implement") == 0)
        {
            return parse_implDef();
        }
        else if (strcmp(current_lexeme, "func") == 0 || strcmp(current_lexeme, "constructor") == 0)
        {
            return parse_funcDef();
        }
    }

    error("Expected class, implement, or func");
    return NULL;
}

struct ASTNode *parse_classDecl()
{

    match(KEYWORD);
    char *id = strdup(current_lexeme);
    match(IDENTIFIER);
    struct ASTNode *isa = parse_isaOpt();
    struct ASTNode *inherit = parse_inheritanceList();
    match(LBRACE);
    struct ASTNode *members = parse_visibilityMemberDeclList();
    match(RBRACE);

    return create_class_decl(id, isa, inherit, members);
}

struct ASTNode *parse_isaOpt()
{
    if (lookahead == KEYWORD && strcmp(current_lexeme, "isa") == 0)
    {

        match(KEYWORD);
        struct ASTNode *id_node = create_id_node(current_lexeme);
        match(IDENTIFIER);
        struct ASTNode *inherit = parse_inheritanceList();
        id_node->next = inherit;
        return id_node;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_inheritanceList()
{
    if (lookahead == COMMA)
    {

        match(COMMA);
        struct ASTNode *head = create_id_node(current_lexeme);
        match(IDENTIFIER);
        head->next = parse_inheritanceList();
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_visibilityMemberDeclList()
{
    if (lookahead == KEYWORD &&
        (strcmp(current_lexeme, "public") == 0 || strcmp(current_lexeme, "private") == 0))
    {

        struct ASTNode *visibility = parse_visibility();
        struct ASTNode *members = parse_memberDeclList();
        struct ASTNode *rest = parse_visibilityMemberDeclList();

        struct ASTNode *head = parse_memberDeclList();
        struct ASTNode *tail = parse_visibilityMemberDeclList();

        if (head == NULL)
            return tail;
        struct ASTNode *current = head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = tail;
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_visibility()
{
    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "public") == 0)
        {
            match(KEYWORD);
            return create_visibility_node("public");
        }
        else if (strcmp(current_lexeme, "private") == 0)
        {
            match(KEYWORD);
            return create_visibility_node("private");
        }
    }
    error("Expected public or private");
    return NULL;
}

struct ASTNode *parse_memberDeclList()
{
    if (lookahead == KEYWORD && (strcmp(current_lexeme, "func") == 0 ||
                                 strcmp(current_lexeme, "attribute") == 0 ||
                                 strcmp(current_lexeme, "constructor") == 0))
    {

        struct ASTNode *head = parse_memberDecl();
        if (head)
        {
            head->next = parse_memberDeclList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_memberDecl()
{
    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "func") == 0 ||
            strcmp(current_lexeme, "constructor") == 0)
        {

            return parse_funcDef();
        }
        else if (strcmp(current_lexeme, "attribute") == 0)
        {

            return parse_attributeDecl();
        }
    }
    error("Expected func, attribute or constructor");
    return NULL;
}

struct ASTNode *parse_funcDecl()
{

    struct ASTNode *head = parse_funcHead();
    match(SEMICOLON);

    return create_node(NODE_FUNC_DECL, head, NULL);
}

struct ASTNode *parse_attributeDecl()
{

    match(KEYWORD);
    struct ASTNode *var_decl = parse_varDecl();

    return create_node(NODE_ATTRIBUTE_DECL, var_decl, NULL);
}

struct ASTNode *parse_implDef()
{

    match(KEYWORD);
    char *id = strdup(current_lexeme);
    match(IDENTIFIER);
    match(LBRACE);
    struct ASTNode *func_list = parse_funcDefList();
    match(RBRACE);

    return create_impl_def(id, func_list);
}

struct ASTNode *parse_funcDefList()
{
    if (lookahead == KEYWORD &&
        (strcmp(current_lexeme, "func") == 0 || strcmp(current_lexeme, "constructor") == 0))
    {

        struct ASTNode *head = parse_funcDef();
        if (head)
        {
            head->next = parse_funcDefList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_funcDef()
{

    struct ASTNode *head = parse_funcHead();
    struct ASTNode *body = parse_funcBody();

    return create_func_def(head, body);
}

struct ASTNode *parse_funcHead()
{
    int is_ctor = 0;
    char *id = NULL;
    struct ASTNode *params = NULL;
    struct ASTNode *ret_type = NULL;

    if (lookahead == KEYWORD && strcmp(current_lexeme, "func") == 0)
    {

        match(KEYWORD);
        id = strdup(current_lexeme);
        match(IDENTIFIER);
        match(LPAREN);
        params = parse_fParams();
        match(RPAREN);
        match(ARROW);
        ret_type = parse_returnType();
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "constructor") == 0)
    {

        is_ctor = 1;
        match(KEYWORD);
        id = strdup("constructor");
        match(LPAREN);
        params = parse_fParams();
        match(RPAREN);

        ret_type = create_type_node("void");
    }
    else
    {
        error("Expected func or constructor");
        return NULL;
    }

    return create_func_head(is_ctor, id, params, ret_type);
}

struct ASTNode *parse_returnType()
{
    if (lookahead == KEYWORD && strcmp(current_lexeme, "void") == 0)
    {

        match(KEYWORD);
        return create_type_node("void");
    }
    else
    {

        return parse_type();
    }
}

struct ASTNode *parse_type()
{
    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "integer") == 0)
        {
            match(KEYWORD);
            return create_type_node("integer");
        }
        else if (strcmp(current_lexeme, "float") == 0)
        {
            match(KEYWORD);
            return create_type_node("float");
        }
        else if (strcmp(current_lexeme, "string") == 0)
        {
            match(KEYWORD);
            return create_type_node("string");
        }
    }

    error("Expected integer, float, or id");
    return NULL;
}

struct ASTNode *parse_funcBody()
{

    match(LBRACE);
    struct ASTNode *list = parse_VarDeclOrStmtList();
    match(RBRACE);
    return create_node(NODE_FUNC_BODY, list, NULL);
}

struct ASTNode *parse_VarDeclOrStmtList()
{
    if (lookahead == KEYWORD &&
            (strcmp(current_lexeme, "local") == 0 ||
             strcmp(current_lexeme, "if") == 0 ||
             strcmp(current_lexeme, "while") == 0 ||
             strcmp(current_lexeme, "read") == 0 ||
             strcmp(current_lexeme, "write") == 0 ||
             strcmp(current_lexeme, "self") == 0 ||
             strcmp(current_lexeme, "return") == 0) ||
        lookahead == IDENTIFIER)
    {

        struct ASTNode *head = parse_VarDeclOrStmt();
        if (head)
        {
            head->next = parse_VarDeclOrStmtList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_VarDeclOrStmt()
{
    if (lookahead == KEYWORD && strcmp(current_lexeme, "local") == 0)
    {

        return parse_localVarDecl();
    }
    else if (lookahead == IDENTIFIER || (lookahead == KEYWORD &&
                                         (strcmp(current_lexeme, "if") == 0 ||
                                          strcmp(current_lexeme, "while") == 0 ||
                                          strcmp(current_lexeme, "read") == 0 ||
                                          strcmp(current_lexeme, "write") == 0 ||
                                          strcmp(current_lexeme, "return") == 0 ||
                                          strcmp(current_lexeme, "self") == 0)))
    {

        return parse_statement();
    }
    else
    {
        error("Expected local variable declaration or statement");
        return NULL;
    }
}

struct ASTNode *parse_localVarDecl()
{

    match(KEYWORD);
    return parse_varDecl();
}

struct ASTNode *parse_varDecl()
{

    char *id = strdup(current_lexeme);
    match(IDENTIFIER);
    match(COLON);
    struct ASTNode *type_node = parse_type();
    struct ASTNode *dims = parse_arraySizeList();
    match(SEMICOLON);

    return create_var_decl(id, type_node, dims);
}

struct ASTNode *parse_arraySizeList()
{
    if (lookahead == LBRACKET)
    {

        struct ASTNode *head = parse_arraySize();
        if (head)
        {
            head->next = parse_arraySizeList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_arraySize()
{

    match(LBRACKET);
    struct ASTNode *size_node = NULL;
    if (lookahead == INTEGER_LIT)
    {
        int val = atoi(current_lexeme);
        match(INTEGER_LIT);
        size_node = create_int_lit(val);
    }
    match(RBRACKET);

    return size_node;
}

struct ASTNode *parse_statement()
{

    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "if") == 0)
        {

            match(KEYWORD);
            match(LPAREN);
            struct ASTNode *cond = parse_expr();
            match(RPAREN);
            if (lookahead == KEYWORD && strcmp(current_lexeme, "then") == 0)
            {
                match(KEYWORD);
            }
            else
            {
                error("Syntax error: expected 'then' after if condition");
            }

            struct ASTNode *if_body = parse_statBlock();
            struct ASTNode *else_body = NULL;

            if (lookahead == KEYWORD && strcmp(current_lexeme, "else") == 0)
            {
                match(KEYWORD);
                if (lookahead == LBRACE)
                {

                    else_body = parse_statBlock();
                }
                else
                {

                    else_body = parse_statement();
                }
            }

            return create_if_node(cond, if_body, else_body);
        }

        else if (strcmp(current_lexeme, "while") == 0)
        {

            match(KEYWORD);
            match(LPAREN);
            struct ASTNode *cond = parse_expr();
            match(RPAREN);
            struct ASTNode *body = parse_statBlock();
            match(SEMICOLON);

            return create_while_node(cond, body);
        }

        else if (strcmp(current_lexeme, "read") == 0)
        {

            match(KEYWORD);
            match(LPAREN);
            struct ASTNode *var = parse_variable();
            match(RPAREN);
            match(SEMICOLON);

            return create_read_node(var);
        }

        else if (strcmp(current_lexeme, "write") == 0)
        {

            match(KEYWORD);
            match(LPAREN);
            struct ASTNode *expr = parse_expr();
            match(RPAREN);
            match(SEMICOLON);

            return create_write_node(expr);
        }

        else if (strcmp(current_lexeme, "return") == 0)
        {

            match(KEYWORD);
            struct ASTNode *expr = parse_expr();
            match(SEMICOLON);

            return create_return_node(expr);
        }
    }
    else if (lookahead == IDENTIFIER)
    {
        struct ASTNode *expr_node = parse_expr();
        if (expr_node->type == NODE_FUNC_CALL)
        {
            match(SEMICOLON);
            return expr_node;
        }
        else if (expr_node->type == NODE_VARIABLE)
        {
            match(ASSIGN_OP);
            struct ASTNode *rhs_expr = parse_expr();
            match(SEMICOLON);
            return create_assign_node(expr_node, rhs_expr);
        }
        else
        {
            error("Syntax error: Statement must be an assignment or function call");
            return NULL;
        }
    }
    else if (lookahead == LBRACE)
    {
        return parse_statBlock();
    }
    else
    {
        error("Syntax error: Unexpected token in statement");
        advance();
        return NULL;
    }

    return NULL;
}

struct ASTNode *parse_assignStat()
{

    struct ASTNode *var = parse_variable();
    match(ASSIGN_OP);
    struct ASTNode *expr = parse_expr();

    return create_assign_node(var, expr);
}

struct ASTNode *parse_statBlock()
{
    if (lookahead == LBRACE)
    {

        match(LBRACE);
        struct ASTNode *list = parse_statementList();
        match(RBRACE);
        return create_node(NODE_STAT_BLOCK, list, NULL);
    }
    else if (lookahead == KEYWORD || lookahead == IDENTIFIER)
    {

        return parse_statement();
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_statementList()
{
    if (lookahead == KEYWORD || lookahead == IDENTIFIER)
    {

        struct ASTNode *head = parse_statement();
        if (head)
        {
            head->next = parse_statementList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_expr()
{

    struct ASTNode *left_arith = parse_arithExpr();

    return parse_exprRest(left_arith);
}

struct ASTNode *parse_exprRest(struct ASTNode *left_arith)
{
    if (lookahead == EQ_OP || lookahead == NE_OP || lookahead == LT_OP ||
        lookahead == GT_OP || lookahead == LE_OP || lookahead == GE_OP)
    {

        int op = lookahead;
        parse_relOp();
        struct ASTNode *right_arith = parse_arithExpr();

        return create_bin_op(op, left_arith, right_arith);
    }
    else
    {

        return left_arith;
    }
}

void parse_relOp()
{

    switch (lookahead)
    {
    case EQ_OP:
        match(EQ_OP);
        break;
    case NE_OP:
        match(NE_OP);
        break;
    case LT_OP:
        match(LT_OP);
        break;
    case GT_OP:
        match(GT_OP);
        break;
    case LE_OP:
        match(LE_OP);
        break;
    case GE_OP:
        match(GE_OP);
        break;
    default:
        error("Expected relational operator");
    }
}

struct ASTNode *parse_arithExpr()
{

    struct ASTNode *left_term = parse_term();

    return parse_arithExprPrime(left_term);
}

struct ASTNode *parse_arithExprPrime(struct ASTNode *left_term)
{
    if (lookahead == PLUS_OP || lookahead == MINUS_OP ||
        (lookahead == KEYWORD && strcmp(current_lexeme, "or") == 0))
    {

        int op = lookahead;
        parse_addOp();
        struct ASTNode *right_term = parse_term();

        struct ASTNode *new_left = create_bin_op(op, left_term, right_term);

        return parse_arithExprPrime(new_left);
    }
    else
    {

        return left_term;
    }
}

void parse_addOp()
{
    if (lookahead == PLUS_OP)
    {
        match(PLUS_OP);
    }
    else if (lookahead == MINUS_OP)
    {
        match(MINUS_OP);
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "or") == 0)
    {
        match(KEYWORD);
    }
    else
    {
        error("Expected +, -, or or");
    }
}

struct ASTNode *parse_term()
{

    struct ASTNode *left_factor = parse_factor();

    return parse_termPrime(left_factor);
}

struct ASTNode *parse_termPrime(struct ASTNode *left_factor)
{
    if (lookahead == MULT_OP || lookahead == DIV_OP ||
        (lookahead == KEYWORD && strcmp(current_lexeme, "and") == 0))
    {

        int op = lookahead;
        parse_multOp();
        struct ASTNode *right_factor = parse_factor();

        struct ASTNode *new_left = create_bin_op(op, left_factor, right_factor);

        return parse_termPrime(new_left);
    }
    else
    {

        return left_factor;
    }
}

void parse_multOp()
{
    if (lookahead == MULT_OP)
    {
        match(MULT_OP);
    }
    else if (lookahead == DIV_OP)
    {
        match(DIV_OP);
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "and") == 0)
    {
        match(KEYWORD);
    }
    else
    {
        error("Expected *, /, or and");
    }
}

struct ASTNode *parse_factor()
{
    if (lookahead == IDENTIFIER)
    {
        char *id = strdup(current_lexeme);
        match(IDENTIFIER);

        if (lookahead == LPAREN)
        {

            match(LPAREN);
            struct ASTNode *args = parse_aParams();
            match(RPAREN);

            return create_func_call(id, NULL, args);
        }
        else
        {

            struct ASTNode *id_node = create_id_node(id);
            struct ASTNode *indices = parse_indiceList();

            return create_var_node(id_node, indices, NULL);
        }
    }
    else if (lookahead == INTEGER_LIT)
    {

        int val = atoi(current_lexeme);
        match(INTEGER_LIT);
        return create_int_lit(val);
    }
    else if (lookahead == FLOAT_LIT)
    {

        float val = atof(current_lexeme);
        match(FLOAT_LIT);
        return create_float_lit(val);
    }
    else if (lookahead == STRING_LIT)
    {

        char *val = strdup(current_lexeme);
        match(STRING_LIT);
        return create_string_lit(val);
    }
    else if (lookahead == LPAREN)
    {

        match(LPAREN);
        struct ASTNode *expr = parse_arithExpr();
        match(RPAREN);
        return expr;
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "not") == 0)
    {

        match(KEYWORD);
        struct ASTNode *operand = parse_factor();

        return create_unary_op(NOT_OP, operand);
    }
    else if (lookahead == PLUS_OP || lookahead == MINUS_OP)
    {

        int op = lookahead;
        parse_sign();
        struct ASTNode *operand = parse_factor();

        return create_unary_op(op, operand);
    }
    else
    {
        error("Expected factor");
        return NULL;
    }
}

struct ASTNode *parse_sign()
{
    if (lookahead == PLUS_OP)
    {
        match(PLUS_OP);
        return create_op_node(PLUS_OP);
    }
    else if (lookahead == MINUS_OP)
    {
        match(MINUS_OP);
        return create_op_node(MINUS_OP);
    }
    else
    {
        error("Expected + or -");
        return NULL;
    }
}

struct ASTNode *parse_variable()
{

    struct ASTNode *var_base = parse_idOrSelf();
    struct ASTNode *indices = parse_indiceList();
    struct ASTNode *members = parse_idnestList();

    return create_var_node(var_base, indices, members);
}

struct ASTNode *parse_idnestList()
{
    if (lookahead == COMMA)
    {

        match(COMMA);
        struct ASTNode *head = parse_idOrSelf();
        struct ASTNode *indices = parse_indiceList();

        struct ASTNode *nested_var = create_var_node(head, indices, NULL);
        nested_var->next = parse_idnestList();
        return nested_var;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_indiceList()
{
    if (lookahead == LBRACKET)
    {

        struct ASTNode *head = parse_indice();
        if (head)
        {
            head->next = parse_indiceList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_indice()
{

    match(LBRACKET);
    struct ASTNode *expr = parse_arithExpr();
    match(RBRACKET);
    return expr;
}

struct ASTNode *parse_functionCall()
{

    struct ASTNode *idnest = parse_idnestList();
    char *id = strdup(current_lexeme);
    match(IDENTIFIER);
    match(LPAREN);
    struct ASTNode *args = parse_aParams();
    match(RPAREN);

    return create_func_call(id, idnest, args);
}

struct ASTNode *parse_idOrSelf()
{
    char *id_name;
    if (lookahead == IDENTIFIER)
    {
        id_name = strdup(current_lexeme);
        match(IDENTIFIER);
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "self") == 0)
    {
        id_name = strdup(current_lexeme);
        match(KEYWORD);
    }
    else
    {
        error("Expected id or self");
        return NULL;
    }
    return create_id_node(id_name);
}

struct ASTNode *parse_fParams()
{
    if (lookahead == IDENTIFIER)
    {

        char *id = strdup(current_lexeme);
        match(IDENTIFIER);
        match(COLON);
        struct ASTNode *type = parse_type();
        struct ASTNode *dims = parse_arraySizeList();

        struct ASTNode *head = create_var_decl(id, type, dims);

        head->next = parse_fParamsTailList();
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_fParamsTailList()
{
    if (lookahead == COMMA)
    {

        struct ASTNode *head = parse_fParamsTail();
        if (head)
        {
            head->next = parse_fParamsTailList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_fParamsTail()
{

    match(COMMA);
    char *id = strdup(current_lexeme);
    match(IDENTIFIER);
    match(COLON);
    struct ASTNode *type = parse_type();
    struct ASTNode *dims = parse_arraySizeList();

    return create_var_decl(id, type, dims);
}

struct ASTNode *parse_aParams()
{
    if (lookahead == IDENTIFIER || lookahead == INTEGER_LIT || lookahead == FLOAT_LIT ||
        lookahead == STRING_LIT ||
        lookahead == LPAREN || lookahead == PLUS_OP || lookahead == MINUS_OP ||
        (lookahead == KEYWORD && strcmp(current_lexeme, "not") == 0))
    {

        struct ASTNode *head = parse_expr();
        if (head)
        {
            head->next = parse_aParamsTailList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_aParamsTailList()
{
    if (lookahead == COMMA)
    {

        struct ASTNode *head = parse_aParamsTail();
        if (head)
        {
            head->next = parse_aParamsTailList();
        }
        return head;
    }
    else
    {

        return NULL;
    }
}

struct ASTNode *parse_aParamsTail()
{

    match(COMMA);
    return parse_expr();
}