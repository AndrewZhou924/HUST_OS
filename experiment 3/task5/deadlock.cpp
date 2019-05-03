#include<stdio.h>
#include<windows.h>
#include<process.h>

#define N 5                          //哲学家个数
#define LEFT(i) ((i) + N - 1) % N    //左边哲学家编号
#define RIGHT(i) ((i) + 1) % N       //右边哲学家编号
#define THINKING 0                   //思考状态
#define HUNGRY 1                     //饥饿状态
#define EATING 2                     //就餐状态

int state[N];                        //哲学家状态数组
HANDLE mutex;                        //状态访问互斥量，保证任一时刻，只有一个哲学家线程在检查左右两边状态
HANDLE semaphore[N];                 //哲学家线程信号量，标识哲学家是否获取了刀叉，否则线程阻塞
HANDLE semPar;                       //线程同步信号量，
int eattime[N] = { 2000,1000,4000,2000,3000 };           //此数组为哲学家就餐时长，可为0
int thinkingtime[N] = { 1000,3000,3000,2000,4000 };      //此数组为哲学家思考时长，可为0

void thinking(int i);
void eating(int i);
void take_forks(int i);
void put_forks(int i);

int random() {
	return rand() % 1000 + 200;
}

unsigned int __stdcall philosopher(void *pM)
{
	int i, cnt = 3;
	i = *(int*)pM;
	//线程编号，同步
	ReleaseSemaphore(semPar, 1, NULL);

	//就餐cnt次，线程结束
	while (cnt--)
	{
		thinking(i);
		take_forks(i);
		eating(i);
		put_forks(i);
	}
	printf("                  哲学家%d线程结束\n", i);
	return 0;
}

void take_forks(int i)
{
	WaitForSingleObject(mutex, INFINITE);
	state[i] = HUNGRY;
	ReleaseSemaphore(mutex, 1, NULL);
	//若获得了两个刀叉，则继续执行，否则阻塞在此处，等待邻居放下刀叉
	WaitForSingleObject(semaphore[i], INFINITE);
}

void put_forks(int i)
{
	WaitForSingleObject(mutex, INFINITE);
	state[i] = THINKING;
	ReleaseSemaphore(mutex, 1, NULL);
}


void eating(int i)
{
    int num = random();
	Sleep(num);
	printf("\t\t\t哲学家%d号吃了%d秒\n", i, num);
}

void thinking(int i)
{
	Sleep(thinkingtime[i]);
}

int main()
{
	int i;
	HANDLE thread[N];
	mutex = CreateSemaphore(NULL, 1, 1, NULL);

	for (i = 0; i < N; i++)
	{
		semaphore[i] = CreateSemaphore(NULL, 0, 1, NULL);
	}

	semPar = CreateSemaphore(NULL, 0, 1, NULL);

	for (i = 0; i < N; i++)
	{
		thread[i] = (HANDLE)_beginthreadex(NULL, 0, philosopher, &i, 0, NULL);
		WaitForSingleObject(semPar, INFINITE);
	}
	WaitForMultipleObjects(N, thread, TRUE, INFINITE);
	system("pause");
	return 0;
}