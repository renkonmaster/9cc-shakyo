#include "parse.h"
#include "util.h"

LVar *locals = NULL;
GVar *globals = NULL;
int functions_count = 0;
StringLiteral *string_literals = NULL;

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
        error(token->str);
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM)
        error(token->str);
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) 
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) 
            return var;
    return NULL;
}

GVar *find_gvar(Token *tok) {
    for (GVar *var = globals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

Type *basetype() {
    if (token->kind == TK_TYPE) {
        Type *type;
        if (token->len == 3 && !memcmp(token->str, "int", 3)) {
            type = int_type();
        } else if (token->len == 4 && !memcmp(token->str, "char", 4)) {
            type = calloc(1, sizeof(Type));
            type->ty = CHAR;
        } else {
            error(token->str);
        }
        token = token->next;
        return type;
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

Type *char_type() {
    Type *type = calloc(1, sizeof(Type));
    type->ty = CHAR;
    return type;
}

int size_of(Type *type) {
    switch (type->ty) {
        case INT:
            return 8;
        case PTR:
            return 8;
        case ARRAY:
            return type->array_size * size_of(type->ptr_to);
        case CHAR:
            return 1;
        default:
            error("Unknown type");
    }
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

Node *new_node_num(int val) {
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
        error(token->str);

    LVar *lvar = find_lvar(tok);
    if (lvar)
        error(tok->str);

    while (consume("[")) {
        int len = expect_number();
        expect("]");
        type = calloc(1, sizeof(Type));
        type->ty = ARRAY;
        type->array_size = len;
        type->ptr_to = base;
        base = type;
    }

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

Node *function_def(Type *ret_type, Token *tok) {
    locals = NULL;

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNCDEF;
    node->funcname = calloc(tok->len + 1, 1);
    memcpy(node->funcname, tok->str, tok->len);
    node->type = ret_type;

    Node **args = calloc(6, sizeof(Node*));
    int arg_count = 0;
    if (!consume(")")) {
        do {
            Type *base = basetype();
            Type *type = declarater(base);

            Token *arg_tok = consume_ident();
            if (!arg_tok) {
                error(token->str);
            }

            Node *arg = calloc(1, sizeof(Node));
            arg->kind = ND_LVAR;

            LVar *lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = arg_tok->str;
            lvar->len = arg_tok->len;
            lvar->type = type;
            
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

bool eval_const(Node *node, int *out) {
    if (!node) return false;
    int a, b;
    switch (node->kind) {
    case ND_NUM:
        *out = node->val;
        return true;
    case ND_ADD:
        if (eval_const(node->lhs, &a) && eval_const(node->rhs, &b)) {
            *out = a + b;
            return true;
        }
        return false;
    case ND_SUB:
        if (node->lhs && node->rhs){
            if (eval_const(node->lhs, &a) && eval_const(node->rhs, &b)) {
                *out = a - b;
                return true;
            }
        } else if (!node->lhs && node->rhs) {
            if (eval_const(node->rhs, &b)) {
                *out = -b;
                return true;
            }
        }
        return false;
    case ND_MUL:
        if (eval_const(node->lhs, &a) && eval_const(node->rhs, &b)) {
            *out = a * b;
            return true;
        }
        return false;
    case ND_DIV:
        if (eval_const(node->lhs, &a) && eval_const(node->rhs, &b)) {
            *out = a / b;
            return true;
        }
        return false;
    default:
        return false;
    }
}

void global_declaration(Type *base, Token *tok) {
    GVar *gvar = find_gvar(tok);
    if (gvar)
        error(tok->str);
    
    gvar = calloc(1, sizeof(GVar));
    gvar->name = calloc(tok->len + 1, 1);
    memcpy(gvar->name, tok->str, tok->len);
    gvar->type = base;
    gvar->next = globals;
    gvar->len = tok->len;
    globals = gvar;
    gvar->init = NULL;
    // グローバル変数の初期化に対応
    if (consume("=")) {
        Node *init_expr = expr();
        expect(";");

        int val;
        if (eval_const(init_expr, &val)) {
            Node *init_node = new_node(ND_NUM);
            init_node->val = val;
            init_node->type = int_type();
            gvar->init = init_node;
        } else if (init_expr->kind == ND_STRING) {
            gvar->init = init_expr;
        }
    } else {
        expect(";");
    }
}

void program() {
    while (!at_eof()) {
        Type *base = basetype();
        Type *type = declarater(base);

        Token *tok = consume_ident();
        if (!tok)
            error(token->str);
        
        if (consume("(")) {
            functions[functions_count++] = function_def(type, tok);
        } else {
            global_declaration(type, tok);
        }
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
                rhs = new_binary(ND_MUL, rhs, new_node_num(n));
            }
            node = new_binary(ND_ADD, node, rhs);
        } else if (consume("-")) {
            Node *rhs = mul();
            if (node->type && node->type->ty == PTR) {
                int n = node->type->ptr_to->ty == INT ? 4 : 8;
                rhs = new_binary(ND_MUL, rhs, new_node_num(n));
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

        
        LVar *lvar = find_lvar(tok);
        GVar *gvar = find_gvar(tok);
        if (lvar) {
            node->kind = ND_LVAR;
            node->offset = lvar->offset;
            node->type = lvar->type;
        } else if (gvar) {
            node->kind = ND_GVAR;
            node->type = gvar->type;
            node->gvar = gvar;
        } else {
            error(tok->str);
        }

        if (consume("[")) {
            while (!consume("]")) {
                if (!node->type || node->type->ty != ARRAY && node->type->ty != PTR) {
                    error(token->str);
                }
                Node *idx = expr();
                Node *ptr = new_binary(ND_ADD, node, idx);
                ptr->type = ptr_to(node->type->ptr_to);
                node = new_node(ND_DEREF);
                node->lhs = ptr;
                node->type = node->lhs->type->ptr_to;
            }
        }
        return node;
    }

    if (token->kind == TK_STR) {
        Token *str_tok = token;
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_STRING;
        StringLiteral *strlit = calloc(1, sizeof(StringLiteral));
        node->str = strlit;
        strlit->contents = calloc(str_tok->len + 1, 1);
        memcpy(strlit->contents, str_tok->str, str_tok->len);
        strlit->next = string_literals;
        string_literals = strlit;

        node->type = ptr_to(char_type());
        return node;
    }

    Node *node = new_node_num(expect_number());
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
        return array_to_ptr(unary());
    if (consume("-"))
        return new_binary(ND_SUB, new_node_num(0), array_to_ptr(unary()));
    if (consume("&")) {
        Node *node = new_node(ND_ADDR);
        node->lhs = unary();
        node->type = ptr_to(node->lhs->type);
        return node;
    }
    if (consume("*")) {
        Node *node = new_node(ND_DEREF);
        node->lhs = array_to_ptr(unary());
        if (node->lhs->type && node->lhs->type->ty == PTR) {
            node->type = node->lhs->type->ptr_to;
        } else {
            node->type = int_type();
        }
        return node;
    }
    Node *node = primary();
    return array_to_ptr(node);
}

Node *array_to_ptr(Node *node) {
    if (!node->type || node->type->ty != ARRAY) {
        return node;
    }
    node->type->ty = PTR;;
    return node;
}