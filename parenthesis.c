
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_NUMBER,
    TK_PLUS,
    TK_STAR,
    TK_LPAREN,   // NEW: '('
    TK_RPAREN,   // NEW: ')'
    TK_END,
} token_type;

typedef struct {
    token_type type;
    int value;
} token;

typedef struct {
    const char *cursor;
    token current;
} lexer;

typedef enum {
    AST_NUMBER,
    AST_ADD,
    AST_MUL
} ast_type;

typedef struct ast_node {
    ast_type type;
    int value;
    struct ast_node *left;
    struct ast_node *right;
} ast_node;

static ast_node *new_number_node(int value) {
    ast_node *node = (ast_node*)malloc(sizeof(ast_node));
    if (!node) { perror("malloc"); exit(1); }
    node->type = AST_NUMBER;
    node->value = value;
    node->left = node->right = NULL;
    return node;
}

static ast_node *new_op_node(ast_type type, ast_node *left, ast_node *right) {
    ast_node *node = (ast_node*)malloc(sizeof(ast_node));
    if (!node) { perror("malloc"); exit(1); }
    node->type = type;
    node->left = left;
    node->right = right;
    return node;
}

static void next_token(lexer *lex) {
    while (*lex->cursor == ' ')
        lex->cursor++;

    if (*lex->cursor == '\0') {
        lex->current.type = TK_END;
        return;
    }

    if (isdigit((unsigned char)*lex->cursor)) {
        char *end;
        long v = strtol(lex->cursor, &end, 10);
        lex->current.type  = TK_NUMBER;
        lex->current.value = (int)v;
        lex->cursor = end;
        return;
    }

    if (*lex->cursor == '+') {
        lex->current.type = TK_PLUS;
        lex->cursor++;
        return;
    }

    if (*lex->cursor == '*') {
        lex->current.type = TK_STAR;
        lex->cursor++;
        return;
    }

    if (*lex->cursor == '(') {            // NEW
        lex->current.type = TK_LPAREN;
        lex->cursor++;
        return;
    }

    if (*lex->cursor == ')') {            // NEW
        lex->current.type = TK_RPAREN;
        lex->cursor++;
        return;
    }

    printf("Unexpected char: %c\n", *lex->cursor);
    exit(1);
}

/*
Grammar with parentheses:

expr   = term   ( "+" term   )*
term   = factor ( "*" factor )*
factor = NUMBER | "(" expr ")"
*/

static ast_node *parse_expr(lexer *lex);  // fwd
static ast_node *parse_term(lexer *lex);  // fwd

static ast_node *parse_factor(lexer *lex) {
    if (lex->current.type == TK_NUMBER) {
        int value = lex->current.value;
        next_token(lex);
        return new_number_node(value);
    }
    if (lex->current.type == TK_LPAREN) {         // NEW: "(" expr ")"
        next_token(lex);                           // consume '('
        ast_node *node = parse_expr(lex);         // parse inside
        if (lex->current.type != TK_RPAREN) {
            fprintf(stderr, "Expected ')' at: %s\n", lex->cursor);
            exit(1);
        }
        next_token(lex);                           // consume ')'
        return node;
    }
    printf("Expected number or '(' expr ')' \n");
    exit(1);
}

static ast_node *parse_term(lexer *lex) {
    ast_node *node = parse_factor(lex);
    while (lex->current.type == TK_STAR) {
        next_token(lex);
        node = new_op_node(AST_MUL, node, parse_factor(lex));
    }
    return node;
}

static ast_node *parse_expr(lexer *lex) {
    ast_node *node = parse_term(lex);
    while (lex->current.type == TK_PLUS) {
        next_token(lex);
        node = new_op_node(AST_ADD, node, parse_term(lex));
    }
    return node;
}

static void print_ast(ast_node *node, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    if (node->type == AST_NUMBER) {
        printf("%d\n", node->value);
    } else if (node->type == AST_ADD) {
        printf("+\n");
        print_ast(node->left, indent + 1);
        print_ast(node->right, indent + 1);
    } else if (node->type == AST_MUL) {
        printf("*\n");
        print_ast(node->left, indent + 1);
        print_ast(node->right, indent + 1);
    }
}

static void free_ast(ast_node *n) {
    if (!n) return;
    free_ast(n->left);
    free_ast(n->right);
    free(n);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"EXPR\"\n", argv[0]);
        return 1;
    }
    char *input = strdup(argv[1]);
    if (!input) { perror("strdup"); return 1; }

    lexer lex = {0};
    lex.cursor = input;
    next_token(&lex);

    ast_node *root = parse_expr(&lex);

    if (lex.current.type != TK_END) {
        fprintf(stderr, "Trailing input at: %s\n", lex.cursor);
        free_ast(root);
        free(input);
        return 1;
    }

    print_ast(root, 0);

    free_ast(root);
    free(input);
    return 0;
}
