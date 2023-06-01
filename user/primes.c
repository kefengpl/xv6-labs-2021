#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define MAX_VAL 36

void prime(int* left_pipe){
    int buf;
    //如果不先close，会导致read语句一直等待(所谓“死锁”)
    close(left_pipe[1]);
    int read_ret = read(left_pipe[0],&buf,sizeof(buf));
    //错误(buf == 0):注意：read的return值和buf不一样！ 
    if(read_ret == 0){
        close(left_pipe[0]);
        exit(0);
    }
    printf("prime %d\n", buf);
    int right_pipe[2];
    pipe(right_pipe);

    if(fork() == 0){
        prime(right_pipe);
    }else{
        close(right_pipe[0]);
        int tmp;
        while(read(left_pipe[0], &tmp, sizeof(tmp))){ //&& tmp < MAX_VAL
            if(tmp % buf != 0){
                write(right_pipe[1], &tmp, sizeof(tmp));
            }
        }
        //write(right_pipe[1], &tmp, sizeof(tmp)); 
        close(left_pipe[0]); 
        close(right_pipe[1]); 
        wait(0);       
    }
    exit(0);
}

int
main(int argc, char *argv[])
{
    int left_pipe[2];
    pipe(left_pipe);

    if(fork() == 0){
        prime(left_pipe);
    }else{
        close(left_pipe[0]);
        for(int i=2; i<MAX_VAL; i++){
            write(left_pipe[1], &i, 4);
        }
        close(left_pipe[1]);
        wait(0);
    }
    exit(0);
}


