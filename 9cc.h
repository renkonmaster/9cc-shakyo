#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_EOF,
    TK_TYPE,
    TK_SIZEOF,
} TokenKind;

typedef struct {
    char *str;
    int len;
    TokenKind kind;
} ReservedWord;

TokenKind check_reserved(char *name, int len);

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

typedef struct Type Type;

struct Type {
    enum { INT, PTR, ARRAY } ty;
    struct Type *ptr_to;
    size_t array_size;
};

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
    Type *type;
};

LVar *find_lvar(Token *tok);

typedef struct GVar GVar;

struct GVar {
    GVar *next;
    char *name;
    int len;
    Type *type;
};

GVar *find_gvar(Token *tok);

Type *basetype();
Type *declarater(Type *base);
Type *int_type();
Type *ptr_to(Type *base);
int size_of(Type *type);

void error(char *fmt, ...);

void error_at(char *loc, char *fmt, ...);

bool consume(char *op);

bool consume_kind(TokenKind kind);

Token *consume_ident();

void expect(char *op);

int expect_number();

bool at_eof();

Token *new_token(TokenKind kind, Token *cur, char *str, int len);

bool startswith(char *p, char *q);

Token *tokenize();

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_NUM,
    ND_ASSIGN,
    ND_LVAR,
    ND_GVAR,
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_FUNCALL,
    ND_FUNCDEF,
    ND_ADDR,
    ND_DEREF,
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;

    Node *cond;
    Node *then;
    Node *els;

    Node *init;
    Node *inc;
    Node *body;

    Node **block;
    int block_size;

    int val;
    int offset;

    char *funcname;
    Node **args;
    int arg_count;
    LVar *locals;
    GVar *gvar;
    Type *type;
};

Node *new_node(NodeKind kind);

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);

Node *new_num(int val);

void program();
Node *function_def(Type *ret_type, Token *tok);
Type *declarater(Type *base);
void declaration();
void global_declaration(Type *base, Token *tok);
Node *stmt();
Node *assign();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *array_to_ptr(Node *node);

void gen(Node *node);
void gen_lval(Node *node);
void gen_if(Node *node);
void gen_while(Node *node);
void gen_for(Node *node);
void gen_block(Node *node);
void gen_funcdef(Node *node);
void gen_funcall(Node *node);

void gen_binary(Node *node);

int count();

extern char *user_input;
extern Token *token;
extern Node *functions[100];
extern int functions_count;
extern GVar *globals;