#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>
#include <stdbool.h>

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;
typedef struct GVar GVar;
typedef struct StringLiteral StringLiteral;

typedef struct Type Type;
struct Type {
    enum { INT, PTR, ARRAY, CHAR } ty;
    struct Type *ptr_to;
    size_t array_size;
};

Type *basetype(void);
Type *declarater(Type *base);
Type *int_type(void);
Type *char_type(void);
Type *ptr_to(Type *base);
int size_of(Type *type);

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
    TK_STR,
} TokenKind;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
    Type *type;
};

struct GVar {
    GVar *next;
    char *name;
    int len;
    Type *type;
};

struct StringLiteral {
    StringLiteral *next;
    char *contents;
    int id;
};

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
    ND_STRING,
} NodeKind;

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
    StringLiteral *str;
};

#endif