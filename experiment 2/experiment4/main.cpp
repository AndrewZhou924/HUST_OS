/*
任务四：
在 windows 环境下，利用高级语言编程环境（限定为 VS 环境或 VC 环境）
调用 CreateThread 函数和相关的同步函数，模拟实现“生产者-消费者”问题。

该问题的关键就是要保证生产者不会在缓冲区满时加入数据，消费者也不会在缓冲区中空时消耗数据。
*/

#include<stdio.h>
#include<Windows.h>
#include<time.h>

#define N 100
#define TRUE 1
typedef int Semaphore;
Semaphore FULL = 0, EMPTY = N;            //共享资源区满槽数目和空槽数目
HANDLE MUTEX;
int in = 0, out = 0;                      //缓冲区生产，消费数据指针
int ProducerThread[5];
int ConsumerThread[5];
int Buffer[N + 4];                         //缓冲区，用于寄存资源区物品的编号


//生产随机数
int produce_item() {                      
	return (rand() % N + N) % N;
}

//插入资源
void insert_item(int item, int ThreadNum) {
	in %= N;
	printf("线程%d生产到缓冲区槽： %d\n", ThreadNum,in);
	Buffer[in] = item;
}

//移出资源
void remove_item(int ThreadNum) {
	out %= N;
	printf("                                 线程%d取走缓冲区槽 %d 的数\n", ThreadNum,out);
}

//上锁操作
void down(HANDLE handle) {
	WaitForSingleObject(handle, INFINITE);
}

//解锁操作
void up(HANDLE handle) {
	ReleaseSemaphore(handle, 1, NULL);
}

//生产者线程函数
DWORD WINAPI producer(LPVOID v) {

	int item;
	int ThreadNum = (int)v;

	while (TRUE) {
		item = produce_item();

		if (EMPTY > 0) {         
			//上锁
			down(MUTEX);          

			//生产产品，并修改FULL和EMPTY
			EMPTY--;
			insert_item(item,ThreadNum);
			FULL++;

			//解锁
			up(MUTEX);            
		}

		//暂停0.5-1.5s
		Sleep((rand() % (1500 - 500)) + 500);
	}
	return 1;
}

//消费者线程函数
DWORD WINAPI consumer(LPVOID v) {

	int item;
	int ThreadNum = (int)v;

	while (TRUE) {

		if (FULL > 0) { 
			//上锁
			down(MUTEX);     

			//用掉产品，并修改FULL和EMPTY
			FULL--;
			remove_item(ThreadNum);
			EMPTY++;          

			//解锁
			up(MUTEX);             
		}

		//暂停0.5-1s
		Sleep((rand() % (1000 - 500)) + 500);
	}
	return 1;
}

int main()
{
	DWORD Tid;

	//创建互斥信号量MUTEX
	MUTEX = CreateSemaphore(NULL,1,1,NULL);

	for (int i = 0; i<4; i++) {

		//创建生产者线程
		ProducerThread[i] = i + 1;
		CreateThread(NULL,0,producer,&ProducerThread[i],0,&Tid);

		//创建消费者线程
		ConsumerThread[i] = i + 1;
		CreateThread(NULL,0,consumer,&ConsumerThread[i],0,&Tid);   
	}

	Sleep(20000);
	char c = getchar();
	return 0;
}



