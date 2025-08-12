#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// ===== Token definitions =====
typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_END
} TokenType;

typedef struct {
    TokenType type;
    int value; // Only for numbers
} Token;

const char *input;
Token current_token;

// ===== Lexer =====
void next_token() {
    while (*input == ' ') input++; // skip spaces

    if (*input == '\0') {
        current_token.type = TOKEN_END;
        return;
    }

    if (isdigit(*input)) {
        current_token.type = TOKEN_NUMBER;
        current_token.value = strtol(input, (char**)&input, 10);
        return;
    }

    if (*input == '+') {
        current_token.type = TOKEN_PLUS;
        input++;
        return;
    }

    if (*input == '*') {
        current_token.type = TOKEN_STAR;
        input++;
        return;
    }

    printf("Unexpected character: %c\n", *input);
    exit(1);
}

// ===== AST definitions =====
typedef enum {
    AST_NUMBER,
    AST_ADD,
    AST_MUL
} ASTType;

typedef struct ASTNode {
    ASTType type;
    int value; // for numbers
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

ASTNode* new_number_node(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NUMBER;
    node->value = value;
    node->left = node->right = NULL;
    return node;
}

ASTNode* new_op_node(ASTType type, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = type;
    node->left = left;
    node->right = right;
    return node;
}

// ===== Parser =====
// Grammar:
// expr   = term ( "+" term )*
// term   = factor ( "*" factor )*
// factor = NUMBER

ASTNode* parse_factor() {
    if (current_token.type == TOKEN_NUMBER) {
        int value = current_token.value;
        next_token();
        return new_number_node(value);
    }
    printf("Expected number\n");
    exit(1);
}

ASTNode* parse_term() {
    ASTNode *node = parse_factor();
    while (current_token.type == TOKEN_STAR) {
        next_token();
        node = new_op_node(AST_MUL, node, parse_factor());
    }
    return node;
}

ASTNode* parse_expr() {
    ASTNode *node = parse_term();
    while (current_token.type == TOKEN_PLUS) {
        next_token();
        node = new_op_node(AST_ADD, node, parse_term());
    }
    return node;
}

// ===== AST printing =====
void print_ast(ASTNode *node, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    switch (node->type) {
        case AST_NUMBER:
            printf("%d\n", node->value);
            break;
        case AST_ADD:
            printf("+\n");
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case AST_MUL:
            printf("*\n");
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
    }
}

// ===== Main =====
int main() {
    input = "1 + 2 * 3 + 4 * 6";
    next_token(); // initialize lexer

    ASTNode *root = parse_expr();
    print_ast(root, 0);

    return 0;
}

