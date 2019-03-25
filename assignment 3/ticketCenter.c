#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#define MAX 10
pthread_t thread[2];
pthread_mutex_t mut;
int number = 1, i = 0;

void *thread1()
{
    printf("窗口1 : 窗口1已开启售票服务\n");
    while (i < MAX)
    {
        printf("窗口1 : 售出第 %d 张票\n", number);
        pthread_mutex_lock(&mut);
        number++;
        i++;
        pthread_mutex_unlock(&mut);
        sleep(2);
    }
    printf("窗口1 :售票已结束，准备关闭售票窗口1\n");
    pthread_exit(NULL);
}

void *thread2()
{
    printf("thread2 : 窗口2已开启售票服务\n");
    while (i < MAX)
    {
        printf("窗口2 : 售出第 %d 张票\n", number);
        pthread_mutex_lock(&mut);
        number++;
        i++;
        pthread_mutex_unlock(&mut);
        sleep(3);
    }
    printf("窗口2 :售票已结束，准备关闭售票窗口2\n");
    pthread_exit(NULL);
}

void thread_create()
{
    int temp;
    memset(&thread, 0, sizeof(thread)); //comment1
    /*创建线程*/
    if ((temp = pthread_create(&thread[0], NULL, thread1, NULL)) != 0) //comment2
        printf("窗口1服务出现问题!\n");
    else
        printf("窗口1服务已开启\n");
    if ((temp = pthread_create(&thread[1], NULL, thread2, NULL)) != 0) //comment3
        printf("窗口2服务出现问题");
    else
        printf("窗口2服务已开启\n");
}

void thread_wait()
{
    /*等待线程结束*/
    if (thread[0] != 0)
    { //comment4
        pthread_join(thread[0], NULL);
        printf("窗口1服务已结束\n");
    }
    if (thread[1] != 0)
    { //comment5
        pthread_join(thread[1], NULL);
        printf("窗口2服务已结束\n");
    }
}

int main()
{
    /*用默认属性初始化互斥锁*/
    pthread_mutex_init(&mut, NULL);
    thread_create();
    thread_wait();
    return 0;
}

