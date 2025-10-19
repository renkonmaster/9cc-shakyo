#include "9cc.h"
static char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};  

int count(void) {
    static int c = 0;
    return c++;
}

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("Left value is not variable");
    }
    
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen_if(Node *node) {
    int c = count();
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if(node->els) {
        printf("  je  .Lelse%d\n", c);
        gen(node->then);
        printf("  jmp .Lend%d\n", c);
        printf(".Lelse%d:\n", c);
        gen(node->els);
        printf(".Lend%d:\n", c);
    }else{
        printf("  je  .Lend%d\n", c);
        gen(node->then);
        printf(".Lend%d:\n", c);
    }
}

void gen_while(Node *node) {
    int c = count();
    printf(".Lbegin%d:\n", c);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", c);
    gen(node->then);
    printf("  jmp  .Lbegin%d\n", c);
    printf(".Lend%d:\n", c);
}

void gen_for(Node *node) {
    int c = count();
    gen(node->init);
    printf(".Lbegin%d:\n", c);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", c);
    gen(node->body);
    gen(node->inc);
    printf("  jmp .Lbegin%d\n", c);
    printf(".Lend%d:\n", c);
}

void gen_block(Node *node) {
    for (int i = 0; i < node->block_size; i++) {
        gen(node->block[i]);
    }
}

void gen_funcall(Node *node) {
    for (int i = node->arg_count - 1; i >= 0; i--) {
        gen(node->args[i]);
    }

    for (int i = 0; i < node->arg_count; i++) {
        printf("  pop %s\n", argregs[i]);
    }

    printf("  mov r10, rsp\n"); 
    printf("  and r10, 15\n");
    printf("  sub rsp, r10\n");
    printf("  call %s\n", node->funcname);
    printf("  add rsp, r10\n");
    printf("  push rax\n");
}

void gen_funcdef(Node *node){
    printf("%s:\n", node->funcname);
    printf("  push rbp\n");
    printf("  mov rbp , rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; i < node->arg_count; i++) {
        Node *arg = node->args[i];
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", arg->offset);
        printf("  mov [rax], %s\n", argregs[i]);
    }

    gen(node->body);

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

void gen(Node *node) {
    if (node->kind == ND_FUNCDEF) {
        gen_funcdef(node);
        return;
    }

    if (node->kind == ND_BLOCK) {
        gen_block(node);
        return;
    }

    if (node->kind == ND_IF) {
        gen_if(node);
        return;
    }

    if (node->kind == ND_WHILE) {
        gen_while(node);
        return;
    }

    if (node->kind == ND_FOR) {
        gen_for(node);
        return;
    }

    if (node->kind == ND_RETURN) {
        gen(node->rhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    }

    if (node->kind == ND_FUNCALL) {
        gen_funcall(node);
        return;
    }

    switch (node->kind) {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;    
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");  
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    }
    printf("  push rax\n");
}