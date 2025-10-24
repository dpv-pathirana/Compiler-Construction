%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

/* Global variables */
int line_num = 1;
int column_num = 1;
int error_count = 0;

/* Function prototypes */
int is_keyword(char *word);
void print_token(int type, char *lexeme, int line, int col);
void print_error(char *lexeme, int line, int col, char *message);

/* Reserved keywords */
char *keywords[] = {
    "else", "integer", "self", "float", "isa", "construct","constructor",
    "func", "private", "then", "if", "public", "local",
    "implement", "read", "void", "class", "return", "while",
    "attribute", "write", "string", NULL
};
%}

%option yylineno
%option noyywrap

/* Regular expression definitions */
letter          [a-zA-Z]
digit           [0-9]
nonzero         [1-9]
alphanum        ({letter}|{digit}|_)
identifier      {letter}{alphanum}*
integer         ({nonzero}{digit}*|0)
fraction        \.({digit}*{nonzero}|0)
exponent        [eE][+-]?{integer}
float_num       {integer}{fraction}({exponent})?
whitespace      [ \t]+
newline         \n

%%

{whitespace}        { column_num += yyleng; }
{newline}           { line_num++; column_num = 1; }

"//".* {
    print_token(COMMENT, yytext, line_num, column_num);
    column_num += yyleng;
}

"/*" {
    int c1, c2;
    int start_line = line_num;
    int start_col = column_num;
    int comment_closed = 0;
    column_num += 2;
    
    while ((c1 = input()) != EOF) {
        if (c1 == '\n') {
            line_num++;
            column_num = 1;
        } else {
            column_num++;
        }
        
        if (c1 == '*') {
            if ((c2 = input()) == '/') {
                column_num++;
                print_token(COMMENT, "/* ... */", start_line, start_col);
                comment_closed = 1;
                break;
            } else {
                unput(c2);
                if (c2 == '\n') {
                    line_num--;
                    column_num = strlen(yytext);
                } else {
                    column_num--;
                }
            }
        }
    }
    
    if (!comment_closed) {
        print_error("/*", start_line, start_col, "Unterminated block comment");
        error_count++;
    }
}

":=" {
    print_token(ASSIGN_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return ASSIGN_OP;
}

"==" {
    print_token(EQ_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return EQ_OP;
}

"=" {
    print_token(ASSIGN_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return ASSIGN_OP;
}

"<>" {
    print_token(NE_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return NE_OP;
}

"<=" {
    print_token(LE_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return LE_OP;
}

">=" {
    print_token(GE_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return GE_OP;
}

"=>" {
    print_token(ARROW, yytext, line_num, column_num);
    column_num += yyleng;
    return ARROW;
}

"<" {
    print_token(LT_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return LT_OP;
}

">" {
    print_token(GT_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return GT_OP;
}

"+" {
    print_token(PLUS_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return PLUS_OP;
}

"-" {
    print_token(MINUS_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return MINUS_OP;
}

"*" {
    print_token(MULT_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return MULT_OP;
}

"/" {
    print_token(DIV_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return DIV_OP;
}

"or" {
    print_token(OR_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return OR_OP;
}

"and" {
    print_token(AND_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return AND_OP;
}

"not" {
    print_token(NOT_OP, yytext, line_num, column_num);
    column_num += yyleng;
    return NOT_OP;
}

"(" {
    print_token(LPAREN, yytext, line_num, column_num);
    column_num += yyleng;
    return LPAREN;
}

")" {
    print_token(RPAREN, yytext, line_num, column_num);
    column_num += yyleng;
    return RPAREN;
}

"{" {
    print_token(LBRACE, yytext, line_num, column_num);
    column_num += yyleng;
    return LBRACE;
}

"}" {
    print_token(RBRACE, yytext, line_num, column_num);
    column_num += yyleng;
    return RBRACE;
}

"[" {
    print_token(LBRACKET, yytext, line_num, column_num);
    column_num += yyleng;
    return LBRACKET;
}

"]" {
    print_token(RBRACKET, yytext, line_num, column_num);
    column_num += yyleng;
    return RBRACKET;
}

";" {
    print_token(SEMICOLON, yytext, line_num, column_num);
    column_num += yyleng;
    return SEMICOLON;
}

"," {
    print_token(COMMA, yytext, line_num, column_num);
    column_num += yyleng;
    return COMMA;
}

"." {
    print_token(DOT, yytext, line_num, column_num);
    column_num += yyleng;
    return DOT;
}

":" {
    print_token(COLON, yytext, line_num, column_num);
    column_num += yyleng;
    return COLON;
}

\"[^"\n]*\" {
    print_token(STRING_LIT, yytext, line_num, column_num);
    column_num += yyleng;
    return STRING_LIT;
}

{float_num} {
    print_token(FLOAT_LIT, yytext, line_num, column_num);
    column_num += yyleng;
    return FLOAT_LIT;
}

{integer} {
    print_token(INTEGER_LIT, yytext, line_num, column_num);
    column_num += yyleng;
    return INTEGER_LIT;
}

{identifier} {
    if (is_keyword(yytext)) {
        print_token(KEYWORD, yytext, line_num, column_num);
        column_num += yyleng;
        return KEYWORD;
    } else {
        print_token(IDENTIFIER, yytext, line_num, column_num);
        column_num += yyleng;
        return IDENTIFIER;
    }
}

[0-9]+\.[0-9]*[eE][+-]?[0-9]*[a-zA-Z_] {
    print_error(yytext, line_num, column_num, "Invalid float literal format");
    column_num += yyleng;
    error_count++;
    return INVALID_TOKEN;
}

[0-9]+[a-zA-Z_][a-zA-Z0-9_]* {
    print_error(yytext, line_num, column_num, "Invalid identifier (cannot start with digit)");
    column_num += yyleng;
    error_count++;
    return INVALID_TOKEN;
}

\.[0-9]* {
    print_error(yytext, line_num, column_num, "Invalid float literal (missing integer part)");
    column_num += yyleng;
    error_count++;
    return INVALID_TOKEN;
}

[0-9]+\. {
    print_error(yytext, line_num, column_num, "Invalid float literal (missing fractional part)");
    column_num += yyleng;
    error_count++;
    return INVALID_TOKEN;
}

[0-9]+[eE][+-]? {
    print_error(yytext, line_num, column_num, "Invalid float literal (incomplete exponent)");
    column_num += yyleng;
    error_count++;
    return INVALID_TOKEN;
}

[@#$%^&`~|\\] {
    print_error(yytext, line_num, column_num, "Invalid character");
    column_num += yyleng;
    error_count++;
    return INVALID_TOKEN;
}

. {
    print_error(yytext, line_num, column_num, "Unrecognized character");
    column_num += yyleng;
    error_count++;
    return INVALID_TOKEN;
}

%%

/* Check if word is a keyword */
int is_keyword(char *word) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Print token information */
void print_token(int type, char *lexeme, int line, int col) {
    char *type_name;
    
    switch(type) {
        case KEYWORD: type_name = "KEYWORD"; break;
        case IDENTIFIER: type_name = "IDENTIFIER"; break;
        case INTEGER_LIT: type_name = "INTEGER"; break;
        case FLOAT_LIT: type_name = "FLOAT"; break;
        case ASSIGN_OP: type_name = "ASSIGN_OP"; break;
        case EQ_OP: type_name = "EQ_OP"; break;
        case NE_OP: type_name = "NE_OP"; break;
        case LE_OP: type_name = "LE_OP"; break;
        case GE_OP: type_name = "GE_OP"; break;
        case LT_OP: type_name = "LT_OP"; break;
        case GT_OP: type_name = "GT_OP"; break;
        case PLUS_OP: type_name = "PLUS_OP"; break;
        case MINUS_OP: type_name = "MINUS_OP"; break;
        case MULT_OP: type_name = "MULT_OP"; break;
        case DIV_OP: type_name = "DIV_OP"; break;
        case OR_OP: type_name = "OR_OP"; break;
        case AND_OP: type_name = "AND_OP"; break;
        case NOT_OP: type_name = "NOT_OP"; break;
        case LPAREN: type_name = "LPAREN"; break;
        case RPAREN: type_name = "RPAREN"; break;
        case LBRACE: type_name = "LBRACE"; break;
        case RBRACE: type_name = "RBRACE"; break;
        case LBRACKET: type_name = "LBRACKET"; break;
        case RBRACKET: type_name = "RBRACKET"; break;
        case SEMICOLON: type_name = "SEMICOLON"; break;
        case COMMA: type_name = "COMMA"; break;
        case DOT: type_name = "DOT"; break;
        case ARROW: type_name = "ARROW"; break;
        case COLON: type_name = "COLON"; break;
        case COMMENT: type_name = "COMMENT"; break;
        case STRING_LIT: type_name = "STRING_LIT"; break;
        case INVALID_TOKEN: type_name = "INVALID"; break;
        default: type_name = "UNKNOWN"; break;
    }
    
    printf("Token: %-12s Lexeme: %-15s Line: %d Column: %d\n", 
           type_name, lexeme, line, col);
}

/* Print error information */
void print_error(char *lexeme, int line, int col, char *message) {
    printf("ERROR: %-25s Lexeme: %-15s Line: %d Column: %d\n", 
           message, lexeme, line, col);
}