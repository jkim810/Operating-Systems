#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
    char s[10];
    while(1){
        printf("Input: ");
        s = NULL;
        scanf("%s", s);
        for(int i = 0; i < strlen(s); i++){
            printf("Echo: %c\n", s[i]);   
        }
    }
    
    return 0;
}