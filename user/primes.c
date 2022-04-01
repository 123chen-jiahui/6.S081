#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ_END 0
#define WRITE_END 1

void func(int fd, int base)
{
	int p[2];
	pipe(p);
	int pid = fork();
	if (pid < 0){
		fprintf(2, "fork failed\n");
		exit(1);
	} else if (pid == 0){
		close(p[WRITE_END]);
		int t;
		int written = read(p[READ_END], &t, sizeof(int));
		if (written){
			printf("prime %d\n", t);
			func(p[READ_END], t);
		}
		close(p[READ_END]);
	} else{
		int t;
		int res = 1;
		while ((res = read(fd, &t, sizeof(int))) != 0){
			if (t % base){
				close(p[READ_END]);
				write(p[WRITE_END], &t, sizeof(int));
			}
		}
		close(p[WRITE_END]);
		wait((int *)0);
	}
}

int main(int argc, char *argv[])
{
	int p[2];
	pipe(p);

	int pid = fork();
	if (pid < 0){
		fprintf(2, "fork failed\n");
		exit(1);
	} else if (pid == 0){
		close(p[WRITE_END]);
		int t;
		int written = read(p[READ_END], &t, sizeof(int));
		if (written){
			printf("prime %d\n", t);
			func(p[READ_END], t);
		}
		close(p[READ_END]);
	} else { //parent process
		close(p[READ_END]);
		for (int i = 2; i <= 35; i ++)
			write(p[WRITE_END], &i, sizeof(int));
		close(p[WRITE_END]);
		wait((int *)0);
	}
	exit(1);
}
