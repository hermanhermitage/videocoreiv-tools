import unittest
from cucu import CucuVM

#
#
#
class TestLocals(unittest.TestCase):

	def test_return_local_int(self):
		c = CucuVM("int main() { int i; i = 7; return i; }")
		self.assertEquals(c.A, 7)

	def test_return_local_int_big(self):
		c = CucuVM("int main() { int i; i = 1000; return i; }")
		self.assertEquals(c.A, 1000)

	def test_return_first_local(self):
		c = CucuVM("int main() { int i; int j; i = 5; j = 7; return i; }")
		self.assertEquals(c.A, 5)

	def test_assign_local(self):
		c = CucuVM("int main() { int i; int j; i = 5; j = i; i = 3; return j; }")
		self.assertEquals(c.A, 5)
		c = CucuVM("int main() { int i; int j; j = 5; i = j; j = 3; return j; }")
		self.assertEquals(c.A, 3)
		c = CucuVM("int main() { int i; int j; j = 5; i = j; j = 3; return i; }")
		self.assertEquals(c.A, 5)

	def test_assign_local_intermixed(self):
		c = CucuVM("int main() { int i; int j = 5; i = 3; int k = j+i; return k; }")
		self.assertEquals(c.A, 8)

	def test_multiple_assignment(self):
		c = CucuVM("int main() { int i; int j; int k = j = i = 5; return k + j + i; }")
		self.assertEquals(c.A, 15)
		c = CucuVM("int main() { int i; int j; int k = j = i = 5; return k + j; }")
		self.assertEquals(c.A, 10)
		c = CucuVM("int main() { int i; int j; int k = j = i = 5; return k; }")
		self.assertEquals(c.A, 5)

#
#
#
class TestGlobals(unittest.TestCase):

	def test_return_global_int(self):
		c = CucuVM("int i; int main() { i = 7; return i; }")
		self.assertEquals(c.A, 7)
		self.assertEquals(c.mem[0], 7)

	def test_return_global_int_big(self):
		c = CucuVM("int i; int main() { i = 1000; return i; }")
		self.assertEquals(c.A, 1000) # 1000 dec = 0x3e8 hex
		self.assertEquals(c.mem[0], 0xe8)
		self.assertEquals(c.mem[1], 0x3)

	def test_return_first_global(self):
		c = CucuVM("int i; int j; int main() { i = 5; j = 7; return i; }")
		self.assertEquals(c.A, 5)
		self.assertEquals(c.mem[0], 5)
		self.assertEquals(c.mem[2], 7)

	def test_return_second_global(self):
		c = CucuVM("int i; int j; int main() { i = 5; j = 7; return j; }")
		self.assertEquals(c.A, 7)
		self.assertEquals(c.mem[0], 5)
		self.assertEquals(c.mem[2], 7)

#
#
#
class TestArrays(unittest.TestCase):
	def test_local_array(self):
		c = CucuVM("int main() { char *s = 1; s[0]=3; s[1]=5; return s[0]+s[1]; }")
		self.assertEquals(c.A, 8)
	def test_local_array_defined(self):
		c = CucuVM("int main() {char *s=\"\\x65\\x67\"; return s[0]+s[1];}")
		self.assertEquals(c.A, 0x65+0x67)
	def test_local_arrays(self):
		c = CucuVM("""int main() {
				char *s1=\"\\x65\\x67\"; 
				char *s2 = \"\\x01\\x02\";
				return s1[1]+s2[0];
		}""")
		self.assertEquals(c.A, 0x68)
	def test_local_array_math(self):
		c = CucuVM("""int main() { 
				char *s = 0; 
				s[0]=257; s[2]=258; 
				return s[0]+s[1]+s[2]+s[3];
		}""")
		self.assertEquals(c.A, 3)
	def test_global_array(self):
		c = CucuVM("char *s; int main() { s=3; s[0]=5; s[1]=257; return s[0]+s[1];}");
		self.assertEquals(c.A, 6)

#
#
#
class TestFunctions(unittest.TestCase):
	def test_noarg_noreturn_func(self):
		c = CucuVM("int f() { } int main() { f(); }")
		# no asserts here

	def test_noarg_return_func(self):
		c = CucuVM("int f() { return 8; } int main() { int i; i = 3; return f(); }")
		self.assertEquals(c.A, 8)

	def test_noarg_return_func_with_locals(self):
		c = CucuVM("int f() { int j; j = 8; return j; } int main() { int i; i = 3; return f(); }")
		self.assertEquals(c.A, 8)

	def test_multiple_functions(self):
		c = CucuVM("""
		int f1() { 
			int j = 8;
			return j; 
		}
		int f2() {
			return 7;
		}
		int main() {
			int i; i = 3; return i+f1()+f2(); 
		}
		""")
		self.assertEquals(c.A, 18)

	def test_multiple_functions_variable_name_collisions(self):
		c = CucuVM("""
		int f1() { 
			int i = 8;
		}
		int f2() {
			int i;
			i = 7;
		}
		int main() {
			int i; 
			i = 3;
			f1();
			f2();
			return i;
		}
		""")
		self.assertEquals(c.A, 3)
	
	def test_func_with_dummy_args(self):
		c = CucuVM("int add(int x, int y) { } int main() { add(3, 4); }")
		# no asserts here

	def test_func_with_args(self):
		c = CucuVM("int add(int x,int y){return x+y;} int main() { return add(3,4); }")
		self.assertEquals(c.A, 7)

#
#
#
class TestBranches(unittest.TestCase):
	def test_if_true(self):
		c = CucuVM("int main(){if (1) { return 3;} return 5;}")
		self.assertEquals(c.A, 3)
		c = CucuVM("int main(){if (1) { return 3;} else {return 5;}}")
		self.assertEquals(c.A, 3)
	def test_if_false(self):
		c = CucuVM("int main(){if (0) { return 3;} else {return 5;}}")
		self.assertEquals(c.A, 5)
	def test_nested_if(self):
		c = CucuVM("int main(){ if (4) { if (3-3) return 2; else return 3;}"+
				"else {return 5;}}")
		self.assertEquals(c.A, 3)
	def test_if_return(self):
		c = CucuVM("int main(){ int i; i = 3; if (i) i=2; else return i;return i;}")
		self.assertEquals(c.A, 2)

#
#
#
class TestLoops(unittest.TestCase):
	def test_zero_loop(self):
		c = CucuVM("int main() { while (0) return 5; return 3; }")
		self.assertEquals(c.A, 3)
		c = CucuVM("int main() { while (1) return 5; return 3; }")
		self.assertEquals(c.A, 5)
	def test_loop_with_counter(self):
		c = CucuVM("int main() { int i = 3; while (i != 5) i = i + 1; return i; }")
		self.assertEquals(c.A, 5)
	def test_nested_loops(self):
		c = CucuVM("int main() { int i;int j; i=j=3; "+
				"while (i != 5) { j = 0; while (j < 10) j=j+3; i=i+1;} return i+j; }")
		self.assertEquals(c.A, 17)

#
#
#
class TestReturn(unittest.TestCase):
	def test_return_const(self):
		c = CucuVM("int main() { return 5; }")
		self.assertEquals(c.A, 5)
	def test_return_void(self):
		c = CucuVM("int main() { return; }")
	def test_return_early(self):
		c = CucuVM("""
		int main() { 
			int i = 3;
			{
				int j = 4;
				{
					int k = 5;
				}
				return i;
			}
			return i; 
		}
		""")
		self.assertEquals(c.A, 3)

#
#
#
class TestMath(unittest.TestCase):
	def test_add(self):
		c = CucuVM("int main() { return 2 + 9; }");
		self.assertEquals(c.A, 2+9);
		c = CucuVM("int main() { return 1 + 2 + 3; }");
		self.assertEquals(c.A, 1+2+3);
	def test_subtract(self):
		c = CucuVM("int main() { return 9 - 2; }");
		self.assertEquals(c.A, 9-2);
		c = CucuVM("int main() { return 9 - 4 - 1; }");
		self.assertEquals(c.A, 9-4-1);
	def test_add_subtract(self):
		c = CucuVM("int main() { return 1 + 2 - 3 - 4 + 10; }")
		self.assertEquals(c.A, 1 + 2 - 3 - 4 + 10);
	def test_shift(self):
		c = CucuVM("int main() { return 3 << 2;}")
		self.assertEquals(c.A, 12)
		c = CucuVM("int main() { return 9 >> 1; }")
		self.assertEquals(c.A, 4)
	def test_bitwise(self):
		c = CucuVM("int main() { return 1 | 2;}")
		self.assertEquals(c.A, 3)
		c = CucuVM("int main() { return 5 & 3;}")
		self.assertEquals(c.A, 1)
	def test_equals(self):
		c = CucuVM("int main() { return 1 == 2;}")
		self.assertEquals(c.A, 0)
		c = CucuVM("int main() { return 2 == 2;}")
		self.assertEquals(c.A, 1)
		c = CucuVM("int main() { return 1+3 == 2+2;}")
		self.assertEquals(c.A, 1)
		c = CucuVM("int main() { return 1+3 != 1+2;}")
		self.assertEquals(c.A, 1)
	def test_compare(self):
		c = CucuVM("int main() { return 1 < 2;}")
		self.assertEquals(c.A, 1)
		c = CucuVM("int main() { return 2 < 2;}")
		self.assertEquals(c.A, 0)


if __name__ == '__main__':
	unittest.main()
