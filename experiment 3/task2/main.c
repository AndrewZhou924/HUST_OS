
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//全局变量，用于计数
int count = 0;


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
            count += info->si_value.sival_int;
            printf("child process get data :%d, and now count is %d\n",info->si_value.sival_int, count);
            break;
        case SIGUSR2: ;
            char* p = info->si_value.sival_ptr;
            printf("child process get string : %s\n",p);
            if (count >= 100) exit(0);
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
    switch(flag) {
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
    if (ret !=0 ) {
        printf("sigaction() failed !\n");
    }
    return ret;
}


int main(int arg, char *args[]) {
    pid_t pid=0;
    pid=fork();

    if(pid==-1) {
        printf("fork process failed！\n");
        return -1;
    }

    //父进程
    if(pid>0) {
        printf("in father\n");

        //等待是为了子进程执行完信号安装
        sleep(2);
        
        while(1) {
            // 等待子进程
            sleep(1);
            if ((waitpid(pid, NULL, WNOHANG)) != 0) {
                break;
            }

            char inputString[100];
            scanf("%s", inputString);

            //判断输入字符串是否为数字
            if ((strspn(inputString, "0123456789")==strlen(inputString))) {
                int num = atoi(inputString);
                // printf("is number %d\n",num);

                //向子进程发送带数据的信号
                union sigval sigvalue;
                sigvalue.sival_int=num;

                //发送信号SIGUSR1
                if(sigqueue(pid,SIGUSR1,sigvalue)==-1) {
                    printf("sigqueue() failed!\n");
                    exit(0);
                }

                if(count >= 100) {
                    printf("finish counting!\n");
                    exit(0);
                    break;
                }
            
            } else {
                // printf("not number\n");

                //向子进程发送带数据的信号
                union sigval sigvalue;

                char* string = "hello";
                sigvalue.sival_ptr= string;

                // printf("%s\n",inputString);

                // const char string[] = &inputString[0];
                // printf("%s\n",string);

                // sigvalue.sival_ptr = string;
                

                //发送信号SIGUSR2
                if(sigqueue(pid,SIGUSR2,sigvalue)==-1) {
                    printf("sigqueue() failed!\n");
                    exit(0);
                }

                // sleep(3);
            }
        }

        printf("parent process shut down\n");
        exit(0);
    }

    //子进程
    if(pid==0) {
        printf("in child!\n");

        //linux内核为用户程序保留了两个信号，一个是10 SIGUSR1还有12 SIGUSR2
        //要想接受信号带来的参数，必须使用了SA_SIGINFO选项
        signal_sigaction(SIGUSR1,signal_handler_param,SA_SIGINFO);
        signal_sigaction(SIGUSR2,signal_handler_param,SA_SIGINFO);

        //等待父进程发送信号
        while(count <= 100) {
            //pause()会令目前的进程暂停(进入睡眠状态), 直到被信号(signal)所中断
            pause();        
        }

        
        printf("finish counting, my work done！\n");
        printf("child process shut down\n");
    }

    return 0;
}