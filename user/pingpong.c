#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int pipe1[2],pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    char byte[1];
    if(fork() == 0){
        read(pipe1[0], byte, 1);
        printf("%d: received ping\n", getpid());
        write(pipe2[1], byte, 1);
    }else{
        write(pipe1[1], "a", 1);
        wait(0);
        read(pipe2[0], byte, 1);
        printf("%d: received pong\n", getpid());
    }
    
    exit(0);
}
