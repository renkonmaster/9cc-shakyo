#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "9cc.h"
Token *tokenize(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);

#endif