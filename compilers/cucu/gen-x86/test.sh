CUCUCC="./cucu-x86"

function testcucu() {
	retval=$1
	f=`mktemp`
	echo $2 > $f
	$CUCUCC < $f > $f.S
	if [ "x$3" != "x" ]; then cat $f.S ; fi
	gcc -m32 -s $f.S -o $f.elf
	$f.elf
	testval=$?
	if [ $retval -ne $testval ]; then
		echo -n "E$retval?$testval"
		exit 0
	else
		echo -n "."
	fi
	rm $f
	rm $f.S
	rm $f.elf
}

# Simple return values
testcucu 0 'int main() { return 0; }'
testcucu 5 'int main() { return 5; }'
testcucu 7 'int main() { return 5+2; }'
testcucu 3 'int main() { return 5-2; }'
testcucu 12 'int main() { return 3 << 2; }'
testcucu 4 'int main() { return 9 >> 1; }'
testcucu 3 'int main() { return 1 | 2; }'
testcucu 1 'int main() { return 5 & 3; }'
testcucu 0 'int main() { return 1 == 2; }'
testcucu 1 'int main() { return 2 == 2; }'
testcucu 1 'int main() { return 1 + 3 == 2 + 2; }'
testcucu 1 'int main() { return 1+3 != 1+2; }'
testcucu 1 'int main() { return 1 < 2; }'
testcucu 0 'int main() { return 2 < 2; }'
# Locals
testcucu 7 "int main() { int i; i = 7; return i; }"
#testcucu 1000 "int main() { int i; i = 1000; return i; }" # fails because of exit status
testcucu 5 "int main() { int i; int j; i = 5; j = 7; return i; }"
testcucu 5 "int main() { int i; int j; i = 5; j = i; i = 3; return j; }"
testcucu 3 "int main() { int i; int j; i = 5; j = i; j = 3; return j; }"
testcucu 5 "int main() { int i; int j; i = 5; j = i; j = 3; return i; }"
testcucu 8 "int main() { int i; int j = 5; i = 3; int k = j+i; return k; }"
testcucu 15 "int main() { int i; int j; int k = j = i = 5; return k + j + i; }"
testcucu 10 "int main() { int i; int j; int k = j = i = 5; return k + j; }"
testcucu 5  "int main() { int i; int j; int k = j = i = 5; return k; }"
# Globals
testcucu 7 "int i; int main() { i = 7; return i; }"
testcucu 5 "int i; int j; int main() { i = 5; j = 7; return i; }"
testcucu 7 "int i; int j; int main() { i = 5; j = 7; return j; }"
# Arrays
testcucu 0  "int main() { char *s = \"\x00\x00\"; return 0; }"
testcucu 5  "int main() { char *s = \"\x05\x07\"; return s[0]; }"
testcucu 7  "int main() { char *s = \"\x05\x07\"; return s[1]; }"
testcucu 3  "int main() { char *s = \"\x05\x07\"; s[0] = 3; return s[0]; }"
testcucu 8  "int main() { char *s = \"\x00\x00\"; s[0]=3; s[1]=5; return s[0]+s[1]; }"
testcucu 3 "int main() { char *s = \"\x00\x00\x00\x00\"; s[0]=257; s[2]=258; return s[0]+s[1]+s[2]+s[3]; }"
testcucu 6 "char *s; int main() { s=\"\x00\x00\"; s[0]=5; s[1]=257; return s[0]+s[1];}"
# Functions
testcucu 0 "int f() { } int main() { f(); return 0; }"
testcucu 8 "int f() { return 8; } int main() { int i; i = 3; return f(); }"
testcucu 8 "int f() { int j; j = 8; return j; } int main() { int i; i = 3; return f(); }"
testcucu 18 "int f1() { int j = 8; return j; } int f2() { return 7; } int main() { int i; i = 3; return i+f1()+f2(); }"
testcucu 3 "int f1() { int i = 8; } int f2() { int i; i = 7; } int main() { int i; i = 3; f1(); f2(); return i; }"
testcucu 7 "int add(int x,int y){return x+y;} int main() { return add(3,4); }"
# Branches
testcucu 3 "int main() { if (1) { return 3; } return 5; }"
testcucu 3 "int main() { if (1) { return 3; } else { return 5; }}"
testcucu 5 "int main(){if (0) { return 3;} else {return 5;}}"
testcucu 3 "int main(){ if (4) { if (3-3) return 2; else return 3;} else {return 5;}}"
testcucu 2 "int main(){ int i; i = 3; if (i) i=2; else return i;return i;}"
# Loops
testcucu 3 "int main() { while (0) return 5; return 3; }"
testcucu 5 "int main() { while (1) return 5; return 3; }"
testcucu 5 "int main() { int i = 3; while (i != 5) i = i + 1; return i; }"
testcucu 17 "int main() { int i;int j; i=j=3; while (i != 5) { j = 0; while (j < 10) j=j+3; i=i+1;} return i+j; }"

