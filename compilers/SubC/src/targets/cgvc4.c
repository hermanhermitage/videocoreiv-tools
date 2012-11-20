/*
 *	NMH's Simple C Compiler, 2011,2012
 *	Videocoreiv target description (based on NMH's 386 target)
 */

#include "defs.h"
#include "data.h"
#include "decl.h"
#include "cgen.h"

/*
 * 386 -> videocore mapping
 * 
 * eax   r0
 * ecx   r1
 * edx   r2
 * tmp   r3
 * ebp   r23
 * esp   r25
 *
 *  g = global
 *  l = local
 *  s = static
 *
 *  b = byte
 *  w = word
 *  a = address
 *
 */

void cgdeclare(int id)  { lgenraw("%s(%c%d);", "declare", id); }
void cglab(int id)      { lgenraw("%s(%c%d);", "label", id); }
void cgname(char *name) { sgenraw("%s(C%s);", "\nlabel", name); }

void cgdata(void)	{ /*gen(".data;");*/ }
void cgtext(void)	{ /*gen(".text;");*/ }
void cgprelude(void)	{ }
void cgpostlude(void)	{ }
void cgpublic(char *s)	{ sgenraw("%s(%s);", "\nglobal", s); /*ngen(".globl\t%s", s, 0);*/ }
void cgstatic(char *s)	{ sgenraw("%s(%s);", "\nstatic", s); }


void cglit(int v)	{ ngen("%s(r0, %d);", "movi", v); } /* ngen("%s\t$%d,%%eax", "movl", v); } */
void cgclear(void)	{ ngen("%s(r0, %d);", "movi", 0); } /* gen("xorl\t%eax,%eax;"); } */

void cgldgb(char *s)	{ sgen("%s(r0, %s);", "ldb", s); } /* sgen("%s\t%s,%%al", "movb", s); } */
void cgldgw(char *s)	{ sgen("%s(r0, %s);", "ld", s); } /* sgen("%s\t%s,%%eax", "movl", s); } */
void cgldlb(int n)	{ ngen("%s(r0, %d, r23);", "ldb", n); } /* { ngen("%s\t%d(%%ebp),%%al", "movb", n); } */
void cgldlw(int n)	{ ngen("%s(r0, %d, r23);", "ld", n); } /* { ngen("%s\t%d(%%ebp),%%eax", "movl", n); } */
void cgldsb(int n)	{ lgen("%s(r0, %c%d);", "ldb", n); } /* lgen("%s\t%c%d,%%al", "movb", n); } */
void cgldsw(int n)	{ lgen("%s(r0, %c%d);", "ld", n); } /* lgen("%s\t%c%d,%%eax", "movl", n); } */

void cgldla(int n)	{ ngen("%s(r0, %d, r23);", "lea", n); } /* ngen("%s\t%d(%%ebp),%%eax", "leal", n); } */
void cgldsa(int n)	{ lgen("%s(r0, %c%d);", "ld", n); } /* lgen("%s\t$%c%d,%%eax", "movl", n); } */
void cgldga(char *s)	{ sgen("%s(r0, %s);", "ld", s); } /* sgen("%s\t$%s,%%eax", "movl", s); } */

void cgindb(void)	{ gen("ldb(r0, r0);"); } /* gen("movl\t%eax,%edx;"); cgclear(); gen("movb\t(%edx),%al;"); } */
void cgindw(void)	{ gen("ld(r0, r0);"); } /* gen("movl\t(%eax),%eax;"); } */
void cgargc(void)	{ gen("ld(r0, 8, r23);"); } /* gen("movl\t8(%ebp),%eax;"); } */

void cgldlab(int id)	{ lgen("%s(r0,%c%d);", "ld", id); } /* lgen("%s\t$%c%d,%%eax", "movl", id); } */

void cgpush(void)	{ gen("st(r0, --(r25));"); } /* movgen("pushl\t%eax;"); } */
void cgpushlit(int n)	{ ngen("%s(r0, %d);", "movi", n); cgpush(); } /* ngen("%s\t$%d", "pushl", n); } */
void cgpop2(void)	{ gen("ld(r1, (r25)++);"); } /* gen("popl\t%ecx;"); } */

void cgswap(void)	{ gen("xor(r0, r1); xor(r1, r0); xor(r0, r1);"); } /* r0, r1, r0^r1,gen("xchgl\t%eax,%ecx;"); } */

void cgand(void)	{ gen("and(r0, r1);"); } /* gen("andl\t%ecx,%eax;"); } */
void cgxor(void)	{ gen("xor(r0, r1);"); } /* gen("xorl\t%ecx,%eax;"); } */
void cgior(void)	{ gen("or(r0, r1);"); } /* gen("orl\t%ecx,%eax;"); } */
void cgadd(void)	{ gen("add(r0, r1);"); } /* gen("addl\t%ecx,%eax;"); } */
void cgmul(void)	{ gen("mul(r0, r1);"); } /* gen("imull\t%ecx,%eax;"); } */
void cgsub(void)	{ gen("sub(r0, r1);"); } /* gen("subl\t%ecx,%eax;"); } */
void cgdiv(void)	{ gen("divs(r0, r1);"); } /* gen("cdq;"); gen("idivl\t%ecx;"); } */
void cgmod(void)	{ gen("mods(r0, r1);"); } /* cgdiv(); gen("movl\t%edx,%eax;"); } */
void cgshl(void)	{ gen("lsl(r0, r1);"); } /* gen("shll\t%cl,%eax;"); } */
void cgshr(void)	{ gen("asr(r0, r1);"); } /* gen("sarl\t%cl,%eax;"); } */

void cgcmp(char *inst)	{ int lab; lab = label(); gen("movi(r2, 0);"); cgpop2(); gen("cmp(r1, r0);"); lgen("%s(%c%d);", inst, lab); gen("addi(r2, 1);"); genlab(lab); gen("mov(r0, r2);"); }
                        /* { int lab; lab = label(); gen("xorl\t%edx,%edx;"); cgpop2(); gen("cmpl\t%eax,%ecx;"); lgen("%s\t%c%d", inst, lab); gen("incl\t%edx;"); genlab(lab); gen("movl\t%edx,%eax;"); } */
void cgeq()		{ cgcmp("bne"); } /* cgcmp("jne;"); } */
void cgne()		{ cgcmp("beq"); } /* cgcmp("je;"); } */ 
void cglt()		{ cgcmp("bge"); } /* cgcmp("jge;"); } */
void cggt()		{ cgcmp("ble"); } /* cgcmp("jle;"); } */
void cgle()		{ cgcmp("bgt"); } /* cgcmp("jg;"); } */
void cgge()		{ cgcmp("blt"); } /* cgcmp("jl;"); } */

void cgneg(void)	{ gen("neg(r0, r0);;"); } /* gen("negl\t%eax;"); } */
void cgnot(void)	{ gen("not(r0, r0);;"); } /* gen("notl\t%eax;"); } */

void cglognot(void)	{ gen("and(r0, r0); moveq(r0, 1); movne(r0, 0);"); } /* gen("negl\t%eax;"); gen("sbbl\t%eax,%eax;"); gen("incl\t%eax;"); } */
void cgscale(void)	{ gen("lsr(r0, 2);"); } /* gen("shll\t$2,%eax;"); } */
void cgscale2(void)	{ gen("lsr(r1, 2);"); } /* gen("shll\t$2,%ecx;"); } */
void cgunscale(void)	{ gen("lsl(r0, 2);"); } /* gen("shrl\t$2,%eax;"); } */
void cgscaleby(int v)	{ ngen("%s(r0, %d);", "muli", v); } /* ngen("%s\t$%d,%%ecx", "movl", v); gen("mull\t%ecx;"); } */
void cgscale2by(int v)	{ ngen("%s(r1, %d);", "muli", v); } /* gen("pushl\t%eax;"); ngen("%s\t$%d,%%eax", "movl", v); gen("mull\t%ecx;"); gen("movl\t%eax,%ecx;"); gen("popl\t%eax;"); } */
void cgunscaleby(int v)	{ ngen("%s(r0, %d);", "divs", v); } /* ngen("%s\t$%d,%%ecx", "movl", v); gen("xorl\t%edx,%edx;"); gen("div\t%ecx;"); } */
void cgbool(void)	{ gen("and(r0, r0); movne(r0, 1);"); } /* gen("negl\t%eax;"); gen("sbbl\t%eax,%eax;"); gen("negl\t%eax;"); } */

void cgldinc(void)	{ gen("mov(r2, r0);"); } /* gen("movl\t%eax,%edx;"); } */

void cginc1pi(int v)	{ ngen("ld(r3, r0); %s(r3, %d); st(r3, r0);", "addi", v); } /* ngen("%s\t$%d,(%%eax);", "addl", v); } */
void cgdec1pi(int v)	{ ngen("ld(r3, r0); %s(r3, %d); st(r3, r0);", "subi", v); } /* ngen("%s\t$%d,(%%eax);", "subl", v); } */
void cginc2pi(int v)	{ ngen("ld(r3, r2); %s(r3, %d); st(r3, r2);", "addi", v); } /* ngen("%s\t$%d,(%%edx);", "addl", v); } */
void cgdec2pi(int v)	{ ngen("ld(r3, r2); %s(r3, %d); st(r3, r2);", "subi", v); } /* ngen("%s\t$%d,(%%edx);", "subl", v); } */

/* increment/decrement pointer to local, static, global */
void cgincpl(int a, int v)	{ ngen("%s(r3, %d, r23);", "ld", a); ngen("%s(r3,%d);", "addi", v); ngen("%s(r3, %d, r23);", "st", a); } /* ngen2("%s\t$%d,%d(%%ebp);", "addl", v, a); } */
void cgdecpl(int a, int v)	{ ngen("%s(r3, %d, r23);", "ld", a); ngen("%s(r3,%d);", "subi", v); ngen("%s(r3, %d, r23);", "st", a); } /* ngen2("%s\t$%d,%d(%%ebp);", "subl", v, a); } */
void cgincps(int a, int v)	{ lgen("%s(r3, %c%d);", "ld", a); ngen("%s(r3,%d);", "addi", v); lgen("%s(r3, %c%d);", "st", a); } /* lgen2("addl\t$%d,%c%d", v, a); } */
void cgdecps(int a, int v)	{ lgen("%s(r3, %c%d);", "ld", a); ngen("%s(r3,%d);", "subi", v); lgen("%s(r3, %c%d);", "st", a); } /* { lgen2("subl\t$%d,%c%d", v, a); } */
void cgincpg(char *s, int v)	{ sgen("%s(r3, %s);", "ld", s); ngen("%s(r3,%d);", "addi", v); sgen("%s(r3, %s);", "st", s); } /* sgen2("%s\t$%d,%s", "addl", v, s); } */
void cgdecpg(char *s, int v)	{ sgen("%s(r3, %s);", "ld", s); ngen("%s(r3,%d);", "subi", v); sgen("%s(r3, %s);", "st", s); } /* sgen2("%s\t$%d,%s", "subl", v, s); } */

void cginc1iw(void)	{ cginc1pi(1); } /* ngen("%s\t(%%eax);", "incl", 0); } */
void cgdec1iw(void)	{ cgdec1pi(1); } /* ngen("%s\t(%%eax);", "decl", 0); } */
void cginc2iw(void)	{ cginc2pi(1); } /* ngen("%s\t(%%edx);", "incl", 0); } */
void cgdec2iw(void)	{ cgdec2pi(1); } /* ngen("%s\t(%%edx);", "decl", 0); } */
void cginclw(int a)	{ cgincpl(a, 1); } /* ngen("%s\t%d(%%ebp);", "incl", a); } */
void cgdeclw(int a)	{ cgdecpl(a, 1); } /* ngen("%s\t%d(%%ebp);", "decl", a); } */
void cgincsw(int a)	{ cgincps(a, 1); } /* lgen("%s\t%c%d", "incl", a); } */
void cgdecsw(int a)	{ cgdecps(a, 1); } /* lgen("%s\t%c%d", "decl", a); } */
void cgincgw(char *s)	{ cgincpg(s, 1); } /* sgen("%s\t%s", "incl", s); } */
void cgdecgw(char *s)	{ cgdecpg(s, 1); } /* sgen("%s\t%s", "decl", s); } */

void cginc1ib(void)	{ ngen("ldb(r3, r0); %s(r3, %d); stb(r3, r0);", "addi", 1); } /* ngen("%s\t(%%eax);", "incb", 0); } */
void cgdec1ib(void)	{ ngen("ldb(r3, r0); %s(r3, %d); stb(r3, r0);", "subi", 1); } /* ngen("%s\t(%%eax);", "decb", 0); } */
void cginc2ib(void)	{ ngen("ldb(r3, r2); %s(r3, %d); stb(r3, r2);", "addi", 1); } /* ngen("%s\t(%%edx);", "incb", 0); } */
void cgdec2ib(void)	{ ngen("ldb(r3, r2); %s(r3, %d); stb(r3, r2);", "subi", 1); } /* ngen("%s\t(%%edx);", "decb", 0); } */
void cginclb(int a)	{ ngen("%s(r3, %d, r23);", "ldb", a); ngen("%s(r3, %d);", "addi", 1); ngen("%s(r3, %d, r23);", "stb", a); } /* ngen("%s\t%d(%%ebp);", "incb", a); } */
void cgdeclb(int a)	{ ngen("%s(r3, %d, r23);", "ldb", a); ngen("%s(r3, %d);", "subi", 1); ngen("%s(r3, %d, r23);", "stb", a); } /* ngen("%s\t%d(%%ebp);", "decb", a); } */
void cgincsb(int a)	{ ngen("%s(r3, %c%d);", "ldb", a); ngen("%s(r3, %d);", "addi", 1); ngen("%s(r3, %c%d);", "stb", a); } /* lgen("%s\t%c%d", "incb", a); } */
void cgdecsb(int a)	{ ngen("%s(r3, %c%d);", "stb", a); ngen("%s(r3, %d);", "subi", 1); ngen("%s(r3, %c%d);", "stb", a); } /* lgen("%s\t%c%d", "decb", a); } */
void cgincgb(char *s)	{ sgen("%s(r3, %s);", "ldb", s); ngen("%s(r3, %d);", "addi", 1); sgen("%s(r3, %s);", "stb", s); } /* sgen("%s\t%s", "incb", s); } */
void cgdecgb(char *s)	{ sgen("%s(r3, %s);", "ldb", s); ngen("%s(r3, %d);", "subi", 1); sgen("%s(r3, %s);", "stb", s); } /* sgen("%s\t%s", "decb", s); } */

void cgbr(char *how, int n)	{ int lab; lab = label(); gen("or(r0, r0);"); lgen("%s(%c%d);", how, lab); lgen("%s(%c%d);", "b", n); genlab(lab); }
                                /* { int lab; lab = label(); gen("orl\t%eax,%eax;"); lgen("%s\t%c%d", how, lab); lgen("%s\t%c%d", "jmp", n); genlab(lab); } */
void cgbrtrue(int n)	{ cgbr("beq", n); } /* cgbr("jz", n); } */
void cgbrfalse(int n)	{ cgbr("bne", n); } /* cgbr("jnz", n); } */
void cgjump(int n)	{ lgen("%s(%c%d);", "b", n); } /* lgen("%s\t%c%d", "jmp", n); } */
void cgldswtch(int n)	{ lgen("%s(r2, %c%d);", "movl", n); } /* lgen("%s\t$%c%d,%%edx", "movl", n); } */
void cgcalswtch(void)	{ gen("b(switch);"); } /* gen("jmp\tswitch;"); } */
void cgcase(int v, int l)	{ lgen2(".long\t%d,%c%d", v, l); }

void cgpopptr(void)	{ gen("ld(r2, (r25)++);"); } /* gen("popl\t%edx;"); } */
void cgstorib(void)	{ ngen("%s(r0, r2);", "stb", 0); } /* ngen("%s\t%%al,(%%edx);", "movb", 0); } */
void cgstoriw(void)	{ ngen("%s(r0, r2);", "st", 0); } /* ngen("%s\t%%eax,(%%edx);", "movl", 0); } */
void cgstorlb(int n)	{ ngen("%s(r0, %d, r23);", "stb", n); } /* ngen("%s\t%%al,%d(%%ebp);", "movb", n); } */
void cgstorlw(int n)	{ ngen("%s(r0, %d, r23);", "st", n); } /* \t%%eax,%d(%%ebp);", "movl", n); } */
void cgstorsb(int n)	{ lgen("%s(r0, %c%d);", "stb", n); } /* lgen("%s\t%%al,%c%d", "movb", n); } */
void cgstorsw(int n)	{ lgen("%s(r0, %c%d);", "st", n); } /* lgen("\t%%eax,%c%d", "movl", n); } */
void cgstorgb(char *s)	{ sgen("%s(r0, %s);", "stb", s); } /* sgen("%s\t%%al,%s", "movb", s); } */
void cgstorgw(char *s)	{ sgen("%s(r0, %s);", "st", s); } /* sgen("%s\t%%eax,%s", "movl", s); } */

void cginitlw(int v, int a)	{ ngen("%s(r3,%d);", "movi", v); ngen("%s(r3,%d,r23);", "st", v); } /* ngen2("%s\t$%d,%d(%%ebp);", "movl", v, a); } */
void cgcall(char *s)	{ sgen("%s(%s);", "bl", s); } /* sgen("%s\t%s", "call", s); } */
void cgcalr(void)	{ gen("bl_r(r0);"); } /* gen("call\t*%eax;"); } */
void cgstack(int n)	{ ngen("%s(r25, %d);", "addi", n); } /* ngen("%s\t$%d,%%esp", "addl", n); } */
void cgentry(void)	{ gen("st(r23, --(r25));"); gen("mov(r23, r25);"); } /* gen("pushl\t%ebp;"); gen("movl\t%esp,%ebp;"); } */
void cgexit(void)	{ gen("ld(r23, (r25)++);"); gen("rts();"); } /* gen("popl\t%ebp;"); gen("ret;"); } */

void cgdefb(int v)	{ ngen("%s(%d);", "dcb", v); /*ngen("%s\t%d", ".byte", v);*/ }
void cgdefw(int v)	{ ngen("%s(%d);", "dc", v); /*ngen("%s\t%d", ".long", v);*/ }
void cgdefp(int v)	{ ngen("%s(%d);", "dc", v); /*ngen("%s\t%d", ".long", v);*/ }
void cgdefl(int v)	{ lgen("%s(%c%d);", "dc", v); /*lgen("%s\t%c%d", ".long", v);*/ }
void cgdefc(int c)	{ ngen("%s('%c');", "dcb", c); /*ngen("%s\t'%c'", ".byte", c);*/ }
void cgbss(char *s, int z)	{ sgenraw("%s(%s);", "\nlabel", s); ngen("%s(%d, 0);", "fillb", z); /* ngen(".lcomm\t%s,%d", s, z);*/ }
