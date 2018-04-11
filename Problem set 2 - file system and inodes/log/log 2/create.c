#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

int makefile(const char *filename, const char *s, int c, int depth);
int removefile(const char *filename, int c, int depth);

int main(){
	
	char *filename = "/media/sdf1/files/randomfile";
	char pathname[PATH_MAX];
	char *s = "some random string: ";
	char buf[BUFSIZ];
	int fd, fdwr, fdcl, fdrm, ctr;
       	ctr = 1;	
	while(ctr++){
		makefile(filename, s, ctr, 0);
		removefile(filename, ctr, 0);
	}
}


int makefile(const char *filename, const char *s, int c, int depth){
	char pathname[PATH_MAX];
	char buf[BUFSIZ];
	int fd, fdwr, fdcl, ctr;
	
	if(depth++ < 6){	
		sprintf(pathname, "%s%d", filename, c);
		sprintf(buf, "%s %d at depth: \n", s, c, depth);
		fd = open(pathname, O_WRONLY|O_CREAT|O_TRUNC, 0666);
		if(fd < 0) fprintf(stderr, "Error opening file %s, %s\n", pathname, strerror(errno));
		fdwr = write(fd, buf, sizeof(buf));
		if(fdwr < 0) fprintf(stderr, "Error writing file %s, %s\n", pathname, strerror(errno));
		fdcl = close(fd);
		if(fdcl < 0) fprintf(stderr, "Error closing file %s, %s\n", pathname, strerror(errno)); 
		for(int i = 0; i < 5; i++) makefile(filename, s, 10*c+i, depth);
	}else{
		return 0;
	}	
}

int removefile(const char *filename, int c, int depth){
	int fd;
	char pathname[PATH_MAX];
	char oldpath[PATH_MAX];
	char newpath[PATH_MAX];
	if(depth++ < 3){
		sprintf(pathname, "%s%d", filename, c);
		fd = unlink(pathname);
		if(fd < 0) fprintf(stderr,"Error deleting file %s, %s\n", pathname, strerror(errno));
		for(int i = 0; i < 5; i++) removefile(filename, 10*c+i, depth);
	} else if (depth++ < 6){
		sprintf(oldpath, "%s%d", filename, c);
		sprintf(newpath, "%s%d", filename, c *10000000 + c^2);
		fd = rename(oldpath,newpath);
		for(int i = 0; i < 5; i++) removefile(filename, 10*c + 1, depth);
		
	}else{
		return 0;
	}
}

