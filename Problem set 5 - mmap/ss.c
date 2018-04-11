/* SUSUNG CHOI (choi17@cooper.edu) */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>

static int sigCalled = 0;

void sig_handler(int signo) {
    sigCalled = 1;
}

int main (int argc, char **argv) {
    int c, op, op2;
    int b = 0;//number of bytes shown near the match
    char* patternfile = NULL;
    long psize;
    struct stat buf, buf2;
    char* map;
    char* map2;
    int e = 0;
    int match = 0;
    
    while ((c = getopt(argc, argv, "p:c:")) != -1) {    
     switch (c) {
    	 case 'c':
    		 b = atoi(optarg);
    		 break;
    	 case 'p':
    		 patternfile = optarg;
    		 break;
    	 case '?':
    		 //if user doesnt supply arguments after -b or -o
    		 if(optopt == 'p' || optopt == 'c')
    			 fprintf(stderr, "Missing argument from -%c\n", optopt);
    	 default:
    		 return 1;
     }
    }
    
    

    if(signal(SIGBUS, sig_handler) == SIG_ERR) {//add signal handler to SIGBUS
        fprintf(stderr, "ERRNO %d - Failed to assign signal SIGBUS to signal handler : %s\n", errno, strerror(errno));
    }    
    if(patternfile == NULL) {//if user doesn't specify pattern file, take from stdin
        map = argv[optind];
        optind++;
        psize = strlen(map);
    } else {//otherwise, open the pattern file and turn into block of bytes
        op = open(patternfile, O_RDONLY);
        if(op < 0) {
            fprintf(stderr, "ERRNO %d - Failed to open '%s' for reading : %s\n", errno, patternfile, strerror(errno));
        	return 1;
        }
        if(fstat(op, &buf) < 0) {
        	fprintf(stderr, "ERRNO %d - Failed to open '%s' for stat : %s\n", errno, patternfile, strerror(errno));
        	return 1;
        }   
        if((map = mmap(NULL, buf.st_size, buf.st_mode, MAP_SHARED, op, 0)) == MAP_FAILED) {
            fprintf(stderr, "ERRNO %d - Failed to map '%s' on virtual address : %s\n", errno, patternfile, strerror(errno));
            return 1;
        }
        psize = buf.st_size-1;//st_size includes null terminator
    }
    
    for (int i = optind;i < argc;i++) {
    int found = 0;  
    op2 = open(argv[i], O_RDONLY);
    if(op2 < 0) {
    	fprintf(stderr, "ERRNO %d - Failed to open '%s' for reading : %s\n", errno, argv[i], strerror(errno));
    	e = -1;
    	continue;
    }
     if(fstat(op2, &buf2) < 0) {
    	 fprintf(stderr, "ERRNO %d - Failed to open '%s' for stat : %s\n", 
 errno, argv[i], strerror(errno));
    	 e = -1;
    	 continue;
     }
     if((map2 = mmap(NULL, buf2.st_size, buf2.st_mode, MAP_SHARED, op2, 0)) == 
 MAP_FAILED) {
    	 fprintf(stderr, "ERRNO %d - Failed to map '%s' on virtual address : %s\n", 
 errno, argv[i], strerror(errno));
    	 e = -1;
    	 continue;
     }
     
     for(int j = 0;j < buf2.st_size-psize;j++) {//go through every byte in the file
    	 //stop iterating through file if sigbus is called
    	 if(sigCalled == 1) {
    		 printf("SIGBUS received while processing file %s\n", argv[i]);
    		 e = -1;
    		 break;
    	 }
    	 int k = 0;
    	 //if the characters match, keep checking until the end of pattern
    	 while(map[k] == map2[j+k]) {
    		 if(k == psize-1) {
    			 found = 1;
    			 match++;
    			 break;
    		 }
    		 k++;
    	 }
    	 
    	 if(found == 1) {
    		 int start;
    		 int end;
    		 
    		 //if -c arg is supplied, extend the range of bytes displayed
    		 if(j-b < 0) start = 0;
    		 else start = j-b;
    		 
    		 if(psize+j+b > buf2.st_size) end = buf2.st_size;
    		 else end = psize+j+b;
    		 
    		 printf("%s:%d ", argv[i], j);
    		 
    		 for(int q = start;q < end;q++) {
    			 printf("%c ", map2[q] > 31 && map2[q] < 127 ? map2[q] : '?');
    		 }
    		 
    		 printf("\t");
    		 
    		 for(int p = start;p < end;p++) {
             printf("%02X ", map2[p]);
         }
         
    		 printf("\n");
         
    		 break;
    	 }
     }
    
 if(sigCalled == 1) sigCalled = 0;
     close(op2);
    }    
    close(op);    
    if(e < 0) return -1;
    return match;
}
