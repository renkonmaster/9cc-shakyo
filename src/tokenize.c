#include "tokenize.h"
#include "util.h"

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

ReservedWord reservedWords[] = {
    {"return", 6, TK_RETURN},
    {"if", 2, TK_IF},
    {"else", 4, TK_ELSE},
    {"while", 5, TK_WHILE},
    {"for", 3, TK_FOR},
    {"break", 5, TK_BREAK},
    {"continue", 8, TK_CONTINUE},
    {"int", 3, TK_TYPE},
    {"char", 4, TK_TYPE},
    {"sizeof", 6, TK_SIZEOF},
    {NULL, 0, 0},
};

TokenKind check_reserved(char *name, int len) {
    for (int i = 0; reservedWords[i].str != NULL; i++) {
        ReservedWord *r = &reservedWords[i];
        if (r->len != len) {
            continue;
        }
        if (!memcmp(name, r->str, len)) {
            return r->kind;
        }
    }
    return TK_IDENT;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p) {

        if (*p == '"') {
            char *start = ++p;
            while (*p && *p != '"') {
                p++;
            }
            cur = new_token(TK_STR, cur, start, p - start);
            p++;
            continue;
        }

        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            while (*p != '\n')
                p++;
            continue;
        }

        if (strncmp(p, "/*", 2) == 0) {
            char *q = strstr(p + 2, "*/");
            if (!q)
                error("コメントが閉じられていません");
            p = q + 2;
            continue;
        }

        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=") ||
            startswith(p, "&&") || startswith(p, "||")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>;={},&[]!", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        if (isalpha(*p) || *p == '_') {
            char *start = p;

            p++;
            while (isalnum(*p) || *p == '_')
                p++;

            int len = p - start;
            TokenKind kind = check_reserved(start, len);
            
            cur = new_token(kind, cur, start, len);
            continue;
        }

        error(p, "invalid token");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
