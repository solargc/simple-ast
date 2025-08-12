#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	TK_NUMBER,
	TK_PLUS,
	TK_MINUS,
	TK_STAR,
	TK_SLASH,
	TK_LPAREN,
	TK_RPAREN,
	TK_END,
} token_type;

typedef struct
{
    token_type type;
    long value;
} token;

typedef struct
{
    const char *cursor;
    token current;
} lexer;

typedef enum
{
	AST_NUMBER,
	AST_ADD,
	AST_SUB,
	AST_MUL,
	AST_DIV,
} ast_type; 

typedef struct ast_node
{
	ast_type type;
	long value;
	struct ast_node *left;
	struct ast_node *right;
} ast_node;

ast_node *new_number_node(long value)
{
	ast_node *node = malloc(sizeof(ast_node));
	if (!node)
		exit(1);
	node->type = AST_NUMBER;
	node->value = value;
	node->left = NULL;
	node->right = NULL;
	return node;
}

ast_node *new_op_node(ast_type type, ast_node *left, ast_node *right)
{
	ast_node *node = malloc(sizeof(ast_node));
	if (!node)
		exit(1);
	node->type = type;
	node->left = left;
	node->right = right;
	return node;
}

void next_token(lexer *lex)
{
	while (isspace((unsigned char)*lex->cursor))
		lex->cursor++;

	if (!*lex->cursor)
	{
		lex->current.type = TK_END;
		return;
	}

	if (isdigit((unsigned char)*lex->cursor))
	{
		char *end;
		lex->current.type = TK_NUMBER;
		lex->current.value = strtol(lex->cursor, &end, 10);
		lex->cursor = end;
		return;
	}

	if (*lex->cursor == '+')
	{
		lex->current.type = TK_PLUS;
		lex->cursor++;
		return;
	}

	if (*lex->cursor == '-')
	{
		lex->current.type = TK_MINUS;
		lex->cursor++;
		return;
	}

	if (*lex->cursor == '*')
	{
		lex->current.type = TK_STAR;
		lex->cursor++;
		return;
	}

	if (*lex->cursor == '/')
	{
		lex->current.type = TK_SLASH;
		lex->cursor++;
		return;
	}

	if (*lex->cursor == '(')
	{
		lex->current.type = TK_LPAREN;
		lex->cursor++;
		return;
	}

	if (*lex->cursor == ')')
	{
		lex->current.type = TK_RPAREN;
		lex->cursor++;
		return;
	}

  	printf("Unexpected char: %c\n", *lex->cursor);
	exit(1);
}



/*
GRAMMAR RULE:

expr   = term ( "+" term )*
term   = factor ( "*" factor )*
factor = NUMBER

WITH PARENTHESIS:

expr   = term ( ("+" | "-") term )*
term   = factor ( ("*" | "/") factor )*
factor = NUMBER | "(" expr ")"

WITH NEGATIVE NUMBERS:
factor = ("+" | "-") factor | NUMBER | "(" expr ")"

*/

ast_node *parse_expr(lexer *lex);  // fwd
ast_node *parse_term(lexer *lex);  // fwd

ast_node *parse_factor(lexer *lex)
{
	if (lex->current.type == TK_PLUS) {   // unary plus
		next_token(lex);
		return parse_factor(lex);
	}
	if (lex->current.type == TK_MINUS) {  // unary minus
		next_token(lex);
		// Represent as 0 - factor (or add AST_NEG if you prefer)
		return new_op_node(AST_SUB, new_number_node(0), parse_factor(lex));
	}
	if (lex->current.type == TK_NUMBER)
	{
		long value = lex->current.value;
		next_token(lex);
		return new_number_node(value);
	}
	if (lex->current.type == TK_LPAREN)
	{
		next_token(lex);
		ast_node *node = parse_expr(lex);
		if (lex->current.type != TK_RPAREN)
		{
			printf("Expected ')' at: %s\n", lex->cursor);
			exit(1);
		}
		next_token(lex);
		return node;
	}
	printf("Expected number \n");
	exit (1);
}

ast_node *parse_term(lexer *lex)
{
	ast_node *node = parse_factor(lex);
	while (lex->current.type == TK_STAR || lex->current.type == TK_SLASH)
	{
		if (lex->current.type == TK_STAR)
		{
			next_token(lex);
			node = new_op_node(AST_MUL, node, parse_factor(lex));
		}
		else
		{
			next_token(lex);
			node = new_op_node(AST_DIV, node, parse_factor(lex));
		}
	}
	return node;
}

ast_node *parse_expr(lexer *lex)
{
	ast_node *node = parse_term(lex);
	while (lex->current.type == TK_PLUS || lex->current.type == TK_MINUS)
	{
		if (lex->current.type == TK_PLUS)
		{
			next_token(lex);
			node = new_op_node(AST_ADD, node, parse_term(lex));
		}
		else
		{
			next_token(lex);
			node = new_op_node(AST_SUB, node, parse_term(lex));
		}
	}
	return node;
}

void print_ast(ast_node *node, int indent)
{
	int i = 0;
	while (i < indent)
	{
		printf("  ");
		i++;
	}
	if (node->type == AST_NUMBER)
		printf("%ld\n", node->value);

	else if (node->type == AST_ADD)
	{
		printf("+\n");
		print_ast(node->left, indent + 1);
		print_ast(node->right, indent + 1);
	}

	else if (node->type == AST_SUB)
	{
		printf("-\n");
		print_ast(node->left, indent + 1);
		print_ast(node->right, indent + 1);
	}

	else if (node->type == AST_MUL)
	{
		printf("*\n");
		print_ast(node->left, indent + 1);
		print_ast(node->right, indent + 1);
	}
}

#define COLOR_RESET  "\033[0m"
#define COLOR_NUM    "\033[1;32m"
#define COLOR_ADD    "\033[1;34m"
#define COLOR_SUB    "\033[1;93m"
#define COLOR_MUL    "\033[1;31m"
#define COLOR_DIV    "\033[1;96m"

void print_ast_pretty(ast_node *node, int indent) {
	if (!node) return;

	// Print right child first (above)
	print_ast_pretty(node->right, indent + 4);

	// Print current node
	for (int i = 0; i < indent; i++) printf(" ");

	if (node->type == AST_NUMBER)
		printf(COLOR_NUM "%ld" COLOR_RESET "\n", node->value);
	else if (node->type == AST_ADD)
		printf(COLOR_ADD "+" COLOR_RESET "\n");
	else if (node->type == AST_SUB)
		printf(COLOR_SUB "-" COLOR_RESET "\n");
	else if (node->type == AST_MUL)
		printf(COLOR_MUL "*" COLOR_RESET "\n");
	else if (node->type == AST_DIV)
		printf(COLOR_DIV "/" COLOR_RESET "\n");
	
	// Print left child (below)
	print_ast_pretty(node->left, indent + 4);
}

void destroy_ast(ast_node *n) {
	if (!n)
		return;
	destroy_ast(n->left);
	destroy_ast(n->right);
	free(n);
}

long eval(const ast_node *n) {
	if (!n) {
		fprintf(stderr, "null node\n");
		exit(1);
	}

	switch (n->type) {
	case AST_NUMBER:
		return n->value;

	case AST_ADD:
		return eval(n->left) + eval(n->right);

	case AST_SUB: /* if you still call it AST_MIN, keep that name here */
		return eval(n->left) - eval(n->right);

	case AST_MUL:
		return eval(n->left) * eval(n->right);

	case AST_DIV: {
		long rhs = eval(n->right);
		if (rhs == 0) {
			fprintf(stderr, "division by zero\n");
			exit(1);
		}
		return eval(n->left) / rhs; // integer division (trunc toward 0)
	}

	default:
		fprintf(stderr, "unknown node type\n");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s \"EXPR\"\n", argv [0]);
		return 1;
	}

	char *input = strdup(argv[1]);
	if (!input)
		return 1;

	lexer lex = {0};
	lex.cursor = input;
	next_token(&lex);

	ast_node *root = parse_expr(&lex);

	if (lex.current.type != TK_END)
	{
		printf("Trailing input at: %s\n", lex.cursor);
		destroy_ast(root);
		free(input);
		return 1;
	}

	print_ast_pretty(root, 0);

	long result = eval(root);
	printf("= %ld\n", result);
	
	destroy_ast(root);
	free(input);

	return 0;
}
