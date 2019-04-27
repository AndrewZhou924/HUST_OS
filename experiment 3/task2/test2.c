
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int logFlag = 0;//全局变量，是否打印log的标志


//信号没有带参数时使用的中断处理函数
void signal_handler(int signo)
{
    switch(signo)
    {
        case SIGINT:
            //SIGINT默认行为是退出进程
            printf("in signal_handler() SIGINT signal\n");
            exit(0);
            break;
        case SIGALRM:
            //SIGALRM默认行为是退出进程
            printf("SIGALRM signal\n");
            break;
    }   
}
//信号携带参数时使用的中断处理函数
void signal_handler_param(int signo,siginfo_t *info,void *context)
{
    switch(signo)
    {
        case SIGINT:
            //SIGINT默认行为是退出进程
            printf(" SIGINT signal \n");
            exit(0);
            break;
        case SIGUSR1:
            //当logFlag等于1的时候表示打开log标志
            logFlag = 1;
            printf("SIGUSR1 , signo=%d ,logFlag = %d,recv data :%d\n",signo,logFlag,info->si_value.sival_int);
            break;
        case SIGUSR2:
            printf("SIGUSR2 ,signo=%d , recv data :%d\n",signo,info->si_value.sival_int);
            break;
    }   
    return ;
}

typedef void (*pFunc)(int);
typedef void (*pSignalFunParam)(int, siginfo_t *, void *);

int signal_sigaction(int signo,void *func,int flag)
{
    struct sigaction act,oact;
    //填充软中断函数
    switch(flag)
    {
        case 0:
            act.sa_handler = (pFunc)func;
            break;
        case SA_SIGINFO:
            act.sa_sigaction = (pSignalFunParam)func;
            break;
    }
    //act.sa_handler=func;
    //将act的属性sa_mask设置一个初始值
    sigemptyset(&act.sa_mask);
    act.sa_flags=flag;
    int ret = sigaction(signo,&act,&oact);
    if (ret !=0 )
    {
        printf("sigaction() failed !\n");
    }
    return ret;
}

int main(int arg, char *args[])
{
    pid_t pid=0;
    pid=fork();
    if(pid==-1)
    {
        printf("fork process failed！\n");
        return -1;
    }

    if(pid>0)//父进程
    {
        printf("in father!\n");

        //linux内核为用户程序保留了两个信号，一个是10 SIGUSR1还有12 SIGUSR2

        //要想接受信号带来的参数，必须使用了SA_SIGINFO选项
        signal_sigaction(SIGUSR1,signal_handler_param,SA_SIGINFO);
        signal_sigaction(SIGUSR2,signal_handler_param,SA_SIGINFO);

        //不带参数
        signal_sigaction(SIGINT,signal_handler,0);

        //等待子进程发送信号
        while(1)
        {
            //pause()会令目前的进程暂停(进入睡眠状态), 直到被信号(signal)所中断
            pause();        
        }
        printf("recv signal from child\n");
    }


    if(pid==0)//子进程
    {
        printf("in child!\n");

        //因为我们不知道子进程还是父进程先执行，这里等待是为了父进程执行完信号安装
        sleep(2);

        //向父进程发送带数据的信号
        union sigval sigvalue;
        sigvalue.sival_int=110;

        //发送信号SIGUSR1
        if(sigqueue(getppid(),SIGUSR1,sigvalue)==-1)
        {
            printf("sigqueue() failed!\n");
            exit(0);
        }
        //发送信号SIGUSR2
        if(sigqueue(getppid(),SIGUSR2,sigvalue)==-1)
        {
            printf("sigqueue() failed!\n");
            exit(0);
        }


        printf("child process send signal successfully\n");
        exit(0);
    }

    return 0;
}