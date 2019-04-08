// 在Ubantu 或Fedora 环境使用fork函数创建一对父子进程，分别输出各自的进程号和提示信息串。
// 子进程可以调用getpid()来获取自己的pid；也可以调用getppid()来获取父进程的id

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

int main() {

    pid_t pid;
    pid = fork();

    if (0 == pid)
        fprintf(stdout, "我是子进程，我的pid是 %d, 我真正的pid是 %d,　我的父进程的pid是 %d\n", pid, getpid(), getppid());
    else
        fprintf(stdout, "我是父进程, 我的子进程的pid是 %d, 我的pid是 %d, 我的父进程的pid是 %d\n", pid, getpid(), getppid());

    return 0;
} 
