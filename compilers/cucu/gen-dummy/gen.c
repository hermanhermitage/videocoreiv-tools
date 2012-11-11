#define emits(s) emit(s, strlen(s))

#define TYPE_NUM_SIZE 2
static int mem_pos = 0;

#define GEN_ADD   "pop B  \nA:=B+A \n"
#define GEN_ADDSZ strlen(GEN_ADD)

#define GEN_SUB   "pop B  \nA:=B-A \n"
#define GEN_SUBSZ strlen(GEN_SUB)

#define GEN_SHL   "pop B  \nA:=B<<A\n"
#define GEN_SHLSZ strlen(GEN_SHL)

#define GEN_SHR   "pop B  \nA:=B>>A\n"
#define GEN_SHRSZ strlen(GEN_SHR)

#define GEN_LESS  "pop B  \nA:=B<A \n"
#define GEN_LESSSZ strlen(GEN_LESS)

#define GEN_EQ "pop B  \nA:=B==A\n"
#define GEN_EQSZ strlen(GEN_EQ)
#define GEN_NEQ  "pop B  \nA:=B!=A\n"
#define GEN_NEQSZ strlen(GEN_NEQ)

#define GEN_OR "pop B  \nA:=B|A \n"
#define GEN_ORSZ strlen(GEN_OR)
#define GEN_AND  "pop B  \nA:=B&A \n"
#define GEN_ANDSZ strlen(GEN_AND)

#define GEN_ASSIGN "pop B  \nM[B]:=A\n"
#define GEN_ASSIGNSZ strlen(GEN_ASSIGN)
#define GEN_ASSIGN8 "pop B  \nm[B]:=A\n"
#define GEN_ASSIGN8SZ strlen(GEN_ASSIGN8)

#define GEN_JMP "jmp....\n"
#define GEN_JMPSZ strlen(GEN_JMP)

#define GEN_JZ "jmz....\n"
#define GEN_JZSZ strlen(GEN_JZ)

static void gen_start() {
	emits("jmpCAFE\n");
}

static void gen_finish() {
	struct sym *funcmain = sym_find("main");
	char s[32];
	sprintf(s, "%04x", funcmain->addr);
	memcpy(code+3, s, 4);
	printf("%s", code);
}

static void gen_ret() {
	emits("ret    \n");
	stack_pos = stack_pos - 1;
}

static void gen_const(int n) {
	char s[32];
	sprintf(s, "A:=%04x\n", n);
	emits(s);
}

static void gen_sym(struct sym *sym) {
	if (sym->type == 'G') {
		sym->addr = mem_pos;
		mem_pos = mem_pos + TYPE_NUM_SIZE;
	}
}

static void gen_loop_start() {}

static void gen_sym_addr(struct sym *sym) {
	gen_const(sym->addr);
}

static void gen_push() {
	emits("push A \n");
	stack_pos = stack_pos + 1;
}

static void gen_pop(int n) {
	char s[32];
	if (n > 0) {
		sprintf(s, "pop%04x\n", n);
		emits(s);
		stack_pos = stack_pos - n;
	}
}

static void gen_stack_addr(int addr) {
	char s[32];
	sprintf(s, "sp@%04x\n", addr);
	emits(s);
}

static void gen_unref(int type) {
	if (type == TYPE_INTVAR) {
		emits("A:=M[A]\n");
	} else if (type == TYPE_CHARVAR) {
		emits("A:=m[A]\n");
	}
}

static void gen_call() {
	emits("call A \n");
}

static void gen_array(char *array, int size) {
	int i = size;
	char *tok = array;
	/* put token on stack */
	for (; i >= 0; i-=2) {
		gen_const((tok[i] << 8 | tok[i-1]));
		gen_push();
	}
	/* put token address on stack */
	gen_stack_addr(0);
}


static void gen_patch(uint8_t *op, int value) {
	char s[32];
	sprintf(s, "%04x", value);
	memcpy(op-5, s, 4);
}

