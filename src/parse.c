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
            type = char_type();
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
            return 4;
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

    switch (kind) {
    case ND_ASSIGN:
        if (lhs && lhs->type) node->type = lhs->type;
        break;
    case ND_ADD:
    case ND_SUB:
        if (lhs && lhs->type && lhs->type->ty == PTR) {
           node->type = lhs->type;
        } else if (rhs && rhs->type && rhs->type->ty == PTR) {
            node->type = rhs->type;
        } else {
            node->type = int_type();
        }
        break;
    case ND_MUL:
    case ND_DIV:
        node->type = int_type();
        break;
    default:
        node->type = int_type();
        break;
    }
    return node;
}

Node *new_node_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    node->type = int_type();
    return node;
}

Node *declaration() {
    Type *base = basetype();
    Type *type = declarater(base);

    Token *tok = consume_ident();
    if (!tok)
        error(token->str);

    LVar *lvar = find_lvar(tok);
    if (lvar)
        error(tok->str);

    if (consume("[")) {
        int len = expect_number();
        expect("]");
        Type *t = calloc(1, sizeof(Type));
        t->ty = ARRAY;
        t->array_size = len;
        t->ptr_to = base;
        type = t;
    }

    lvar = calloc(1, sizeof(LVar));
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = type;
    
    int sz = size_of(type);
    int alloc = (sz + 7) / 8 * 8;
    lvar->next = locals;
    lvar->offset = locals ? locals->offset + alloc : alloc;
    locals = lvar;

    if (consume("=")) {
        Node *lhs = new_node(ND_LVAR);
        lhs->offset = lvar->offset;
        lhs->type = lvar->type;
        Node *node = new_binary(ND_ASSIGN, lhs, assign());
        expect(";");
        return node;
    }

    expect(";");
    return NULL;
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
            
            int sz = size_of(type);
            int alloc = (sz + 7) / 8 * 8;
            lvar->offset = locals ? locals->offset + alloc : alloc;

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

    // Array Type
    if (consume("[")) {
        int array_size = 0;
        if (!consume("]")) {
            Node *size_expr = expr();
            if (!eval_const(size_expr, &array_size)) {
                error("Array size must be constant expression");
            }
            expect("]");
        }
        Type *t = calloc(1, sizeof(Type));
        t->ty = ARRAY;
        t->array_size = array_size;
        t->ptr_to = base;
        gvar->type = t;
    }
    
    globals = gvar;
    gvar->init = NULL;

    // グローバル変数の初期化に対応
    if (consume("=")) {
        // String Literal
        if (token->kind == TK_STR) {
            Token *str_tok = token;
            token = token->next;
            expect(";");

            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_STRING;
            StringLiteral *strlit = calloc(1, sizeof(StringLiteral));
            strlit->contents = calloc(str_tok->len + 1, 1);
            memcpy(strlit->contents, str_tok->str, str_tok->len);
            strlit->next = string_literals;
            string_literals = strlit;

            node->str = strlit;
            gvar->init = node;
            return;
        }

        //Array Initializer
        if (gvar->type->ty == ARRAY) {
            if (!consume("{")) {
                error("Expected '{' for array initializer");
            }

            int index = 0;
            Node **elems = calloc(gvar->type->array_size, sizeof(Node*));
            while(!consume("}")) {
                if (index >= gvar->type->array_size) {
                    error("Too many initializers for array");
                }

                Node *init_expr = expr();
                if (init_expr->kind == ND_STRING || init_expr->kind == ND_ADDR) {
                    elems[index++] = init_expr;
                }else {
                    int val;
                    if (!eval_const(init_expr, &val)) {
                        error("Array initializer must be constant expression");
                    }
                    Node *n = new_node(ND_NUM);
                    n->val = val;
                    n->type = int_type();
                    elems[index++] = n;
                }

                consume(",");
            }

            Node *init_node = calloc(1, sizeof(Node));
            init_node->kind = ND_BLOCK;
            init_node->block = elems;
            init_node->block_size = gvar->type->array_size;
            gvar->init = init_node;
            expect(";");
            return;
        }

        // const expr or global address
        Node *init_expr = expr();
        expect(";");

        int val;
        if (eval_const(init_expr, &val)) {
            Node *init_node = new_node(ND_NUM);
            init_node->val = val;
            init_node->type = int_type();
            gvar->init = init_node;
        } else if (init_expr->kind == ND_ADDR || init_expr->kind == ND_STRING) {
            gvar->init = init_expr;
        } else {
            error("Invalid global initializer");
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
                Node *node = declaration();
                if (node)
                    stmts[i++] = node;
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
            // Ptr + Int
            if (node->type->ty == PTR && rhs->type->ty == INT) {
                rhs = new_binary(ND_MUL, rhs, new_node_num(size_of(node->type->ptr_to)));
                node = new_binary(ND_ADD, node, rhs);
                continue;
            }

            // Int + Ptr
            if (node->type->ty == INT && rhs->type->ty == PTR) {
                node = new_binary(ND_ADD, rhs, node);
                node->type = rhs->type;
                continue;
            }

            node = new_binary(ND_ADD, node, rhs);
            node->type = int_type();
            continue;
        } else if (consume("-")) {
            Node *rhs = mul();
            // Ptr - Int
            if (node->type->ty == PTR && rhs->type->ty == INT) {
                rhs = new_binary(ND_MUL, rhs, new_node_num(size_of(node->type->ptr_to)));
                node = new_binary(ND_SUB, node, rhs);
                continue;
            }

            // Ptr - Ptr
            if (node->type->ty == PTR && rhs->type->ty == PTR){
                node = new_binary(ND_SUB, node, rhs);
                node->type = int_type();
                continue;
            }
            node = new_binary(ND_SUB, node, rhs);
            node->type = int_type();
            continue;
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
            //返り値をint型とする
            node->type = int_type();
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
            if (!node->type || node->type->ty != ARRAY && node->type->ty != PTR) {
                error(token->str);
            }
            Node *idx = expr();
            expect("]");

            Node *base_addr = new_node(ND_ADDR);
            base_addr->lhs = node;
            base_addr->type = ptr_to(node->type->ty == ARRAY ? node->type->ptr_to : node->type);

            Node *mul = new_binary(ND_MUL, idx, new_node_num(size_of(node->type->ptr_to)));
            Node *ptr = new_binary(ND_ADD, base_addr, mul);
            ptr->type = ptr_to(node->type->ptr_to);
            node = new_node(ND_DEREF);
            node->lhs = ptr;
            node->type = node->lhs->type->ptr_to;
            return node;
        }

        if (node->type->ty == ARRAY) {
            Node *addr = new_node(ND_ADDR);
            addr->lhs = node;
            addr->type = ptr_to(node->type->ptr_to);
            node = addr;
            return node;
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
        size_node->val = size_of(node->type);
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
    node->type = ptr_to(node->type->ptr_to);
    return node;
}