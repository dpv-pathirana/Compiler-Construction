#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

extern int yylex();
extern char *yytext;
extern int yylineno;
extern FILE *yyin;
extern int error_count;

FILE *derivation_file;

int lookahead;
char current_lexeme[100];

void match(int expected);

// Program Structure
void parse_prog();
void parse_classOrImplOrFuncList();
void parse_classOrImplOrFunc();

// Class Declaration
void parse_classDecl();
void parse_isaOpt();
void parse_inheritanceList();
void parse_visibilityMemberDeclList();
void parse_visibility();
void parse_memberDeclList();
void parse_memberDecl();
void parse_funcDecl();
void parse_attributeDecl();

// Implementation Definition
void parse_implDef();
void parse_funcDefList();

// Function Definition
void parse_funcDef();
void parse_funcHead();
void parse_returnType();
void parse_type();
void parse_funcBody();
void parse_VarDeclOrStmtList();
void parse_VarDeclOrStmt();
void parse_localVarDecl();
void parse_varDecl();
void parse_arraySizeList();
void parse_arraySize();

// Statements
void parse_statement();
void parse_assignStat();
void parse_statBlock();
void parse_statementList();

// Expressions
void parse_expr();
void parse_exprRest();
void parse_relOp();

void parse_arithExpr();
void parse_arithExprPrime();
void parse_addOp();

void parse_term();
void parse_termPrime();
void parse_multOp();

void parse_factor();
void parse_sign();

// Variables and Function Calls
void parse_variable();
void parse_idnestList();
void parse_indiceList();
void parse_indice();

void parse_functionCall();

// void parse_idnest();
void parse_idOrSelf();

// Parameters
void parse_fParams();
void parse_fParamsTailList();
void parse_fParamsTail();

void parse_aParams();
void parse_aParamsTailList();
void parse_aParamsTail();

// Utility
void advance();

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
    }
}

void write_derivation(const char *production)
{
    fprintf(derivation_file, "%s\n", production);
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

    derivation_file = fopen("derivation.txt", "w");
    if (!derivation_file)
    {
        perror("Failed to open derivation file");
        fclose(input_file);
        return 1;
    }

    advance();
    parse_prog();

    if (lookahead != 0)
    {
        error("Unexpected tokens at end of input");
    }

    if (error_count > 0)
    {
        printf("\nTotal syntax errors found: %d\n", error_count);
    }
    else
    {
        printf("No syntax errors found.\n");
        printf("Parsing completed successfully!\n");
        printf("Derivation written to 'derivation.txt'\n");
    }

    fclose(derivation_file);
    fclose(input_file);
    return error_count > 0 ? 1 : 0;
}

void parse_prog()
{
    write_derivation("prog -> classOrImplOrFuncList");
    parse_classOrImplOrFuncList();
}

void parse_classOrImplOrFuncList()
{
    if (lookahead == KEYWORD &&
        (strcmp(current_lexeme, "class") == 0 ||
         strcmp(current_lexeme, "implement") == 0 ||
         strcmp(current_lexeme, "func") == 0 ||
         strcmp(current_lexeme, "constructor") == 0))
    {
        write_derivation("classOrImplOrFuncList -> classOrImplOrFunc classOrImplOrFuncList");
        parse_classOrImplOrFunc();
        parse_classOrImplOrFuncList();
    }
    else
    {
        write_derivation("classOrImplOrFuncList -> epsilon");
        // epsilon production
    }
}

void parse_classOrImplOrFunc()
{
    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "class") == 0)
        {
            write_derivation("classOrImplOrFunc -> classDecl");
            parse_classDecl();
        }
        else if (strcmp(current_lexeme, "implement") == 0)
        {
            write_derivation("classOrImplOrFunc -> implDef");
            parse_implDef();
        }
        else if (strcmp(current_lexeme, "func") == 0 || strcmp(current_lexeme, "constructor") == 0)
        {
            write_derivation("classOrImplOrFunc -> funcDef");
            parse_funcDef();
        }
        else
        {
            error("Expected class, implement, or func");
        }
    }
    else
    {
        error("Expected class, implement, or func");
    }
}

void parse_classDecl()
{
    write_derivation("classDecl -> class id isaOpt inheritanceList { visibilityMemberDeclList }");
    match(KEYWORD);    // class
    match(IDENTIFIER); // id
    parse_isaOpt();
    parse_inheritanceList();
    match(LBRACE);
    parse_visibilityMemberDeclList();
    match(RBRACE);
}

void parse_isaOpt()
{
    if (lookahead == KEYWORD && strcmp(current_lexeme, "isa") == 0)
    {
        write_derivation("isaOpt -> isa id inheritanceList");
        match(KEYWORD);    // isa
        match(IDENTIFIER); // id
        parse_inheritanceList();
    }
    else
    {
        write_derivation("isaOpt -> epsilon");
        // epsilon production
    }
}

void parse_inheritanceList()
{
    if (lookahead == COMMA)
    {
        write_derivation("inheritanceList -> , id inheritanceList");
        match(COMMA);
        match(IDENTIFIER);
        parse_inheritanceList();
    }
    else
    {
        write_derivation("inheritanceList -> epsilon");
        // epsilon production
    }
}

void parse_visibilityMemberDeclList()
{
    if (lookahead == KEYWORD &&
        (strcmp(current_lexeme, "public") == 0 || strcmp(current_lexeme, "private") == 0))
    {
        write_derivation("visibilityMemberDeclList -> visibility memberDeclList visibilityMemberDeclList");
        parse_visibility();
        parse_memberDeclList();
        parse_visibilityMemberDeclList();
    }
    else
    {
        write_derivation("visibilityMemberDeclList -> epsilon");
        // epsilon production
    }
}

void parse_visibility()
{
    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "public") == 0)
        {
            write_derivation("visibility -> public");
            match(KEYWORD);
        }
        else if (strcmp(current_lexeme, "private") == 0)
        {
            write_derivation("visibility -> private");
            match(KEYWORD);
        }
        else
        {
            error("Expected public or private");
        }
    }
    else
    {
        error("Expected public or private");
    }
}

void parse_memberDeclList()
{
    if (lookahead == KEYWORD && (strcmp(current_lexeme, "func") == 0 ||
                                 strcmp(current_lexeme, "attribute") == 0 ||
                                 strcmp(current_lexeme, "constructor") == 0))
    {
        write_derivation("memberDeclList -> memberDecl memberDeclList");
        parse_memberDecl();
        parse_memberDeclList();
    }
    else
    {
        // Epsilon case for when the list of members ends
        write_derivation("memberDeclList -> epsilon");
    }
}

void parse_memberDecl()
{
    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "func") == 0 ||
            strcmp(current_lexeme, "constructor") == 0)
        {
            write_derivation("memberDecl -> funcDef");
            parse_funcDef();
        }
        else if (strcmp(current_lexeme, "attribute") == 0)
        {
            write_derivation("memberDecl -> attributeDecl");
            parse_attributeDecl();
        }
        else
        {
            error("Expected func, attribute or constructor");
        }
    }
    else
    {
        error("Expected func, attribute or constructor");
    }
}

void parse_funcDecl()
{
    write_derivation("funcDecl -> funcHead ;");
    parse_funcHead();
    match(SEMICOLON);
}

void parse_attributeDecl()
{
    write_derivation("attributeDecl -> attribute varDecl");
    match(KEYWORD); // attribute
    parse_varDecl();
}

void parse_implDef()
{
    write_derivation("implDef -> implement id { funcDefList }");
    match(KEYWORD);    // implement
    match(IDENTIFIER); // id
    match(LBRACE);
    parse_funcDefList();
    match(RBRACE);
}

void parse_funcDefList()
{
    if (lookahead == KEYWORD &&
        (strcmp(current_lexeme, "func") == 0 || strcmp(current_lexeme, "constructor") == 0))
    {
        write_derivation("funcDefList -> funcDef funcDefList");
        parse_funcDef();
        parse_funcDefList();
    }
    else
    {
        write_derivation("funcDefList -> epsilon");
        // epsilon production
    }
}

void parse_funcDef()
{
    write_derivation("funcDef -> funcHead funcBody");
    parse_funcHead();
    parse_funcBody();
}

void parse_funcHead()
{
    if (lookahead == KEYWORD && strcmp(current_lexeme, "func") == 0)
    {
        write_derivation("funcHead -> func id ( fParams ) => returnType");
        match(KEYWORD);    // func
        match(IDENTIFIER); // id
        match(LPAREN);
        parse_fParams();
        match(RPAREN);
        match(ARROW);
        parse_returnType();
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "constructor") == 0)
    {
        write_derivation("funcHead -> constructor ( fParams )");
        match(KEYWORD); // constructor
        match(LPAREN);
        parse_fParams();
        match(RPAREN);
    }
    else
    {
        error("Expected func or constructor");
    }
}

void parse_returnType()
{
    if (lookahead == KEYWORD && strcmp(current_lexeme, "void") == 0)
    {
        write_derivation("returnType -> void");
        match(KEYWORD);
    }
    else
    {
        write_derivation("returnType -> type");
        parse_type();
    }
}

void parse_type()
{
    if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "integer") == 0)
        {
            write_derivation("type -> integer");
            match(KEYWORD);
        }
        else if (strcmp(current_lexeme, "float") == 0)
        {
            write_derivation("type -> float");
            match(KEYWORD);
        }
        else
        {
            error("Expected integer, float, or id");
        }
    }
    else if (lookahead == IDENTIFIER)
    {
        write_derivation("type -> id");
        match(IDENTIFIER);
    }
    else
    {
        error("Expected integer, float, or id");
    }
}

void parse_funcBody()
{
    write_derivation("funcBody -> { VarDeclOrStmtList }");
    match(LBRACE);
    parse_VarDeclOrStmtList();
    match(RBRACE);
}

void parse_VarDeclOrStmtList()
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
        write_derivation("VarDeclOrStmtList -> VarDeclOrStmt VarDeclOrStmtList");
        parse_VarDeclOrStmt();
        parse_VarDeclOrStmtList();
    }
    else
    {
        write_derivation("VarDeclOrStmtList -> epsilon");
        // epsilon production
    }
}

void parse_VarDeclOrStmt()
{
    if (lookahead == KEYWORD && strcmp(current_lexeme, "local") == 0)
    {
        write_derivation("VarDeclOrStmt -> localVarDecl");
        parse_localVarDecl();
    }
    else
    {
        write_derivation("VarDeclOrStmt -> statement");
        parse_statement();
    }
}

void parse_localVarDecl()
{
    write_derivation("localVarDecl -> local varDecl");
    match(KEYWORD); // local
    parse_varDecl();
}

void parse_varDecl()
{
    write_derivation("varDecl -> id : type arraySizeList ;");
    match(IDENTIFIER);
    match(COLON);
    parse_type();
    parse_arraySizeList();
    match(SEMICOLON);
}

void parse_arraySizeList()
{
    if (lookahead == LBRACKET)
    {
        write_derivation("arraySizeList -> arraySize arraySizeList");
        parse_arraySize();
        parse_arraySizeList();
    }
    else
    {
        write_derivation("arraySizeList -> epsilon");
        // epsilon production
    }
}

void parse_arraySize()
{
    write_derivation("arraySize -> [ intLit ] | [ ]");
    match(LBRACKET);
    if (lookahead == INTEGER_LIT)
    {
        match(INTEGER_LIT);
    }
    match(RBRACKET);
}

void parse_statement()
{
    if (lookahead == IDENTIFIER || strcmp(current_lexeme, "self") == 0)
    {
        write_derivation("statement -> assignStat ;");
        parse_assignStat();
        match(SEMICOLON);
    }
    else if (lookahead == KEYWORD)
    {
        if (strcmp(current_lexeme, "if") == 0)
        {
            write_derivation("statement -> if ( relExpr ) then statBlock else statBlock ;");
            match(KEYWORD); // if
            match(LPAREN);
            parse_expr();
            match(RPAREN);
            match(KEYWORD); // then
            parse_statBlock();
            match(KEYWORD); // else
            parse_statBlock();
            match(SEMICOLON);
        }
        else if (strcmp(current_lexeme, "while") == 0)
        {
            write_derivation("statement -> while ( relExpr ) statBlock ;");
            match(KEYWORD); // while
            match(LPAREN);
            parse_expr();
            match(RPAREN);
            parse_statBlock();
            match(SEMICOLON);
        }
        else if (strcmp(current_lexeme, "read") == 0)
        {
            write_derivation("statement -> read ( variable ) ;");
            match(KEYWORD); // read
            match(LPAREN);
            parse_variable();
            match(RPAREN);
            match(SEMICOLON);
        }
        else if (strcmp(current_lexeme, "write") == 0)
        {
            write_derivation("statement -> write ( expr ) ;");
            match(KEYWORD); // write
            match(LPAREN);
            parse_expr();
            match(RPAREN);
            match(SEMICOLON);
        }
        else if (strcmp(current_lexeme, "return") == 0)
        {
            write_derivation("statement -> return expr ;");
            match(KEYWORD); // return
            parse_expr();
            match(SEMICOLON);
        }
        else
        {
            error("Expected if, while, read, write, or return");
        }
    }
    else
    {
        error("Expected statement");
    }
}

void parse_assignStat()
{
    write_derivation("assignStat -> variable assignOp expr");
    parse_variable();
    match(ASSIGN_OP);
    parse_expr();
}

void parse_statBlock()
{
    if (lookahead == LBRACE)
    {
        write_derivation("statBlock -> { statementList }");
        match(LBRACE);
        parse_statementList();
        match(RBRACE);
    }
    else if (lookahead == KEYWORD || lookahead == IDENTIFIER)
    {
        write_derivation("statBlock -> statement");
        parse_statement();
    }
    else
    {
        write_derivation("statBlock -> epsilon");
        // epsilon production
    }
}

void parse_statementList()
{
    if (lookahead == KEYWORD || lookahead == IDENTIFIER)
    {
        write_derivation("statementList -> statement statementList");
        parse_statement();
        parse_statementList();
    }
    else
    {
        write_derivation("statementList -> epsilon");
        // epsilon production
    }
}

void parse_expr()
{
    write_derivation("expr -> arithExpr exprRest");
    parse_arithExpr();
    parse_exprRest();
}

void parse_exprRest()
{
    if (lookahead == EQ_OP || lookahead == NE_OP || lookahead == LT_OP ||
        lookahead == GT_OP || lookahead == LE_OP || lookahead == GE_OP)
    {
        write_derivation("exprRest -> relOp arithExpr");
        parse_relOp();
        parse_arithExpr();
    }
    else
    {
        write_derivation("exprRest -> epsilon");
        // epsilon production
    }
}

void parse_relOp()
{
    switch (lookahead)
    {
    case EQ_OP:
        write_derivation("relOp -> ==");
        match(EQ_OP);
        break;
    case NE_OP:
        write_derivation("relOp -> <>");
        match(NE_OP);
        break;
    case LT_OP:
        write_derivation("relOp -> <");
        match(LT_OP);
        break;
    case GT_OP:
        write_derivation("relOp -> >");
        match(GT_OP);
        break;
    case LE_OP:
        write_derivation("relOp -> <=");
        match(LE_OP);
        break;
    case GE_OP:
        write_derivation("relOp -> >=");
        match(GE_OP);
        break;
    default:
        error("Expected relational operator");
    }
}

void parse_arithExpr()
{
    write_derivation("arithExpr -> term arithExpr'");
    parse_term();
    parse_arithExprPrime();
}

void parse_arithExprPrime()
{
    if (lookahead == PLUS_OP || lookahead == MINUS_OP ||
        (lookahead == KEYWORD && strcmp(current_lexeme, "or") == 0))
    {
        write_derivation("arithExpr' -> addOp term arithExpr'");
        parse_addOp();
        parse_term();
        parse_arithExprPrime();
    }
    else
    {
        write_derivation("arithExpr' -> epsilon");
        // epsilon production
    }
}

void parse_addOp()
{
    if (lookahead == PLUS_OP)
    {
        write_derivation("addOp -> +");
        match(PLUS_OP);
    }
    else if (lookahead == MINUS_OP)
    {
        write_derivation("addOp -> -");
        match(MINUS_OP);
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "or") == 0)
    {
        write_derivation("addOp -> or");
        match(KEYWORD);
    }
    else
    {
        error("Expected +, -, or or");
    }
}

void parse_term()
{
    write_derivation("term -> factor term'");
    parse_factor();
    parse_termPrime();
}

void parse_termPrime()
{
    if (lookahead == MULT_OP || lookahead == DIV_OP ||
        (lookahead == KEYWORD && strcmp(current_lexeme, "and") == 0))
    {
        write_derivation("term' -> multOp factor term'");
        parse_multOp();
        parse_factor();
        parse_termPrime();
    }
    else
    {
        write_derivation("term' -> epsilon");
        // epsilon production
    }
}

void parse_multOp()
{
    if (lookahead == MULT_OP)
    {
        write_derivation("multOp -> *");
        match(MULT_OP);
    }
    else if (lookahead == DIV_OP)
    {
        write_derivation("multOp -> /");
        match(DIV_OP);
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "and") == 0)
    {
        write_derivation("multOp -> and");
        match(KEYWORD);
    }
    else
    {
        error("Expected *, /, or and");
    }
}

void parse_factor()
{
    if (lookahead == IDENTIFIER || strcmp(current_lexeme, "self") == 0)
    {
        write_derivation("factor -> variable");
        parse_variable();
    }
    else if (lookahead == INTEGER_LIT)
    {
        write_derivation("factor -> intLit");
        match(INTEGER_LIT);
    }
    else if (lookahead == FLOAT_LIT)
    {
        write_derivation("factor -> floatLit");
        match(FLOAT_LIT);
    }
    else if (lookahead == STRING_LIT)
    {
        write_derivation("factor -> stringLit");
        match(STRING_LIT);
    }
    else if (lookahead == LPAREN)
    {
        write_derivation("factor -> ( arithExpr )");
        match(LPAREN);
        parse_arithExpr();
        match(RPAREN);
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "not") == 0)
    {
        write_derivation("factor -> not factor");
        match(KEYWORD);
        parse_factor();
    }
    else if (lookahead == PLUS_OP || lookahead == MINUS_OP)
    {
        write_derivation("factor -> sign factor");
        parse_sign();
        parse_factor();
    }
    else
    {
        error("Expected factor");
    }
}

void parse_sign()
{
    if (lookahead == PLUS_OP)
    {
        write_derivation("sign -> +");
        match(PLUS_OP);
    }
    else if (lookahead == MINUS_OP)
    {
        write_derivation("sign -> -");
        match(MINUS_OP);
    }
    else
    {
        error("Expected + or -");
    }
}

void parse_variable()
{
    write_derivation("variable -> idOrSelf indiceList idnestList");
    parse_idOrSelf();
    parse_indiceList();
    parse_idnestList();
}

void parse_idnestList()
{
    if (lookahead == COMMA)
    {
        write_derivation("idnestList -> , idOrSelf indiceList idnestList");
        match(COMMA);
        parse_idOrSelf();
        parse_indiceList();
        parse_idnestList();
    }
    else
    {
        write_derivation("idnestList -> epsilon");
        // epsilon production
    }
}

void parse_indiceList()
{
    if (lookahead == LBRACKET)
    {
        write_derivation("indiceList -> indice indiceList");
        parse_indice();
        parse_indiceList();
    }
    else
    {
        write_derivation("indiceList -> epsilon");
        // epsilon production
    }
}

void parse_indice()
{
    write_derivation("indice -> [ arithExpr ]");
    match(LBRACKET);
    parse_arithExpr();
    match(RBRACKET);
}

void parse_functionCall()
{
    write_derivation("functionCall -> idnestList id ( aParams )");
    parse_idnestList();
    match(IDENTIFIER);
    match(LPAREN);
    parse_aParams();
    match(RPAREN);
}
/*
void parse_idnest()
{
    if (lookahead == IDENTIFIER || (lookahead == KEYWORD && strcmp(current_lexeme, "self") == 0))
    {
        write_derivation("idnest -> idOrSelf indiceList ,");
        parse_idOrSelf();
        parse_indiceList();
        match(COMMA);
    }
    else
    {
        error("Expected id or self");
    }
}
*/
void parse_idOrSelf()
{
    if (lookahead == IDENTIFIER)
    {
        write_derivation("idOrSelf -> id");
        match(IDENTIFIER);
    }
    else if (lookahead == KEYWORD && strcmp(current_lexeme, "self") == 0)
    {
        write_derivation("idOrSelf -> self");
        match(KEYWORD);
    }
    else
    {
        error("Expected id or self");
    }
}

void parse_fParams()
{
    if (lookahead == IDENTIFIER)
    {
        write_derivation("fParams -> id : type arraySizeList fParamsTailList");
        match(IDENTIFIER);
        match(COLON);
        parse_type();
        parse_arraySizeList();
        parse_fParamsTailList();
    }
    else
    {
        write_derivation("fParams -> epsilon");
        // epsilon production
    }
}

void parse_fParamsTailList()
{
    if (lookahead == COMMA)
    {
        write_derivation("fParamsTailList -> fParamsTail fParamsTailList");
        parse_fParamsTail();
        parse_fParamsTailList();
    }
    else
    {
        write_derivation("fParamsTailList -> epsilon");
        // epsilon production
    }
}

void parse_fParamsTail()
{
    write_derivation("fParamsTail -> , id : type arraySizeList");
    match(COMMA);
    match(IDENTIFIER);
    match(COLON);
    parse_type();
    parse_arraySizeList();
}

void parse_aParams()
{
    if (lookahead == IDENTIFIER || lookahead == INTEGER_LIT || lookahead == FLOAT_LIT ||
        lookahead == STRING_LIT ||
        lookahead == LPAREN || lookahead == PLUS_OP || lookahead == MINUS_OP ||
        (lookahead == KEYWORD && strcmp(current_lexeme, "not") == 0))
    {
        write_derivation("aParams -> expr aParamsTailList");
        parse_expr();
        parse_aParamsTailList();
    }
    else
    {
        write_derivation("aParams -> epsilon");
        // epsilon production
    }
}

void parse_aParamsTailList()
{
    if (lookahead == COMMA)
    {
        write_derivation("aParamsTailList -> aParamsTail aParamsTailList");
        parse_aParamsTail();
        parse_aParamsTailList();
    }
    else
    {
        write_derivation("aParamsTailList -> epsilon");
        // epsilon production
    }
}

void parse_aParamsTail()
{
    write_derivation("aParamsTail -> , expr");
    match(COMMA);
    parse_expr();
}