#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(void)
{
    /*
     test 1 
    */
    // pid_t pid;
    // pid = fork();
    
    // if(pid == -1)
    //     // ERR_EXIT("fork error");
    //     printf("fork error");
    // if(pid == 0){
    //     printf("this is child process and child's pid = %d,parent's pid = %d\n",getpid(),getppid());
    // }
    // if(pid > 0){
    //     //sleep(1);
    //     printf("this is parent process and pid =%d ,child's pid = %d\n",getpid(),pid);
    // }


    /*
     test 2 
    */
    fork();
    fork();

    printf("hello!\n");
    return 0;
}