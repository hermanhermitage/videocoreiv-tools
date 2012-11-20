struct test_struct {
  int x;
  int y;
  char *z;
};

int test(struct test_struct *st) {
  int r;
  r = st->x + st->y + st->z[0];
  return r;
}

int hmm(int x, int y) {
  return x*x+y*y;
}

int func(int x, int y) {
  int tmp[1024];
  return x+y+hmm(tmp[10], tmp[20]);
}
