#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

// 简化的错误类型枚举
typedef enum {
    ERROR_NONE = 0,
    ERROR_LEXICAL_ERROR = 1        // 词法错误
} ErrorType;

// 错误信息结构体
typedef struct {
    ErrorType type;
    int line;
    int column;
    char message[200];
    char context[100];
} ErrorInfo;

// 统计结构体
typedef struct {
    int total_lines;        // 总行数
    int statement_lines;    // 语句行数
    int keyword_count;      // 关键字个数
    int identifier_count;   // 标识符个数
    int constant_count;     // 常数个数
    int operator_count;     // 运算符个数
    int delimiter_count;    // 分隔符个数
    int total_chars;        // 字符总数
    int error_count;        // 错误总数
} Statistics;

// C 语言关键字列表
const char* keywords[] = {
    "auto", "break", "case", "char", "const", "continue",
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "inline", "int",
    "long", "register", "restrict", "return", "short", "signed",
    "sizeof", "static", "struct", "switch", "typedef", "union",
    "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof",
    "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn",
    "_Static_assert", "_Thread_local"
};
//运算符一类
const char operator_1[4] = { '.','~','?',':' };
//运算符二类(后面可能会接=）
const char operator_2[6] = { '!','*','/','%','^','=' };

// 简化的错误处理函数
void report_error(ErrorInfo* error, FILE* output_file) {
    printf("词法错误 [行%d, 列%d]\n", error->line, error->column);
    
    if (output_file) {
        fprintf(output_file, "词法错误 [行%d, 列%d]\n", error->line, error->column);
    }
}

// 错误恢复函数 - 跳过错误并继续分析
int recover_from_error(FILE* file, char* current_char, int* line, int* column) {
    char ch = *current_char;
    
    // 根据错误类型进行不同的恢复策略
    if (ch == '"') {
        // 字符串错误恢复：跳过到行末或下一个引号
        while ((ch = fgetc(file)) != EOF && ch != '\n' && ch != '"') {
            (*column)++;
        }
        if (ch == '\n') {
            (*line)++;
            *column = 1;
        }
    } else if (ch == '/') {
        // 注释错误恢复：跳过到行末
        while ((ch = fgetc(file)) != EOF && ch != '\n') {
            (*column)++;
        }
        if (ch == '\n') {
            (*line)++;
            *column = 1;
        }
    } else {
        // 其他错误：跳过当前字符
        ch = fgetc(file);
        (*column)++;
    }
    
    *current_char = ch;
    return ch;
}

//预处理（带错误检测）
void pretreatment(char* getfilepath, char* putfilepath, Statistics* stats) {
    FILE* yuan = NULL;
    FILE* yuchli = NULL;
    char ch;
    int line = 1, column = 1;
    bool in_comment = false;
    int comment_start_line = 0, comment_start_col = 0;
    
    if (fopen_s(&yuan, getfilepath, "r") != 0 ) {
        printf("源程序 文件打开失败！");
        exit(1);
    }
    if (fopen_s(&yuchli, putfilepath, "w") != 0) {
        printf("预处理 文件打开失败！");
        exit(1);
    }
    
    while ((ch = fgetc(yuan)) != EOF) {
        // 注释处理
        if (ch == '/') {
            char ch2 = fgetc(yuan);
            if (ch2 == '/') {
                // 单行注释
                while ((ch = fgetc(yuan)) != '\n' && ch != EOF) {
                    column++;
                }
                if (ch == '\n') {
                    line++;
                    column = 1;
                }
                fputc('\n', yuchli);
                putchar('\n');
                continue;
            }
            else if (ch2 == '*') {
                // 多行注释开始
                in_comment = true;
                comment_start_line = line;
                comment_start_col = column;
                while (true) {
                    while ((ch = fgetc(yuan)) != EOF && ch != '*') {
                        if (ch == '\n') {
                            line++;
                            column = 1;
                        } else {
                            column++;
                        }
                    }
                    if (ch == EOF) {
                        // 未闭合的注释错误
                        ErrorInfo error = {ERROR_NONE, 0, 0, "", ""};
                        error.type = ERROR_LEXICAL_ERROR;
                        error.line = comment_start_line;
                        error.column = comment_start_col;
                        report_error(&error, yuchli);
                        stats->error_count++;
                        break;
                    }
                    ch = fgetc(yuan);
                    if (ch == '/') {
                        in_comment = false;
                        break;
                    }
                }
                continue;
            }
            else {
                fputc(ch, yuchli);
                putchar(ch);
                ch = ch2;
                column++;
            }
        } 
        //处理汉字
        else if (ch < 0 && ch != EOF) {
            fseek(yuan, -1L, 1);
            char zh[3] = { 0 };
            fgets(zh, 2, yuan);
            fputs(zh, yuchli);
            printf("%s", zh);
            column += 2; // 中文字符占2个字节
            continue;
        }
        else {
            fputc(ch, yuchli);
            putchar(ch);
            if (ch == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
        }
    }
    
    fclose(yuan);
    fclose(yuchli);
    printf("\n预处理完成\n\n");
}


//词法分析（带错误检测和恢复）
void lexical_analysis(char* getfilepath, char* putfilepath, Statistics* stats) {
    printf("开始词法分析：\n");
    // 打开文件
    FILE* cifa = NULL;
    FILE* yuchli = NULL;
    char ch;
    char token[100] = {0};
    int row = 1, col = 1;
    bool in_statement = false;  // 是否在语句中
    bool in_string = false;    // 是否在字符串中
    int string_start_line = 0, string_start_col = 0;
    
    if (fopen_s(&yuchli, getfilepath, "r") != 0) {
        printf("预处理 文件打开失败！");
        exit(1);
    }
    if (fopen_s(&cifa, putfilepath, "w") != 0) {
        printf("词法分析 文件打开失败！");
        exit(1);
    }

    //词法分析
    ch = fgetc(yuchli);
    while (true) {
        if (ch == EOF) {
            // 检查是否有未闭合的字符串
            if (in_string) {
                ErrorInfo error = {ERROR_NONE, 0, 0, "", ""};
                error.type = ERROR_LEXICAL_ERROR;
                error.line = string_start_line;
                error.column = string_start_col;
                report_error(&error, cifa);
                stats->error_count++;
            }
            break;
        }

        // 更新行列位置
        if (ch == '\n') {
            row++;
            col = 1;
        } else {
            col++;
        }

        //标识符或保留字
        if (ch > 0 && (isalpha(ch) || ch == '_')) { 
            int tokenindex = 0;
            token[tokenindex++] = ch;
            while ((ch = fgetc(yuchli)) != EOF && ch > 0 && (isalpha(ch) || isdigit(ch) || ch == '_')) {
                token[tokenindex++] = ch;
                col++;
            }
            token[tokenindex++] = '\0';

            int s = 0;
            int i = 0;
            for (; i < 44; i++) {
                if (strcmp(token, keywords[i]) == 0) {
                    char text[100] = "(";
                    strcat_s(text, 100, token);
                    strcat_s(text, 100, ", 基本保留字)");
                    printf("%s\n", text);
                    fputs(text, cifa);
                    stats->keyword_count++;
                    in_statement = true;
                    break;
                }
            }
            if (i == 44) {
                char text[100] = "(";
                strcat_s(text, 100, token);
                strcat_s(text, 100, ", 标识符)");
                printf("%s\n", text);
                fputs(text, cifa);
                stats->identifier_count++;
                in_statement = true;
            }
        }

        //字符串处理 - 带错误检测
        else if (ch == '"') {
            in_string = true;
            string_start_line = row;
            string_start_col = col - 1;
            
            // 跳过字符串内容，直到遇到结束引号
            while ((ch = fgetc(yuchli)) != EOF && ch != '"') {
                if (ch == '\n') {
                    row++;
                    col = 1;
                } else {
                    col++;
                }
                
                // 处理中文字符，直接跳过
                if (ch < 0 && ch != EOF) {
                    if (fseek(yuchli, -1L, 1) == 0) {
                        char zh[3] = { 0 };
                        if (fgets(zh, 2, yuchli) != NULL) {
                            // 跳过中文字符，不进行任何处理
                            col += 2;
                        }
                    }
                }
            }
            
            if (ch == '"') {
                in_string = false;
                col++;
            } else {
                // 字符串未闭合错误
                ErrorInfo error = {ERROR_NONE, 0, 0, "", ""};
                error.type = ERROR_LEXICAL_ERROR;
                error.line = string_start_line;
                error.column = string_start_col;
                report_error(&error, cifa);
                stats->error_count++;
                in_string = false;
            }
        }
        //常数（带错误检测）
        else if (ch > 0 && isdigit(ch)) {
            int tokenindex = 0;
            token[tokenindex++] = ch;
            bool valid_number = true;
            int number_start_line = row;
            int number_start_col = col - 1;
            
            while ((ch = fgetc(yuchli)) != EOF && ch > 0 && isdigit(ch)) {
                token[tokenindex++] = ch;
                col++;
            }
            if (ch == '.') {
                token[tokenindex++] = ch;
                col++;
                while ((ch = fgetc(yuchli)) != EOF && ch > 0 && isdigit(ch)) {
                    token[tokenindex++] = ch;
                    col++;
                }
            }
            if (ch == 'e') {
                token[tokenindex++] = ch;
                col++;
                if ((ch = fgetc(yuchli)) != EOF && ch > 0 && (isdigit(ch) || ch == '+' || ch == '-')) {
                    token[tokenindex++] = ch;
                    col++;
                    while ((ch = fgetc(yuchli)) != EOF && ch > 0 && isdigit(ch)) {
                        token[tokenindex++] = ch;
                        col++;
                    }
                } else {
                    valid_number = false;
                }
            }
            
            // 检查数字后是否跟着非法字符
            if (ch > 0 && isalpha(ch)) {
                valid_number = false;
            }
            
            token[tokenindex++] = '\0';

            if (valid_number) {
                char text[100] = "(";
                strcat_s(text, 100, token);
                strcat_s(text, 100, ", 常数)");
                printf("%s\n", text);
                fputs(text, cifa);
                stats->constant_count++;
                in_statement = true;
            } else {
                // 无效数字错误
                ErrorInfo error = {ERROR_NONE, 0, 0, "", ""};
                error.type = ERROR_LEXICAL_ERROR;
                error.line = number_start_line;
                error.column = number_start_col;
                report_error(&error, cifa);
                stats->error_count++;
            }
        }

        //算数运算符与界符
        else if (ch > 0 && isgraph(ch)) {
            int jie = 1;  //是否为界符
            int up = 0;  //是否更新ch
            int tokenindex = 0;
            token[tokenindex++] = ch;
            if (ch == '+') {
                jie = 0;
                ch = fgetc(yuchli);
                if (ch == '+' || ch == '=') {
                    token[tokenindex++] = ch;
                    up = 1;
                }
            }
            else if (ch == '-') {
                jie = 0;
                ch = fgetc(yuchli);
                if (ch == '-' || ch == '='|| ch == '>') {
                    token[tokenindex++] = ch;
                    up = 1;
                }
            }
            else if (ch == '<') {
                jie = 0;
                ch = fgetc(yuchli);
                if (ch == '=') {
                    token[tokenindex++] = ch;
                    up = 1;
                }
                if (ch == '<') {
                    token[tokenindex++] = ch;
                    ch = fgetc(yuchli);
                    if (ch == '=') {
                        token[tokenindex++] = ch;
                        up = 1;
                    }
                }
            }
            else if (ch == '>') {
                jie = 0;
                ch = fgetc(yuchli);
                if (ch == '=') {
                    token[tokenindex++] = ch;
                    up = 1;
                }
                if (ch == '>') {
                    token[tokenindex++] = ch;
                    ch = fgetc(yuchli);
                    if (ch == '=') {
                        token[tokenindex++] = ch;
                        up = 1;
                    }
                }
            }
            else if (ch == '&') {
                jie = 0;
                ch = fgetc(yuchli);
                if (ch == '&') {
                    token[tokenindex++] = ch;
                    up = 1;
                }
            }
            else if (ch == '|') {
                jie = 0;
                ch = fgetc(yuchli);
                if (ch == '|') {
                    token[tokenindex++] = ch;
                    up = 1;
                }
            }
            else {
                for (int i = 0; i < 4; i++) {
                    if (ch == operator_1[i]) {
                        jie = 0;
                        up = 1;
                        break;
                    }
                }
                for (int i = 0; i < 6; i++) {
                    if (ch == operator_2[i])
                    {
                        jie = 0;
                        ch = fgetc(yuchli);
                        if (ch == '=') {
                            token[tokenindex++] = ch;
                            up = 1;
                        }
                    }

                }
            }
            token[tokenindex++] = '\0';

            if (jie) {
                char text[100] = "(";
                strcat_s(text, 100, token);
                strcat_s(text, 100, ", 分隔符)");
                printf("%s\n", text);
                fputs(text, cifa);
                stats->delimiter_count++;
                if (ch == ';' || ch == '{' || ch == '}') {
                    in_statement = false;  // 语句结束
                }
                up = 1;
            }
            else {
                char text[100] = "(";
                strcat_s(text, 100, token);
                strcat_s(text, 100, ", 运算符)");
                printf("%s\n", text);
                fputs(text, cifa);
                stats->operator_count++;
                in_statement = true;
            }

            if (up) {
                ch = fgetc(yuchli);
            }
        }

        //出错处理 - 带错误恢复
        else if(ch != EOF && !isspace(ch)) {
            // 如果是中文字符，跳过不报错
            if (ch < 0) {
                // 处理中文字符，跳过
                if (fseek(yuchli, -1L, 1) == 0) {
                    char zh[3] = { 0 };
                    if (fgets(zh, 2, yuchli) != NULL) {
                        // 跳过中文字符
                        col += 2;
                    }
                }
                ch = fgetc(yuchli);
                continue;
            }
            // 其他无法识别的字符报错并恢复
            ErrorInfo error = {ERROR_NONE, 0, 0, "", ""};
            error.type = ERROR_LEXICAL_ERROR;
            error.line = row;
            error.column = col - 1;
            report_error(&error, cifa);
            stats->error_count++;
            
            // 错误恢复：跳过当前字符
            ch = fgetc(yuchli);
        }

        if (ch > 0 && isspace(ch)) {
            if (ch == '\n') {
                row++;
                stats->total_lines++;
                if (in_statement) {
                    stats->statement_lines++;
                    in_statement = false;
                }
            }
            stats->total_chars++;
            ch = fgetc(yuchli);
        }
        else {
            stats->total_chars++;
        }
    }

    printf("词法分析完成\n");
    fclose(yuchli);
    fclose(cifa);
}

// 输出统计结果
void print_statistics(Statistics* stats) {
    printf("\n========== 统计结果 ==========\n");
    printf("总行数: %d\n", stats->total_lines);
    printf("语句行数: %d\n", stats->statement_lines);
    printf("关键字个数: %d\n", stats->keyword_count);
    printf("标识符个数: %d\n", stats->identifier_count);
    printf("常数个数: %d\n", stats->constant_count);
    printf("运算符个数: %d\n", stats->operator_count);
    printf("分隔符个数: %d\n", stats->delimiter_count);
    printf("字符总数: %d\n", stats->total_chars);
    printf("词法错误总数: %d\n", stats->error_count);
    printf("==============================\n");
}


int main() {
    char yuan[100] = "raw_test3.txt";
    char yuchli[100] = "pretreatment.txt";
    char cifa[100] = "lexical_analysis.txt";
    
    // 初始化统计结构体
    Statistics stats = {0};
    
    printf("========== C语言词法分析器（带错误检测和恢复） ==========\n");
    printf("正在分析文件: %s\n", yuan);
    printf("====================================================\n\n");
    
    pretreatment(yuan, yuchli, &stats);
    lexical_analysis(yuchli, cifa, &stats);
    
    // 输出统计结果
    print_statistics(&stats);
    
    if (stats.error_count > 0) {
        printf("\n注意：发现 %d 个词法错误，但分析已继续完成。\n", stats.error_count);
    } else {
        printf("\n恭喜：未发现词法错误！\n");
    }

    return 0;
}


