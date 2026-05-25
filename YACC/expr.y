%{
    #include <stdio.h>
    #include <stdlib.h>

    int yylex(void);
    void yyerror(const char *s);
%}

/* 终结符声明 */
%token NUM

/* 指定运算符优先级和结合性：从低到高 */
%left '+' '-'
%left '*' '/'

%%

/* 文法规则部分 */

E   : E '+' T
        {
            printf("E -> E + T\n");
        }
    | E '-' T
        {
            printf("E -> E - T\n");
        }
    | T
        {
            printf("E -> T\n");
        }
    ;

T   : T '*' F
        {
            printf("T -> T * F\n");
        }
    | T '/' F
        {
            printf("T -> T / F\n");
        }
    | F
        {
            printf("T -> F\n");
        }
    ;

F   : '(' E ')'
        {
            printf("F -> ( E )\n");
        }
    | NUM
        {
            printf("F -> num\n");
        }
    ;

%%

int main(void)
{
    printf("请输入一个算术表达式，例如：\n");
    printf("  3+4*5-6/2\n\n");
    printf("输入后回车，然后再按 Ctrl+D（Linux/mac）或 Ctrl+Z 回车（Windows）结束输入。\n\n");

    if (yyparse() == 0) {
        printf("\n语法分析成功！\n");
    } else {
        printf("\n语法分析失败！\n");
    }
    return 0;
}

void yyerror(const char *s)
{
    fprintf(stderr, "语法错误: %s\n", s);
}
