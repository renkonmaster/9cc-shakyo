#include "9cc.h"

void dump_ast_simple(Node *node, int indent) {
    if (!node) {
        return;
    }

    // インデントを出力
    for (int i = 0; i < indent; i++) {
        printf("  "); // スペース2つでインデント
    }

    // ノードの種類と情報を出力
    switch (node->kind) {
        case ND_NUM:
            printf("NUM: %d\n", node->val);
            break;
        case ND_LVAR:
            printf("LVAR: offset=%d\n", node->offset);
            break;
        case ND_ASSIGN:
            printf("ASSIGN (=\n");
            break;
        case ND_ADD:
            printf("OP: ADD (+)\n");
            break;
        case ND_SUB:
            printf("OP: SUB (-)\n");
            break;
        case ND_MUL:
            printf("OP: MUL (*)\n");
            break;
        case ND_DIV:
            printf("OP: DIV (/)\n");
            break;
        default:
            // 他のノードはとりあえず種類だけ出力
            printf("OP: kind=%d\n", node->kind); 
    }

    // 左辺 (lhs) を再帰的に深く（DFS）
    if (node->lhs) {
        dump_ast_simple(node->lhs, indent + 1);
    }
    // 右辺 (rhs) を再帰的に深く（DFS）
    if (node->rhs) {
        dump_ast_simple(node->rhs, indent + 1);
    }
    
    // ND_ASSIGNのように、開始時に 'ASSIGN (' を出力したノードのために、閉じカッコを出力
    if (node->kind == ND_ASSIGN) {
        for (int i = 0; i < indent; i++) {
            printf("  ");
        }
        printf(")\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("%s: invalid number of arguments", argv[0]);
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Token *copyToken = token;
    //show tokenized token
    // while(copyToken->kind != TK_EOF) {
    //     printf("kind: %d, str: %.*s, val: %d\n", copyToken->kind, copyToken->len, copyToken->str, copyToken->val);  
    //     copyToken = copyToken->next;
    // }

    program();

    // printf("\n--- AST DUMP START ---\n");
    // for (int i = 0; code[i]; i++) {
    //     printf("Statement %d:\n", i + 1);
    //     // インデント0から開始
    //     dump_ast_simple(code[i], 0); 
    // }
    // printf("--- AST DUMP END ---\n\n");


    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  push rbp\n");
    printf("  mov rbp , rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        printf("  pop rax\n");
    }

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");

    return 0;
}