#include "9cc.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

bool consume_kind(TokenKind kind) {
    if (token->kind != kind)
        return false;
    token = token->next;
    return true;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "expected \"%s\"", op);
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "Expected a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

LVar *locals = NULL;

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) 
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) 
            return var;
    return NULL;
}

Type *basetype() {
    if (token->kind == TK_TYPE) {
        token = token->next;
        return int_type();
    }

    error("Unknown type");
}

Type *declarater(Type *base) {
    while(consume("*")) {
        base = ptr_to(base);
    }
    return base;
}

Type *int_type() {
    Type *type = calloc(1, sizeof(Type));
    type->ty = INT;
    return type;
}

Type *ptr_to(Type *base) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = PTR;
    type->ptr_to = base;
    return type;
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    node->type = int_type();  // デフォルトはint型
    return node;
}

Node *new_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    node->type = int_type();
    return node;
}

void declaration() {
    Type *base = basetype();
    Type *type = declarater(base);

    Token *tok = consume_ident();
    if (!tok) 
        error_at(token->str, "Expected variable name");

    LVar *lvar = find_lvar(tok);
    if (lvar)
        error_at(tok->str, "Variable redeclaration");

    lvar = calloc(1, sizeof(LVar));
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = type;
    
    lvar->next = locals;

    if (locals)
        lvar->offset = locals->offset + 8;
    else
        lvar->offset = 8;
    locals = lvar;

    expect(";");
}

Node *function_def() {
    locals = NULL;
    if (token->kind != TK_TYPE) {
        error_at(token->str, "Expected type in function definition");
    }
    token = token->next;

    Token *tok = consume_ident();
    if (!tok) {
        error_at(token->str, "Expected function name");
    }
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNCDEF;
    node->funcname = calloc(tok->len + 1, 1);
    memcpy(node->funcname, tok->str, tok->len);

    expect("(");
    Node **args = calloc(6, sizeof(Node*));
    int arg_count = 0;
    if (!consume(")")) {
        do {
            if (token->kind != TK_TYPE) {
                error_at(token->str, "Expected type in function argument");
            }
            token = token->next;

            Token *arg_tok = consume_ident();
            if (!arg_tok) {
                error_at(token->str, "Expected argument name");
            }

            Node *arg = calloc(1, sizeof(Node));
            arg->kind = ND_LVAR;

            LVar *lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = arg_tok->str;
            lvar->len = arg_tok->len;
            lvar->type = int_type();  // デフォルトはint型
            
            if (locals) 
                lvar->offset = locals->offset + 8;
            else
                lvar->offset = 8;

            arg->offset = lvar->offset;
            arg->type = lvar->type;
            locals = lvar;

            args[arg_count++] = arg;
        }while (consume(","));
        expect(")");
    }
    node->args = args;
    node->arg_count = arg_count;

    node->body = stmt();
    node->locals = locals;

    return node;
}

void program() {
    int i = 0;
    while (!at_eof()) {
        functions[functions_count++] = function_def();
    }
}

Node *stmt() {
    Node *node;

    if(consume_kind(TK_IF)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if(consume_kind(TK_ELSE)) {
            node->els = stmt();
        }
        return node;
    }

    if(consume_kind(TK_WHILE)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    }

    if(consume_kind(TK_FOR)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        node->init = expr();
        expect(";");    
        node->cond = expr();
        expect(";");
        node->inc = expr();
        expect(")");
        node->body = stmt();
        return node;
    }

    if(consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;

        Node **stmts = calloc(100, sizeof(Node*));
        int i = 0;

        while(!consume("}")) {
            if (token->kind == TK_TYPE) {
                declaration();
                continue;
            }
            stmts[i++] = stmt();
        }

        node->block = stmts;
        node->block_size = i;
        return node;
    }

    if(consume_kind(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->rhs = expr();
    } else {
        node = expr();
    }

    expect(";");
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume("=")){
        node = new_binary(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();

    for(;;) {
        if (consume("==")) 
            node = new_binary(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_binary(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    for(;;) {
        if (consume("<")) 
            node = new_binary(ND_LT, node, add());
        else if (consume("<="))
            node = new_binary(ND_LE, node, add());
        else if (consume(">"))
            node = new_binary(ND_LT, add(), node);
        else if (consume(">="))
            node = new_binary(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    for(;;) {
        if (consume("+")) {
            Node *rhs = mul();
            if (node->type && node->type->ty == PTR) {
                int n = node->type->ptr_to->ty == INT ? 4 : 8;
                rhs = new_binary(ND_MUL, rhs, new_num(n));
            }
            node = new_binary(ND_ADD, node, rhs);
        } else if (consume("-")) {
            Node *rhs = mul();
            if (node->type && node->type->ty == PTR) {
                int n = node->type->ptr_to->ty == INT ? 4 : 8;
                rhs = new_binary(ND_MUL, rhs, new_num(n));
            }
            node = new_binary(ND_SUB, node, rhs);
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    for(;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if(consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));

        if (consume("(")) {
            node->kind = ND_FUNCALL;
            node->funcname = calloc(tok->len + 1, 1);
            memcpy(node->funcname, tok->str, tok->len);
            // 引数のパース
            Node **args = calloc(6, sizeof(Node*));
            int arg_count = 0;
            if (!consume(")")) {
                do {
                    args[arg_count++] = expr();
                } while (consume(","));
                expect(")");
            }
            node->args = args;
            node->arg_count = arg_count;
            return node;
        }

        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
            node->type = lvar->type;
        } else {
            error_at(tok->str, "Undefined variable");
        }
        return node;
    }

    Node *node = new_num(expect_number());
    node->type = int_type();
    return node;
}

Node *unary() {
    if (consume_kind(TK_SIZEOF)) {
        Node *node = unary();
        Node *size_node = new_node(ND_NUM);
        size_node->val = (node->type->ty == INT) ? 4 : 8;
        size_node->type = int_type();
        return size_node;
    }
    if (consume("+")) 
        return unary();
    if (consume("-"))
        return new_binary(ND_SUB, new_num(0), primary());
    if (consume("&")) {
        Node *node = new_node(ND_ADDR);
        node->lhs = unary();
        node->type = ptr_to(node->lhs->type);
        return node;
    }
    if (consume("*")) {
        Node *node = new_node(ND_DEREF);
        node->lhs = unary();
        if (node->lhs->type && node->lhs->type->ty == PTR) {
            node->type = node->lhs->type->ptr_to;
        } else {
            node->type = int_type();
        }
        return node;
    }
    return primary();
}
