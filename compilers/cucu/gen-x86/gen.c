/* some helper macros to emit text */
#define emits(s) emit(s, strlen(s))
#define emitf(fmt, ...) \
	do { \
		char buf[128]; \
		snprintf(buf, sizeof(buf)-1, fmt, __VA_ARGS__); \
		emits(buf); \
	} while (0)

#define TYPE_NUM_SIZE 4

#define GEN_ADD   "pop %ebx\nadd %ebx, %eax\n"
#define GEN_ADDSZ strlen(GEN_ADD)

#define GEN_SUB   "pop %ebx\nsub %ebx, %eax\nneg %eax\n"
#define GEN_SUBSZ strlen(GEN_SUB)

#define GEN_SHL   "pop %ebx\nmov %al, %cl\nshl %cl, %ebx\nmov %ebx, %eax\n"
#define GEN_SHLSZ strlen(GEN_SHL)

#define GEN_SHR   "pop %ebx\nmov %al, %cl\nshr %cl, %ebx\nmov %ebx, %eax\n"
#define GEN_SHRSZ strlen(GEN_SHR)

#define GEN_LESS  "pop %ebx\ncmp %eax, %ebx\nsetl %al\nmovzx %al, %eax\n"
#define GEN_LESSSZ strlen(GEN_LESS)

#define GEN_EQ "pop %ebx\ncmp %ebx, %eax\nsete %al\nmovzx %al, %eax\n"
#define GEN_EQSZ strlen(GEN_EQ)
#define GEN_NEQ  "pop %ebx\ncmp %ebx, %eax\nsetne %al\nmovzx %al, %eax\n"
#define GEN_NEQSZ strlen(GEN_NEQ)

#define GEN_OR "pop %ebx\nor %ebx, %eax \n"
#define GEN_ORSZ strlen(GEN_OR)
#define GEN_AND  "pop %ebx\nand %ebx, %eax \n"
#define GEN_ANDSZ strlen(GEN_AND)

#define GEN_ASSIGN "pop %ebx\nmovl %eax, (%ebx)\n"
#define GEN_ASSIGNSZ strlen(GEN_ASSIGN)
#define GEN_ASSIGN8 "pop %ebx\nmovb %al, (%ebx)\n"
#define GEN_ASSIGN8SZ strlen(GEN_ASSIGN8)

#define GEN_JMP "jmp                \n"
#define GEN_JMPSZ strlen(GEN_JMP)

#define GEN_JZ "cmp $0, %eax\nje                  \n"
#define GEN_JZSZ strlen(GEN_JZ)

static void gen_start() {
	emits(".align 4\n.globl main\n");
}

static void gen_finish() {
	int i;
	printf("%s", code);
	printf(".data\n");
	for (i = 0; i < sympos; i++) {
		if (sym[i].type == 'G') {
			printf("%s:\n.long 0\n", sym[i].name);
		}
	}
}

/* put constant to primary register */
static void gen_const(int n) {
	emitf("mov $0x%x, %%eax\n", n);
}

static void gen_push() {
	emits("push %eax\n");
	stack_pos = stack_pos + 1;
}

static void gen_pop(int n) {
	if (n > 0) {
		emitf("add $0x%04x, %%esp\n", n * TYPE_NUM_SIZE);
		stack_pos = stack_pos - n;
	}
}

static void gen_stack_addr(int addr) {
	emitf("mov %%esp, %%eax\nadd $0x%x, %%eax\n", addr*TYPE_NUM_SIZE);
}

static void gen_unref(int type) {
	if (type == TYPE_INTVAR) {
		emits("mov (%eax), %eax\n");
	} else if (type == TYPE_CHARVAR) {
		emits("movb (%eax), %al\n"); /* TODO fill ah with 0? */
	}
}

/* Call function by address stored in primary register */
/* no, call doesn't increase current stack size?????!!! XXX  */
static void gen_call() {
	emits("call *%eax\n");
}

/* return from function (return address is stored on the stack) */
static void gen_ret() {
	emits("ret\n");
	stack_pos = stack_pos - 1;
}

static void gen_sym(struct sym *sym) {
	if (sym->type == 'F') {
		emits(sym->name);
		emits(":\n");
	}
}

static void gen_loop_start() {
	emitf("___ifelse%04x:\n", codepos);
}

static void gen_sym_addr(struct sym *sym) {
	emitf("mov $%s, %%eax\n", sym->name);
}

static int array_index = 0;
static void gen_array(char *array, int size) {
	int i;
	emitf(".data\n___s%d:\n.string \"", array_index);
	for (i = 0; i < size; i++) {
		emitf("\\x%02x", array[i]);
	}
	emits("\"\n.text\n");
	emitf("mov $___s%d, %%eax\n", array_index);
	array_index++;
}

/* patch jump address */
static void gen_patch(uint8_t *op, int value) {
	char s[32];
	sprintf(s, "___ifelse%04x", value);
	if (value >= codepos) {
		emits(s);
		emits(":\n");
	}
	memcpy(op-strlen(s)-1, s, strlen(s));
}

