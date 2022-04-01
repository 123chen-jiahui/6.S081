#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(2, "too fewer arguments\n");
		exit(1);
	}

	char *std_in_lines[MAXARG];
	for (int i = 1; i < argc; i ++) {
		std_in_lines[i - 1] = (char *)malloc(sizeof(char) * strlen(argv[i]));
		strcpy(std_in_lines[i - 1], argv[i]);
	}
	char s[20], c;
	int p = 0, cnt = argc - 1;
	while (read(0, &c, sizeof(char)) == sizeof(char)) {
		if (c == '\n') {
			std_in_lines[cnt] = (char *)malloc(sizeof(char) * strlen(s));
			strcpy(std_in_lines[cnt ++],s);
			p = 0;
			memset(s, 0, sizeof(s));
		} else {
			s[p ++] = c;
		}
	}
	if (strlen(s) > 0) {
		std_in_lines[cnt] = (char *)malloc(sizeof(char) * strlen(s));
		strcpy(std_in_lines[cnt ++], s);
	}
	std_in_lines[cnt] = (void *)0;
	int pid = fork();
	if (pid < 0) {
		fprintf(2, "fork failed\n");
		exit(1);
	} else if (pid == 0) { //child process
		if (strcmp(argv[1], "-n")) { // 不是多行
			exec(argv[1], std_in_lines);
		}
		/*
		   else {
		   for (int i = argc - 1; i < cnt; i ++) {
		   int inner_pid = fork();
		   if (inner_pid < 0) {
		   fprintf(2, "fork faild\n");
		   exit(1);
		   } else if (pid == 0) {
		   char **p = (char **)malloc(sizeof(char *) * (argc - 1));
		   for (int j = 0; j < argc - 3; j ++) {
		   p[j] = (char *)malloc(sizeof(char) * strlen(argv[j + 3]));
		   strcpy(p[j], argv[j + 3]);
		   }
		   p[argc - 3] = (char *)malloc(sizeof(char) * strlen(std_in_lines[i]));
		   strcpy(p[argc - 3], std_in_lines[i]);
		   p[argc - 2] = (void *)0;
		   exec(argv[3], p);
		   for (int j = 0; j < argc - 1; j ++) {
		   free(p[j]);
		   p[j] = (void *)0;
		   }
		   free(p);
		   p = (void *)0;
		   } else {
		   wait((void *)0);
		   printf("\n");
		   }
		   }
		   }
		   */
	} else { //parent process
		wait((int *)0);
	}
	for (int i = 0; i < cnt; i ++) {
		free(std_in_lines[i]);
		std_in_lines[i] = (void *)0;
	}
	exit(0);
}
