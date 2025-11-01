#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "9cc.h"

typedef struct {
    char *str;
    int len;
    TokenKind kind;
} ReservedWord;

Token *tokenize(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
TokenKind check_reserved(char *name, int len);
bool startswith(char *p, char *q);

#endif