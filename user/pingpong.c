#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

#define READ 0
#define WRITE 1

int main()
{
	int p_ptoc[2];
	int p_ctop[2];
	int pid;
	// create tow pipes and fork
	pipe(p_ptoc);
	pipe(p_ctop);
	pid = fork();

	if (pid < 0){
		fprintf(2, "fork failed");
		exit(1);
	} else if (pid == 0){//child process
		char s[2];
		close(p_ptoc[WRITE]);
		read(p_ptoc[READ], s, 1);	
		close(p_ptoc[READ]);
		if (strcmp(s, "A") == 0){
			printf("%d: received ping\n", getpid());

			close(p_ctop[READ]);
			write(p_ctop[WRITE], "B", 1);
			close(p_ctop[WRITE]);
		}
	} else {//parent process
		char s[2];
		close(p_ptoc[READ]);
		write(p_ptoc[WRITE], "A", 1);
		close(p_ptoc[WRITE]);

		wait((int *)0);

		close(p_ctop[WRITE]);
		read(p_ctop[READ], s, 1);
		close(p_ctop[READ]);
		if (strcmp(s, "B") == 0){
			printf("%d: received pong\n", getpid());
			close(p_ctop[READ]);
		}	
	}
	exit(0);
}
