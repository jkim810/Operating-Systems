#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]){
  //file descriptors for output, read, write, and open operations
  int fd, wr, rd, op;
  //store optcode result
  int opt;
  //default buffer size
  int bytes = 0x100;
  //default output: stdout if output_file_name = NULL
  char *output_file_name = NULL;

  //gets optcode and sets them for analysis
  while((opt = getopt(argc, argv, "b:o:")) != -1){
    switch( opt ){
      case 'b':
        bytes = strtol(optarg, (char **) NULL, 10);
        if(bytes <= 0) {
          fprintf(stderr, "Buffer size has to be a positive integer: %d\n", bytes);
          return -1;
        }
        //should add error handing for case where wrong parameter (non-integer) is passed
        break;
      case 'o':
        output_file_name = optarg;
        break;
      case '?':
        if(optopt == 'b' || optopt == 'o') fprintf(stderr, "Missing argument from -%c\n", optopt);
      default:
        return 1;
    }
  }

  //no infile, read from stdin
  if(argv[optind] == NULL){
    while(rd = read(STDIN_FILENO, buf, bytes) > 0) wr = write(fd, buf, strlen(buf));
  }

  //if no ouput file is specified, output result to terminal
  fd = (output_file_name == NULL) ? STDOUT_FILENO:open(output_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  
  //initialize buffer to specified size or otherwise to default size
  char buf[bytes];
  
  //read in rest of the arguments to output
  for(int i = optind; i < argc; i++){
    op = open(argv[i], O_RDONLY);
    if(op < 0){
      fprintf(stderr, "Failed to open %s: %s\n", argv[i], strerror(errno));
      return -1;
    }
    while((rd = read(op, buf, bytes)) != 0){
      if(rd < 0){
        fprintf(stderr, "Failed to read %s: %s\n", argv[i], strerror(errno));
        return -1;
      }
      if(wr = write(fd,buf,rd) < 0){
        fprintf(stderr, "Failed to write to %s: %s\n", output_file_name, strerror(errno));
        return -1;
      } else {
        //partial rewrite
        if(wr > 0 && wr < rd) write(fd, &buf[wr], rd-wr);
      }
    }
    if(close(op) < 0){
      fprintf(stderr, "Failed to close %s: %s\n", argv[i], strerror(errno));
      return -1;
    }
  }

  if(close(fd) < 0){
    fprintf(stderr, "Failed to close %s: %s\n", output_file_name, strerror(errno));
    return -1;
  }

  //no errors found until the end of file
  return 0;
}