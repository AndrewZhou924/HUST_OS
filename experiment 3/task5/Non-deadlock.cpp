#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <time.h>

using namespace std;

HANDLE chop[5];
HANDLE ph[5];
HANDLE mutex;
int nums = 0;

int random() {
	return rand() % 1000 + 200;
}

int random1() {
	return rand() % 1000 + 200;
}

int random2() {
	return rand() % 1000 + 200;
}

int random3() {
	return rand() % 1000 + 200;
}

int random4() {
	return rand() % 1000 + 200;
}

int random5() {
	return rand() % 1000 + 200;
}

void eating(int id) {
	int num = random();
	Sleep(num);
	printf("\t\t\t哲学家%d号吃了%d秒\n", id, num);
}

DWORD WINAPI phthread(LPVOID param) {
	nums++;
	int id = nums;
	int lc = id - 1;
	int rc = id % 5;
	int times = 0;
	int ret1, ret2;
	while (true)
	{	
		int considerTime;
		switch (id) {
		case 1: considerTime = random1();
		case 2: considerTime = random2();
		case 3: considerTime = random3();
		case 4: considerTime = random4();
		case 5: considerTime = random5();
		}
			

		//int considerTime = random();
		printf("哲学家%d号思考了%ds。\n", id, considerTime);
		Sleep(considerTime);

		if (id % 2 == 0)
		{
			ret1 = WaitForSingleObject(chop[lc], 0);
			if (ret1 == WAIT_OBJECT_0)
			{
				ret2 = WaitForSingleObject(chop[rc], 0);
				if (ret2 == WAIT_OBJECT_0)
				{
					WaitForSingleObject(mutex, INFINITE);
					printf("哲学家%d号拿到两只筷子开始吃饭。\n", id);
					eating(id);
					ReleaseMutex(mutex);
					printf("\t\t\t哲学家%d号吃完饭啦，放下筷子。\n", id);
					ReleaseSemaphore(chop[rc], 1, NULL);
				}
				ReleaseSemaphore(chop[lc], 1, NULL);
			}
		}
		else
		{
			ret1 = WaitForSingleObject(chop[rc], 0);
			if (ret1 == WAIT_OBJECT_0)
			{
				ret2 = WaitForSingleObject(chop[lc], 0);
				if (ret2 == WAIT_OBJECT_0)
				{
					WaitForSingleObject(mutex, INFINITE);
					printf("哲学家%d号拿到两只筷子开始吃饭。\n", id);
					eating(id);
					ReleaseMutex(mutex);
					printf("\t\t\t哲学家%d号吃完饭啦，放下筷子。\n", id);
					ReleaseSemaphore(chop[lc], 1, NULL);
				}
				ReleaseSemaphore(chop[rc], 1, NULL);
			}
		}
		WaitForSingleObject(mutex, INFINITE);
		ReleaseMutex(mutex);
	}
	//printf("=======哲学家%d吃饱了然后离开了。=======\n", id);
	return 0;

}

int main() {
	srand((unsigned)time(0));
	mutex = CreateMutex(NULL, false, NULL);
	for (int i = 0; i < 5; ++i)
	{
		chop[i] = CreateSemaphore(NULL, 1, 1, NULL);
	}
	for (int i = 0; i < 5; ++i)
	{
		int j = i + 1;
		ph[i] = CreateThread(NULL, 0, phthread, NULL, 0, NULL);
	}


	Sleep(10000);//释放句柄
	for (int i = 0; i < 5; ++i)
	{
		CloseHandle(ph[i]);
		CloseHandle(chop[i]);
	}
	CloseHandle(mutex);
	//Sleep(500);
	//system("pause");
	return 0;
}