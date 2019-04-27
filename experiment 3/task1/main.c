#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>  

#define TEXT_SZ 101

struct shared_use_st {
    int written;       //作为一个标志，非0：表示可读，0表示可写
    int text[TEXT_SZ]; //记录写入和读取的文本
};

void main() {
    int pRunning = 1;             //程序是否继续运行的标志(父)
    int cRunning = 1;             //程序是否继续运行的标志(子)
    void *shm = NULL;             //分配的共享内存的原始首地址
    struct shared_use_st *shared; //指向shm
    char buffer[BUFSIZ + 1];      //用于保存输入的文本
    int shmid;                    //共享内存标识符

    //创建共享内存
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    //将共享内存连接到当前进程的地址空间
    shm = shmat(shmid, 0, 0);
    if (shm == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    //设置共享内存
    shared = (struct shared_use_st *)shm;
    shared->written = 0;
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "can't fork ,error %d\n", errno);
        exit(1);
    } else if (pid == 0) {
        while (cRunning) //读取共享内存中的数据
        {
            //没有进程向共享内存定数据有数据可读取
            if (shared->written != 0)
            {
                int i = 0;
                while (i < 100)
                {
                    printf("child  read: %d\n", shared->text[i]);
                    i++;
                }
                //读取完数据，设置written使共享内存段可写
                shared->written = 0;
                //退出循环（程序）
                cRunning = 0;
            }
            else //有其他进程在写数据，不能读取数据
                sleep(1);
        }
    }
    else //父进程
    {
        int i = 1;
        while (pRunning) //向共享内存中写数据
        {
            //数据还没有被读取，则等待数据被读取,不能向共享内存中写入文本
            while (shared->written == 1)
            {
                sleep(1);
            }
            //向共享内存中写入数据
            shared->text[i - 1] = i;
            printf("parent write: %d\n", shared->text[i - 1]);
            i++;
            if (i > 100)
            {
                //写完数据，设置written使共享内存段可读,退出循环（程序）
                shared->written = 1;
                pRunning = 0;
            }
        }
    }
    //把共享内存从当前进程中分离
    if (shmdt(shm) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    //删除共享内存
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        // fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}