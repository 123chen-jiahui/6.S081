#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int g(int x) {
  return x+3;
}

int f(int x) {
  return g(x);
}

void main(void) {
	int a = 1;
	char str[] = "hello world";
  printf("%d %d\n", f(8)+1, 13);
	printf("%d\n", a);
	printf("%p\n", &a);
	printf("%s\n", str);
  exit(0);
}
