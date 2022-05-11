#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "kernel/memlayout.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main()
{
	while (1) {
		uint64 a = (uint64) sbrk(4096);
		printf("%p\n", a);
		if (a == 0xfffffffffffffff)
			break;
	}
	return 0;
}
