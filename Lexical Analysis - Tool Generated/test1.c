// Intentionally contains multiple lexical errors for testing
int main() {
    int a = 10;
    int badHex = 0xG;     // hex not supported by this lexer (x, G illegal)

    char ch = 'z;         // unclosed char literal
    const char* s = "hello; // unclosed string literal

    a = a & 1;            // single '&' is illegal in current rules

    #include <stdio.h>    // preprocessor line starts with '#', illegal here

    /* unclosed block comment starts here and never ends at EOF
}

