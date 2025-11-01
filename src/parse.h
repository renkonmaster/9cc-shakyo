#ifndef PARSE_H
#define PARSE_H

#include "9cc.h"

Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

bool consume(char *op);
bool consume_kind(TokenKind kind);
Token *consume_ident();
void expect(char *op);
int expect_number();

bool at_eof();

void program(void);
Node *function_def(Type *ret_type, Token *tok);
void declaration(void);
void global_declaration(Type *base, Token *tok);
Node *stmt(void);
Node *expr(void);
Node *assign(void);
Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);
Node *array_to_ptr(Node *node);

LVar *find_lvar(Token *tok);
GVar *find_gvar(Token *tok);

#endif