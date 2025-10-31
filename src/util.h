#ifndef UTIL_H
#define UTIL_H

#include "9cc.h"

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

bool consume(char *op);
bool consume_kind(TokenKind kind);
Token *consume_ident(void);
void expect(char *op);
int expect_number(void);
bool at_eof(void);

#endif