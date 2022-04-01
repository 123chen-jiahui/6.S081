#include "kernel/types.h"
#include "kernel/fs.h"
#include "user/user.h"
#include "kernel/stat.h"

void find(char *path, char *file_name)
{
	int fd;
	struct stat st;
	struct dirent de;
	if ((fd = open(path, 0)) < 0){
		fprintf(2, "cannot open %s\n", path);
		exit(1);
	}
	if (fstat(fd, &st) < 0){
		fprintf(2, "cannot stat %s\n", path);
		close(fd);
		exit(1);
	}
	char buf[50], *p;
	strcpy(buf, path);
	p = buf + strlen(buf);
	*p ++ = '/';
	while (read(fd, &de, sizeof(de)) == sizeof(de)){
		if (de.inum == 0)
			continue;
		memmove(p, de.name, strlen(de.name));
		p[strlen(de.name)] = 0;
		stat(buf, &st);
		if (st.type == T_FILE && strcmp(de.name, file_name) == 0){
				printf("%s\n", buf);
		} else if (st.type == T_DIR && strcmp(de.name, ".") && strcmp(de.name, "..")){
			find(buf, file_name);
		}
	}
	close(fd);
}

int main(int argc, char *argv[])
{
	if (argc < 2){
		fprintf(2, "too fewer arguments\n");
		exit(1);
	} else if (argc > 3){
		fprintf(2, "too many arguments\n");
		exit(1);
	} else{
		find(argv[1], argv[2]);
	}
	exit(0);
}
