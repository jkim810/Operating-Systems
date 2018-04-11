#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

int traverse (const char* path, const char* root, const struct stat fstat, int readableByOther);
int compareFiles(const char *file1,const char *file2);

int main(int argc, char **argv){

	if(argc != 3){
		printf("Insufficient parameters. Required program parameters: [target file] [start path]\n");
		exit(1);
	}

	char* targetFile = argv[1];
	char* rootPath = argv[2];
	char targetFileDir[PATH_MAX];
	DIR* dir;
	struct dirent *ls;
	struct stat fstat;
	int targetExists = 0;

	if(!(dir = opendir(rootPath))){
		fprintf(stderr,"DEBUG: Failed to open directory [%s]: %s\n", rootPath, strerror(errno));
		exit(1);
	}

	while((ls = readdir(dir))){
		if(strcmp(ls->d_name, targetFile) == 0){
			sprintf(targetFileDir, "%s/%s", rootPath, targetFile);
			if(stat(targetFileDir, &fstat) < 0){
				fprintf(stderr, "DEBUG: Failed to open [%s] stat: %s\n", targetFileDir, strerror(errno));
				exit(1);
			}

			//print stats of the target file
			printf("DEBUG: Target [%s] is %li bytes long, dev %li, ino %lu\n",targetFileDir, fstat.st_size, fstat.st_dev,fstat.st_ino);
			printf("DEBUG: Hard Link #: %lu, Block size: %li, # of 512B blocks allocated: %li\n\n", fstat.st_nlink, fstat.st_blksize, fstat.st_blocks);
			targetExists = 1;
			break;
		}
	}

	if(!readdir(dir)){
		fprintf(stderr, "DEBUG: Target [%s} does not exist in [%s]\n", argv[1], argv[2]);
		exit(1);
	}else{
		(fstat.st_mode & (S_IROTH | S_IXOTH)) ? traverse(rootPath, targetFileDir, fstat, 1): traverse(rootPath, targetFileDir, fstat, 0);
	}

	if(closedir(dir) == -1) fprintf(stderr,"DEBUG: Failed to close directory %s: %s\n", rootPath, strerror(errno));
}

int traverse (const char* path, const char *root, const struct stat target_fstat, int readableByOther){
	DIR* dir;
	char* child;
	char newPath[PATH_MAX];
	struct dirent *ls;
	struct stat fstat;
	ssize_t r;
	char *linkname;

	if(!(dir = opendir(path))){
		//report if directory failed to open
		fprintf(stderr,"DEBUG: Failed to open directory [%s]: %s\n", path, strerror(errno));
	} else {
		while((ls = readdir(dir))){
			child = ls->d_name;
			//ignore . and .. file for efficiency
			if(strcmp(child, ".")!=0 && strcmp(child, "..")!=0){
				sprintf(newPath, "%s/%s", path, child);
				if(strcmp(newPath, root)!=0){
					switch(ls->d_type){
						case DT_REG:
							//if traverse fails to open stats
							if(stat(newPath, &fstat) < 0) { 
								fprintf(stderr, "DEBUG: Failed to open [%s] stat for file: %s\n", newPath, strerror(errno));
								continue;
							}
							if((ls->d_ino == target_fstat.st_ino) &&(fstat.st_dev == target_fstat.st_dev)){
								printf("[%s]\tHARD LINK TO TARGET\t", newPath);
								((fstat.st_mode & S_IROTH) && readableByOther) ? printf("OK READ BY OTHERS\n"):printf("NOT READABLE BY OTHERS\n");
							} else {
								//inspect file if they have same size
								if(fstat.st_size == target_fstat.st_size){
									if(compareFiles(root, newPath) == 0) { 
										printf("[%s]\tDuplicate of Target (nlink = %lu)\t" , newPath, fstat.st_nlink);
										((fstat.st_mode & S_IROTH) && readableByOther)? printf("OK READ BY OTHERS\n"):printf("NOT READABLE BY OTHERS\n");
									}
								}
							}
						break;

						case DT_LNK:
							if(stat(newPath, &fstat) < 0) { 
								fprintf(stderr, "DEBUG: [%s] links to something not a file: %s, skipping\n", newPath, strerror(errno));
								continue;
							} 
							linkname = malloc(fstat.st_size + 1);
							r = readlink(newPath, linkname, fstat.st_size+1);
							if(r < 0) {
								fprintf(stderr, "Error opening symlink [%s]: %s\n", newPath, strerror(errno));
							}
							linkname[r] = '\0';
							if((fstat.st_ino == target_fstat.st_ino)&&(fstat.st_dev == target_fstat.st_dev)) printf("[%s]\tSYMLINK RESOLVES TO TARGET\n", newPath);
							else{
								if(fstat.st_size == target_fstat.st_size){
									sprintf(newPath, "%s/%s", path, linkname);
									if(compareFiles(newPath, root) == 0)//symlink resolves to duplicate
										printf("[%s]\tSYMLINK [%s] RESOLVES TO DUPLICATE\n", newPath, linkname);
								}
							}
							free(linkname);
													 
							break;

						case DT_DIR:
							if(stat(newPath, &fstat) < 0) { 
								fprintf(stderr, "DEBUG: Failed to open [%s] stat for directory: %s\n", newPath, strerror(errno));							
							} else {
								((fstat.st_mode & S_IXOTH) && readableByOther) ? traverse(newPath, root, target_fstat, 1):traverse(newPath, root, target_fstat, 0);
							}
							break;
						default:
							;
					}
				}
				
			}
		}

		if(closedir(dir) == -1) fprintf(stderr,"DEBUG: Failed to close directory %s: %s\n", path, strerror(errno));
	}
};

//compare bitwise
//returns 0 on same file, 1 on different files
int compareFiles(const char *file1, const char *file2){
	int fd1, fd2, one, two, result;
	char buf1[BUFSIZ];
	char buf2[BUFSIZ];
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));

	if((fd1 = open(file1, O_RDONLY)) < 0) {
		fprintf(stderr, "DEBUG: Error opening file %s: %s\n", file1, strerror(errno));
		return -1;
	}
	if((fd2 = open(file2, O_RDONLY)) < 0) {
		fprintf(stderr, "DEBUG: Error opening file %s: %s\n", file2, strerror(errno));
		return -1;
	}

	while(one = read(fd1, buf1, sizeof(buf1)) != 0){
		two = read(fd2, buf2, sizeof(buf2));
		if(one < 0) {
			fprintf(stderr, "DEBUG: Error reading file %s: %s\n", file1, strerror(errno));
			return -1;
		}
		if(two < 0) {
			fprintf(stderr, "DEBUG: Error reading file %s: %s\n", file2, strerror(errno));
			return -1;
		}
		if(result = memcmp(buf1, buf2, BUFSIZ) != 0){
			return 1;
		}
	}
	if(close(fd1)<0) {
		fprintf(stderr, "DEBUG: Error closing file %s: %s\n", file1, strerror(errno));
		return -1;
	}
	if(close(fd2)<0) {
		fprintf(stderr, "DEBUG: Error closing file %s: %s\n", file2, strerror(errno));
		return -1;
	}
	return 0;
}