#include "codegen.h"
#include "util.h"

static char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};  

void gen_lval(Node *node) {
    if (node->kind == ND_LVAR) {
        printf("  lea rax, [rbp-%d]\n", node->offset);
        printf("  push rax\n");
        return;
    }
    
    if (node->kind == ND_DEREF) {
        gen(node->lhs);
        return;
    }

    if (node->kind == ND_GVAR) {
        printf("  lea rax, %s[rip]\n", node->gvar->name);
        printf("  push rax\n");
        return;
    }

    error("Left value is not variable or dereference");
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
    for (int i = 0; i < node->arg_count; i++) {
        gen(node->args[i]);
    }

    for (int i = node->arg_count - 1; i >= 0; i--) {
        printf("  pop %s\n", argregs[i]);
    }

    int seq = count();
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n");
    printf("  jnz .L.call.%d\n", seq);
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  jmp .L.call.end.%d\n", seq);
    printf(".L.call.%d:\n", seq);
    printf("  sub rsp, 8\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  add rsp, 8\n");
    printf(".L.call.end.%d:\n", seq);
    printf("  push rax\n");
}

void gen_funcdef(Node *node){
    printf("%s:\n", node->funcname);
    printf("  push rbp\n");
    printf("  mov rbp , rsp\n");

    int stack_size = 0;
    for (LVar *lvar = node->locals; lvar; lvar = lvar->next) {
        int sz = size_of(lvar->type);
        int alloc = ((sz + 7) / 8) * 8;
        stack_size += alloc;
    }

    const int TEMP_STACK_SIZE = 128;
    int total = stack_size + TEMP_STACK_SIZE;
    int aligned = (total + 15) / 16 * 16;

    if (aligned > 0) 
        printf("  sub rsp, %d\n", aligned);

    for (int i = 0; i < node->arg_count; i++) {
        Node *arg = node->args[i];
        printf("  lea rax, [rbp-%d]\n", arg->offset);
        printf("  mov qword ptr [rax], %s\n", argregs[i]);
    }

    gen(node->body);

    /* 明示的なreturnがなくても安全に戻る*/
    // printf("  mov rsp, rbp\n");
    // printf("  pop rbp\n");
    // printf("  ret\n");
}

void gen(Node *node) {
    switch (node->kind)
    {
    case ND_FUNCDEF:
        gen_funcdef(node);
        return;
    case ND_BLOCK:
        gen_block(node);
        return;
    case ND_IF:
        gen_if(node);
        return;
    case ND_WHILE:
        gen_while(node);
        return;
    case ND_FOR:
        gen_for(node);
        return;
    case ND_RETURN:
        gen(node->rhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case ND_FUNCALL:
        gen_funcall(node);
        return;
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;    
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        if (node->type->ty == CHAR) {
            printf("  movsx rax, byte ptr [rax]\n");
        } else if (node->type->ty == INT) {
            printf("  movsxd rax, dword ptr [rax]\n");
        } else {
            printf("  mov rax, qword ptr [rax]\n");
        }
        printf("  push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        if (node->type->ty == CHAR) {
            printf("  mov byte ptr [rax], dil\n");
        } else if (node->type->ty == INT) {
            printf("  mov dword ptr [rax], edi\n");
        } else {
            printf("  mov qword ptr [rax], rdi\n");
        }
        printf("  push rdi\n");
        return;
    case ND_DEREF:
        gen(node->lhs);
        printf("  pop rax\n");
        if (node->type && node->type->ty == CHAR) {
            printf("  movsx rax, byte ptr [rax]\n");
        } else if (node->type && node->type->ty == INT) {
            printf("  movsxd rax, dword ptr [rax]\n");
        } else {
            printf("  mov rax, qword ptr [rax]\n");
        }
        printf("  push rax\n");
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
        return;
    case ND_GVAR:
        gen_lval(node);
        printf("  pop rax\n");
        if (node->gvar->type->ty == CHAR) {
            printf("  movsx rax, byte ptr [rax]\n");
        } else if (node->gvar->type->ty == INT) {
            printf("  movsxd rax, dword ptr [rax]\n");
        } else {
            printf("  mov rax, qword ptr [rax]\n");
        }
        printf("  push rax\n");  
        return;
    case ND_STRING:
        printf("  lea rax, .LC%d[rip]\n", node->str->id);
        printf("  push rax\n");  
        return;
    case ND_NOT:
        printf("  cmp rax, 0\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        return;
    }

    gen_binary(node);
}

void gen_binary(Node *node) {
    gen(node->lhs);
    
    // short-circuit evaluation
    if (node->kind == ND_AND) {
        int c = count();
        printf("  cmp rax, 0\n");
        printf("  je .Lfalse%d\n", c);
        gen(node->rhs);
        printf("  cmp rax, 0\n");
        printf("  je .Lfalse%d\n", c);
        printf("  mov rax, 1\n");
        printf("  jmp .Lend%d\n", c);
        printf(".Lfalse%d:\n", c);
        printf("  mov rax, 0\n");
        printf(".Lend%d:\n", c);
        return;
    } else if (node->kind == ND_OR) {
        int c = count();
        printf("  cmp rax, 0\n");
        printf("  jne .Ltrue%d\n", c);
        gen(node->rhs);
        printf("  cmp rax, 0\n");
        printf("  jne .Ltrue%d\n", c);
        printf("  mov rax, 0\n");
        printf("  jmp .Lend%d\n", c);
        printf(".Ltrue%d:\n", c);
        printf("  mov rax, 1\n");
        printf(".Lend%d:\n", c);
        return;
    }

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