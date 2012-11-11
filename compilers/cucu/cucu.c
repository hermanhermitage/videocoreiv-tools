#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/* print fatal error message and exit */
static void error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

/*
 * LEXER
 */
#define MAXTOKSZ 256
static FILE *f;            /* input source file */
static char tok[MAXTOKSZ]; /* current token */
static int tokpos;         /* offset inside the current token */
static int nextc;          /* next char to be pushed into token */

/* read next char */
void readchr() {
	if (tokpos == MAXTOKSZ - 1) {
		tok[tokpos] = '\0';
		error("Token too long: %s\n", tok);
	}
	tok[tokpos++] = nextc;
	nextc = fgetc(f);
}

/* read single token */
void readtok() {
	for (;;) {
		/* skip spaces */
		while (isspace(nextc)) {
			nextc = fgetc(f);
		}
		/* try to read a literal token */
		tokpos = 0;
		while (isalnum(nextc) || nextc == '_') {
			readchr();
		}
		/* if it's not a literal token */
		if (tokpos == 0) {
			while (nextc == '<' || nextc == '=' || nextc == '>'
					|| nextc == '!' || nextc == '&' || nextc == '|') {
				readchr();
			}
		}
		/* if it's not special chars that looks like an operator */
		if (tokpos == 0) {
			/* try strings and chars inside quotes */
			if (nextc == '\'' || nextc == '"') {
				char c = nextc;
				readchr();
				while (nextc != c) {
					readchr();
				}
				readchr();
			} else if (nextc == '/') { /* skip comments */
				readchr();
				if (nextc == '*') {
					nextc = fgetc(f);
					while (nextc != '/') {
						while (nextc != '*') {
							nextc = fgetc(f);
						}
						nextc = fgetc(f);
					}
					nextc = fgetc(f);
					continue;
				}
			} else if (nextc != EOF) {
				/* otherwise it looks like a single-char symbol, like '+', '-' etc */
				readchr();
			}
		}
		break;
	}
	tok[tokpos] = '\0';
}

/* check if the current token machtes the string */
int peek(char *s) {
	return (strcmp(tok, s) == 0);
}

/* read the next token if the current token machtes the string */
int accept(char *s) {
	if (peek(s)) {
		readtok();
		return 1;
	}
	return 0;
}

/* throw fatal error if the current token doesn't match the string */
void expect(char *s) {
	if (accept(s) == 0) {
		error("Error: expected '%s', but found: %s\n", s, tok);
	}
}

/*
 * SYMBOLS
 */
#define MAXSYMBOLS 4096
static struct sym {
	char type;
	int addr;
	char name[MAXTOKSZ];
} sym[MAXSYMBOLS];
static int sympos = 0;

int stack_pos = 0;

static struct sym *sym_find(char *s) {
	int i;
	struct sym *symbol = NULL;
	for (i = 0; i < sympos; i++) {
		if (strcmp(sym[i].name, s) == 0) {
			symbol = &sym[i];
		}
	}
	return symbol;
}

static struct sym *sym_declare(char *name, char type, int addr) {
	strncpy(sym[sympos].name, name, MAXTOKSZ);
	sym[sympos].addr = addr;
	sym[sympos].type = type;
	sympos++;
	if (sympos > MAXSYMBOLS) {
		error("Too many symbols\n");
	}
	return &sym[sympos-1];
}

/*
 * BACKEND
 */
#define MAXCODESZ 4096
static char code[MAXCODESZ];
static int codepos = 0;

static void emit(void *buf, size_t len) {
	memcpy(code + codepos, buf, len);
	codepos += len;
}

#define TYPE_NUM  0
#define TYPE_CHARVAR 1
#define TYPE_INTVAR  2

#ifndef GEN
#error "A code generator (backend) must be provided (use -DGEN=...)"
#endif

#include GEN

/*
 * PARSER AND COMPILER
 */

static int expr();

/* read type name: int, char and pointers are supported */
static int typename() {
	if (peek("int") || peek("char")) {
		readtok();
		while (accept("*"));
		return 1;
	}
	return 0;
}

static int prim_expr() {
	int type = TYPE_NUM;
	if (isdigit(tok[0])) {
		int n = strtol(tok, NULL, 10); /* TODO: parse 0x.. */
		gen_const(n);
	} else if (isalpha(tok[0])) {
		struct sym *s = sym_find(tok);
		if (s == NULL) {
			error("Undeclared symbol: %s\n", tok);
		}
		if (s->type == 'L') {
			gen_stack_addr(stack_pos - s->addr - 1);
		} else {
			gen_sym_addr(s);
		}
		type = TYPE_INTVAR;
	} else if (accept("(")) {
		type = expr();
		expect(")");
	} else if (tok[0] == '"') {
		int i, j;
		i = 0; j = 1;
		while (tok[j] != '"') {
			if (tok[j] == '\\' && tok[j+1] == 'x') {
				char s[3] = {tok[j+2], tok[j+3], 0};
				uint8_t n = strtol(s, NULL, 16);
				tok[i++] = n;
				j += 4;
			} else {
				tok[i++] = tok[j++];
			}
		}
		tok[i] = 0;
		if (i % 2 == 0) {
			i++;
			tok[i] = 0;
		}
		gen_array(tok, i);
		type = TYPE_NUM;
	} else {
		error("Unexpected primary expression: %s\n", tok);
	}
	readtok();
	return type;
}

static int binary(int type, int (*f)(), char *buf, size_t len) {
	if (type != TYPE_NUM) {
		gen_unref(type);
	}
	gen_push();
	type = f();
	if (type != TYPE_NUM) {
		gen_unref(type);
	}
	emit(buf, len);
	stack_pos = stack_pos - 1; /* assume that buffer contains a "pop" */
	return TYPE_NUM;
}

static int postfix_expr() {
	int type = prim_expr();
	if (type == TYPE_INTVAR && accept("[")) {
		binary(type, expr, GEN_ADD, GEN_ADDSZ);
		expect("]");
		type = TYPE_CHARVAR;
	} else if (accept("(")) {
		int prev_stack_pos = stack_pos;
		gen_push(); /* store function address */
		int call_addr = stack_pos - 1;
		if (accept(")") == 0) {
			expr();
			gen_push();
			while (accept(",")) {
				expr();
				gen_push();
			}
			expect(")");
		}
		type = TYPE_NUM;
		gen_stack_addr(stack_pos - call_addr - 1);
		gen_unref(TYPE_INTVAR);
		gen_call();
		/* remove function address and args */
		gen_pop(stack_pos - prev_stack_pos);
		stack_pos = prev_stack_pos;
	}
	return type;
}

static int add_expr() {
	int type = postfix_expr();
	while (peek("+") || peek("-")) {
		if (accept("+")) {
			type = binary(type, postfix_expr, GEN_ADD, GEN_ADDSZ);
		} else if (accept("-")) {
			type = binary(type, postfix_expr, GEN_SUB, GEN_SUBSZ);
		}
	}
	return type;
}

static int shift_expr() {
	int type = add_expr();
	while (peek("<<") || peek(">>")) {
		if (accept("<<")) {
			type = binary(type, add_expr, GEN_SHL, GEN_SHLSZ);
		} else if (accept(">>")) {
			type = binary(type, add_expr, GEN_SHR, GEN_SHRSZ);
		}
	}
	return type;
}

static int rel_expr() {
	int type = shift_expr();
	while (peek("<")) {
		if (accept("<")) {
			type = binary(type, shift_expr, GEN_LESS, GEN_LESSSZ);
		}
	}
	return type;
}

static int eq_expr() {
	int type = rel_expr();
	while (peek("==") || peek("!=")) {
		if (accept("==")) {
			type = binary(type, rel_expr, GEN_EQ, GEN_EQSZ);
		} else if (accept("!=")) {
			type = binary(type, rel_expr, GEN_NEQ, GEN_NEQSZ);
		}
	}
	return type;
}

static int bitwise_expr() {
	int type = eq_expr();
	while (peek("|") || peek("&")) {
		if (accept("|")) {
			type = binary(type, eq_expr, GEN_OR, GEN_ORSZ);
		} else if (accept("&")) {
			type = binary(type, eq_expr, GEN_AND, GEN_ANDSZ);
		}
	}
	return type;
}

static int expr() {
	int type = bitwise_expr();
	if (type != TYPE_NUM) {
		if (accept("=")) {
			gen_push(); expr(); 
			if (type == TYPE_INTVAR) {
				emit(GEN_ASSIGN, GEN_ASSIGNSZ);
			} else {
				emit(GEN_ASSIGN8, GEN_ASSIGN8SZ);
			}
			stack_pos = stack_pos - 1; /* assume ASSIGN contains pop */
			type = TYPE_NUM;
		} else {
			gen_unref(type);
		}
	}
	return type;
}

static void statement() {
	if (accept("{")) {
		int prev_stack_pos = stack_pos;
		while (accept("}") == 0) {
			statement();
		}
		gen_pop(stack_pos-prev_stack_pos);
		stack_pos = prev_stack_pos;
	} else if (typename()) {
		struct sym *var = sym_declare(tok, 'L', stack_pos);
		readtok();
		if (accept("=")) {
			expr();
		}
		gen_push(); /* make room for new local variable */
		var->addr = stack_pos-1;
		expect(";");
	} else if (accept("if")) {
		expect("(");
		expr();
		emit(GEN_JZ, GEN_JZSZ);
		int p1 = codepos;
		expect(")");
		int prev_stack_pos = stack_pos;
		statement();
		emit(GEN_JMP, GEN_JMPSZ);
		int p2 = codepos;
		gen_patch(code + p1, codepos);
		if (accept("else")) {
			stack_pos = prev_stack_pos;
			statement();
		}
		stack_pos = prev_stack_pos;
		gen_patch(code + p2, codepos);
	} else if (accept("while")) {
		expect("(");
		int p1 = codepos;
		gen_loop_start();
		expr();
		emit(GEN_JZ, GEN_JZSZ);
		int p2 = codepos;
		expect(")");
		statement();
		emit(GEN_JMP, GEN_JMPSZ);
		gen_patch(code + codepos, p1);
		gen_patch(code + p2, codepos);
	} else if (accept("return")) {
		if (peek(";") == 0) {
			expr();
		}
		expect(";");
		gen_pop(stack_pos); /* remove all locals from stack (except return address) */
		gen_ret();
	} else {
		expr();
		expect(";");
	}
}

static void compile() {
	while (tok[0] != 0) { /* until EOF */
		if (typename() == 0) {
			error("Error: type name expected\n");
		}
		struct sym *var = sym_declare(tok, 'U', 0);
		readtok();
		if (accept(";")) {
			var->type = 'G';
			gen_sym(var);
			continue;
		}
		expect("(");
		int argc = 0;
		for (;;) {
			argc++;
			if (typename() == 0) {
				break;
			}
			sym_declare(tok, 'L', -argc-1);
			readtok();
			if (peek(")")) {
				break;
			}
			expect(",");
		}
		expect(")");
		if (accept(";") == 0) {
			stack_pos = 0;
			var->addr = codepos;
			var->type = 'F';
			gen_sym(var);
			statement(); /* function body */
			gen_ret(); /* another ret if user forgets to put 'return' */
		}
	}
}

int main(int argc, char *argv[]) {
	f = stdin;
	/* prefetch first char and first token */
	nextc = fgetc(f);
	readtok();
	gen_start();
	compile();
	gen_finish();
	return 0;
}

