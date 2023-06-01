#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#define MAX_SIZE 512

int getLines(char buf[]){
    int i = 0;
    while(read(0, buf+i, 1) && *(buf+i) != '\n'){
        i++;
    }
    *(buf+i) = 0;
    return strlen(buf);
}

int splitToken(char buf[], char* x_argv[], int end_idx){
    char *p = strchr(buf,' ') , *q = buf;
    if(!p){
        x_argv[end_idx++] = q;
    }else{
        *p = 0;
        x_argv[end_idx++] = q; 
    }

    while(p){
        q = p + 1;
        p = strchr(q, ' ');
        if(p){
            *p = 0;
            x_argv[end_idx++] = q; 
        }else{
            x_argv[end_idx++] = q; 
        }
    }
    x_argv[end_idx] = 0;
    return end_idx;
}

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Too few arguments!");
        exit(1);
    }
    char* x_argv[MAXARG];
    char buf[MAX_SIZE];

    if(argc > MAXARG){
        printf("Too many arguments!");
        exit(1);        
    }

    for(int i = 1; i<argc; i++){
        x_argv[i-1] = argv[i];
    }

    while(getLines(buf)){
        if(fork() == 0){
            splitToken(buf,x_argv,argc-1);
            exec(x_argv[0],x_argv);
        }else{
            wait(0);
        }        
    } 
    exit(0);
}

