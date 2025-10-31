#ifndef CODEGEN_H
#define CODEGEN_H

#include "9cc.h"

void gen(Node *node);
void gen_lval(Node *node);
void gen_if(Node *node);
void gen_while(Node *node);
void gen_for(Node *node);
void gen_block(Node *node);
void gen_funcdef(Node *node);
void gen_funcall(Node *node);
void gen_binary(Node *node);

#endif