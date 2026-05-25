%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* 行列与统计 */
static long total_lines = 1;      /* 当前行号，从1开始 */
static long total_cols  = 1;      /* 当前列号，从1开始 */
static long total_chars = 0;      /* 字符总数 */
static long error_count = 0;      /* 错误总数 */

static long cnt_keywords = 0;
static long cnt_identifiers = 0;
static long cnt_numbers = 0;
static long cnt_operators = 0;
static long cnt_delimiters = 0;
static long cnt_strings = 0;
static long cnt_chars = 0;

/* 打印Token */
static void print_token(const char* type, const char* lexeme) {
    printf("%ld:%ld\t%-12s\t%s\n", total_lines, total_cols, type, lexeme);
}

/* 关键字判断 */
static int is_keyword(const char* s) {
    static const char* kws[] = {
        "auto","break","case","char","const","continue","default","do","double","else",
        "enum","extern","float","for","goto","if","int","long","register","return",
        "short","signed","sizeof","static","struct","switch","typedef","union","unsigned",
        "void","volatile","while","bool","true","false"
    };
    for (size_t i = 0; i < sizeof(kws)/sizeof(kws[0]); ++i) {
        if (strcmp(s, kws[i]) == 0) return 1;
    }
    return 0;
}

/* 逐字符推进行列与字符总数 */
static void advance_position(const char* text) {
    for (const char* p = text; *p; ++p) {
        ++total_chars;
        if (*p == '\n') {
            ++total_lines;
            total_cols = 1;
        } else {
            ++total_cols;
        }
    }
}
%}

/* 选项与 start conditions */
%option noyywrap
%x STR CHR MLCOM

/* 正则定义 */
ID      [A-Za-z_][A-Za-z0-9_]*
INT     0|([1-9][0-9]*)
EXP     ([eE][+-]?[0-9]+)
FLOAT   ((([0-9]+\.[0-9]*)|(\.[0-9]+))({EXP})?|(([0-9]+){EXP}))
WS      [\t\r \f]+
NL      \n
%%

{WS}            { advance_position(yytext); }
{NL}            { advance_position(yytext); }

"//"[^\n]*     { advance_position(yytext); }
"/*"           { BEGIN(MLCOM); advance_position(yytext); }
<MLCOM>"*/"    { BEGIN(INITIAL); advance_position(yytext); }
<MLCOM>\n      { advance_position(yytext); }
<MLCOM>.       { advance_position(yytext); }
<MLCOM><<EOF>> { 
                    fprintf(stderr, "Unclosed block comment at %ld:%ld\n", total_lines, total_cols); 
                    ++error_count; 
                    printf("\n==== Summary ====\n");
                    printf("Lines=%ld\n", total_lines);
                    printf("Chars=%ld\n", total_chars);
                    printf("Keywords=%ld\n", cnt_keywords);
                    printf("Idents=%ld\n", cnt_identifiers);
                    printf("Numbers=%ld\n", cnt_numbers);
                    printf("Strings=%ld\n", cnt_strings);
                    printf("CharsLit=%ld\n", cnt_chars);
                    printf("Ops=%ld\n", cnt_operators);
                    printf("Delims=%ld\n", cnt_delimiters);
                    printf("Errors=%ld\n", error_count);
                    return 0; 
                }


[\"]            { BEGIN(STR); advance_position(yytext); }
<STR>\\.        { advance_position(yytext); }
<STR>[\"]       { BEGIN(INITIAL); ++cnt_strings; print_token("STRING", "<str>"); advance_position(yytext); }
<STR>\n         { fprintf(stderr, "String literal newline (unclosed) at %ld:%ld\n", total_lines, total_cols); ++error_count; advance_position(yytext); BEGIN(INITIAL); }
<STR>.          { advance_position(yytext); }
<STR><<EOF>>    { 
                    fprintf(stderr, "Unclosed string literal at %ld:%ld\n", total_lines, total_cols); 
                    ++error_count; 
                    printf("\n==== Summary ====\n");
                    printf("Lines=%ld\n", total_lines);
                    printf("Chars=%ld\n", total_chars);
                    printf("Keywords=%ld\n", cnt_keywords);
                    printf("Idents=%ld\n", cnt_identifiers);
                    printf("Numbers=%ld\n", cnt_numbers);
                    printf("Strings=%ld\n", cnt_strings);
                    printf("CharsLit=%ld\n", cnt_chars);
                    printf("Ops=%ld\n", cnt_operators);
                    printf("Delims=%ld\n", cnt_delimiters);
                    printf("Errors=%ld\n", error_count);
                    return 0; 
                }


[\']            { BEGIN(CHR); advance_position(yytext); }
<CHR>\\.        { advance_position(yytext); }
<CHR>[^\\'\n]   { advance_position(yytext); }
<CHR>[\']       { BEGIN(INITIAL); ++cnt_chars; print_token("CHAR", "<char>"); advance_position(yytext); }
<CHR>\n         { fprintf(stderr, "Char literal newline (unclosed) at %ld:%ld\n", total_lines, total_cols); ++error_count; advance_position(yytext); BEGIN(INITIAL); }
<CHR><<EOF>>    { 
                    fprintf(stderr, "Unclosed char literal at %ld:%ld\n", total_lines, total_cols); 
                    ++error_count; 
                    printf("\n==== Summary ====\n");
                    printf("Lines=%ld\n", total_lines);
                    printf("Chars=%ld\n", total_chars);
                    printf("Keywords=%ld\n", cnt_keywords);
                    printf("Idents=%ld\n", cnt_identifiers);
                    printf("Numbers=%ld\n", cnt_numbers);
                    printf("Strings=%ld\n", cnt_strings);
                    printf("CharsLit=%ld\n", cnt_chars);
                    printf("Ops=%ld\n", cnt_operators);
                    printf("Delims=%ld\n", cnt_delimiters);
                    printf("Errors=%ld\n", error_count);
                    return 0; 
                }


{ID}            {
                    if (is_keyword(yytext)) { ++cnt_keywords; print_token("KEYWORD", yytext); }
                    else { ++cnt_identifiers; print_token("IDENT", yytext); }
                    advance_position(yytext);
                }


{FLOAT}         { ++cnt_numbers; print_token("NUMBER", yytext); advance_position(yytext); }
{INT}           { ++cnt_numbers; print_token("NUMBER", yytext); advance_position(yytext); }


"=="|"!="|"<="|">="|"&&"|"||" { ++cnt_operators; print_token("OP", yytext); advance_position(yytext); }
"++"|"--"|"+="|"-="|"*="|"/="   { ++cnt_operators; print_token("OP", yytext); advance_position(yytext); }
"+"|"-"|"*"|"/"|"%"|"="|"<"|">"|"!" { ++cnt_operators; print_token("OP", yytext); advance_position(yytext); }
"("|")"|"{"|"}"|"["|"]"|","|";" { ++cnt_delimiters; print_token("DELIM", yytext); advance_position(yytext); }


.               { fprintf(stderr, "Illegal character '%s' at %ld:%ld\n", yytext, total_lines, total_cols); ++error_count; advance_position(yytext); }

<<EOF>>         {
                    printf("\n==== Summary ====\n");
                    printf("Lines=%ld\n", total_lines);
                    printf("Chars=%ld\n", total_chars);
                    printf("Keywords=%ld\n", cnt_keywords);
                    printf("Idents=%ld\n", cnt_identifiers);
                    printf("Numbers=%ld\n", cnt_numbers);
                    printf("Strings=%ld\n", cnt_strings);
                    printf("CharsLit=%ld\n", cnt_chars);
                    printf("Ops=%ld\n", cnt_operators);
                    printf("Delims=%ld\n", cnt_delimiters);
                    printf("Errors=%ld\n", error_count);
                    return 0;
                }

%%
int main(void) {
    yylex();
    return 0;
}
