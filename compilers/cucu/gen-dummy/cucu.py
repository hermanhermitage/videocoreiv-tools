import subprocess

import time

#
# Interpret VM instruction
#
class CucuVM:
	CUCU_PATH='./cucu-dummy'
	def __init__(self, src, debug=False):
		self.A = 0
		self.B = 0
		self.PC = 0
		self.SP = 16
		self.mem = [0 for i in range(0, 16)]
		self.compile(src.encode('ascii'))
		self.debug = debug
		if debug:
			print(self.code)
		while (self.PC < len(self.code)):
			self.step()
			if debug:
				self.dump()

	def compile(self, src):
		p = subprocess.Popen(self.CUCU_PATH, stdout=subprocess.PIPE, stdin=subprocess.PIPE)
		self.code = p.communicate(input=src)[0]

	def getint(self, addr):
		return self.mem[addr] + self.mem[addr+1] * 256
	def putint(self, addr, n):
		self.mem[addr] = n & 0xff
		self.mem[addr+1] = (n & 0xff00) >> 8

	def step(self):
		op = (self.code[self.PC:self.PC+8]).decode('ascii')
		if (self.debug):
			print("op", op)
		self.PC = self.PC + 8
		if (op.startswith(';')):
			return
		elif (op.startswith('ret')):
			try:
				addr = self.getint(self.SP)
				#addr = self.mem[self.SP+1]*256 + self.mem[self.SP]
				self.SP = self.SP + 2
				self.PC = addr
			except IndexError:
				self.PC = 0xffffff
		elif (op.startswith('A:=m[A]')):
			self.A = self.mem[self.A]
		elif (op.startswith('A:=M[A]')):
			self.A = self.getint(self.A)
				#self.A = self.mem[self.A] + self.mem[self.A + 1] * 256
		elif (op.startswith('m[B]:=A')):
			self.mem[self.B] = self.A & 0xff
		elif (op.startswith('M[B]:=A')):
			self.putint(self.B, self.A)
			#self.mem[self.B] = self.A & 0xff
			#self.mem[self.B+1] = (self.A & 0xff00) >> 8
		elif (op.startswith('push A')):
			self.SP = self.SP - 2
			self.putint(self.SP, self.A)
			#self.mem[self.SP] = self.A & 0xff
			#self.mem[self.SP+1] = (self.A & 0xff00) >> 8
		elif (op.startswith('pop B')):
			self.B = self.getint(self.SP)
			#self.B = self.mem[self.SP+1]*256 + self.mem[self.SP]
			self.SP = self.SP + 2
		elif (op.startswith('A:=B+A')):
			self.A = (self.B + self.A) & 0xffff
		elif (op.startswith('A:=B-A')):
			self.A = (self.B - self.A) & 0xffff
		elif (op.startswith('A:=B&A')):
			self.A = (self.B & self.A) & 0xffff
		elif (op.startswith('A:=B<<A')):
			self.A = (self.B << self.A) & 0xffff
		elif (op.startswith('A:=B>>A')):
			self.A = (self.B >> self.A) & 0xffff
		elif (op.startswith('A:=B|A')):
			self.A = (self.B | self.A) & 0xffff
		elif (op.startswith('A:=B<A')):
			if self.A > self.B:
				self.A = 1
			else:
				self.A = 0
		elif (op.startswith('A:=B==A')):
			if self.A == self.B:
				self.A = 1
			else:
				self.A = 0
		elif (op.startswith('A:=B!=A')):
			if self.A != self.B:
				self.A = 1
			else:
				self.A = 0
		elif (op.startswith('pop')):
			n = int(op[3:], 16)
			self.SP = self.SP + n*2
		elif (op.startswith('A:=')): 
			self.A = int(op[3:], 16)
		elif (op.startswith('sp@')): 
			self.A = self.SP + int(op[3:], 16)*2 # TODO
		elif (op.startswith('jmp')):
			self.PC = int(op[3:], 16)
		elif (op.startswith('jmz')):
			if self.A == 0:
				self.PC = int(op[3:], 16)
		elif (op.startswith('call A')):
			self.SP = self.SP - 2
			self.putint(self.SP, self.PC)
			#self.mem[self.SP] = (self.PC & 0xff)
			#self.mem[self.SP+1] = ((self.PC & 0xff00) >> 8)
			self.PC = self.A
		else:
			print("UNKNOWN OPERATOR STRING: " + op)

	def dump(self):
		print("A:%04x  B:%04x   PC:%x  SP:%x" % (self.A, self.B, self.PC, self.SP))
		print("Mem:", self.mem)
		print()


