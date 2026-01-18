#ifndef SRC_LEXER_H
#define SRC_LEXER_H

#include <stddef.h>

typedef struct {
    char *buf;
    size_t buf_size;
    size_t cursor;
    size_t line_start;
    size_t line_nr;
} Lexer;

typedef enum {
    TOKEN_EOF,
    TOKEN_ERR,
    TOKEN_IDENT,
    TOKEN_KEYWORD,
    TOKEN_NUMBER,
    TOKEN_OPEN_CURLY,
    TOKEN_CLOSE_CURLY,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_SEMI_COLON,
} Token_Type;

typedef struct {
    Token_Type type;
    char *str;
    size_t len;
} Token;

void lexer_init(Lexer *l, char *buf, size_t buf_size);
Token lexer_next_token(Lexer *l);
void lexer_report_error(Lexer *l, Token tok, char *source_path);

#endif

#ifdef LEXER_IMPLEMENTATION

#include <ctype.h>
#include <stdbool.h>

static bool
is_c_ident_start(char c)
{
    if (isalpha(c) || c == '_') {
        return true;
    }
    return false;
}

static bool
is_c_ident(char c)
{
    if (is_c_ident_start(c) || isdigit(c)) {
        return true;
    }
    return false;
}

static bool
is_c_digit(char c)
{
    if (isdigit(c)) {
        return true;
    }
    return false;
}

static bool
is_c_boundry(char c)
{
    if (isspace(c) || ispunct(c)) {
        return true;
    }
    return false;
}

static bool
is_str_keyword(char *str, size_t len)
{
    const char *c_keywords[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline",
        "int", "long", "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void",
        "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local",
    };

    for (uint i = 0; i < array_len(c_keywords); ++i) {
        if (strncmp(str, c_keywords[i], len) == 0) {
            return true;
        }
    }
    return false;
}

static int
str_find_next_newline(char *str, size_t len)
{
    size_t i = 0;
    for (; i < len; ++i) {
        if (str[i] == '\n') break;
    }
    return i;
}

void
lexer_init(Lexer *l, char *buf, size_t buf_size) {
    memset(l, 0, sizeof(*l));
    l->line_nr = 1;
    l->buf = buf;
    l->buf_size = buf_size;
}

Token
lexer_next_token(Lexer *l) {
    if (l->cursor >= l->buf_size) return (Token){ .type = TOKEN_EOF };

    while(isspace(l->buf[l->cursor])) {
        if (l->buf[l->cursor] == '\n' && l->cursor + 1 < l->buf_size) {
            l->line_start = l->cursor + 1;
            l->line_nr += 1;
        }
        l->cursor += 1;
        if (l->cursor >= l->buf_size) return (Token){ .type = TOKEN_EOF };
    }

    size_t mark = l->cursor;
    Token tok;

    if (is_c_ident_start(l->buf[l->cursor])) {
        while (++l->cursor < l->buf_size && is_c_ident(l->buf[l->cursor]));
        if (!is_c_boundry(l->buf[l->cursor])) return (Token){ .type = TOKEN_ERR };
        tok.type = TOKEN_IDENT;
        if (is_str_keyword(&l->buf[mark], l->cursor - mark)) tok.type = TOKEN_KEYWORD;
    } else if (is_c_digit(l->buf[l->cursor])) {
        while (++l->cursor < l->buf_size && is_c_digit(l->buf[l->cursor]));
        if (!is_c_boundry(l->buf[l->cursor])) return (Token){ .type = TOKEN_ERR };
        tok.type = TOKEN_NUMBER;
    } else if (l->buf[l->cursor] == '{') {
        ++l->cursor;
        tok.type = TOKEN_OPEN_CURLY;
    } else if (l->buf[l->cursor] == '}') {
        ++l->cursor;
        tok.type = TOKEN_CLOSE_CURLY;
    } else if (l->buf[l->cursor] == '(') {
        ++l->cursor;
        tok.type = TOKEN_OPEN_PAREN;
    } else if (l->buf[l->cursor] == ')') {
        ++l->cursor;
        tok.type = TOKEN_CLOSE_PAREN;
    } else if (l->buf[l->cursor] == ';') {
        ++l->cursor;
        tok.type = TOKEN_SEMI_COLON;
    } else {
        ++l->cursor;
        tok.type = TOKEN_ERR;
        tok.str = "Unknown symbol!";
        tok.len = strlen(tok.str);
    }

    if (tok.type > TOKEN_ERR) {
        tok.str = &l->buf[mark];
        tok.len = l->cursor - mark;
    }

    return tok;
}

void
lexer_report_error(Lexer *l, Token tok, char *source_path)
{
        int column = l->cursor - l->line_start;
        char *line = &l->buf[l->line_start];
        int line_len = str_find_next_newline(line, l->buf_size - l->line_start);

        fprintf(stderr, "%s:%ld:%d: Syntax Error: %.*s\n", source_path, l->line_nr, column, (int)tok.len, tok.str);
        fprintf(stderr, "%.*s\n", line_len, line);
        fprintf(stderr, "%*c^\n", column - 1, ' ');
}

#endif
