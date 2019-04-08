/*
任务五：
在 windows 环境下，利用高级语言编程环境（限定为 VS 环境或 VC 环境）
调用 CreateThread 函数实现“并发地画圆和画方”。圆的中心，半径，颜色，正
方形的中心，边长，颜色等参数自己确定，合适就行。圆和正方形的边界上建议
取 720 个点。为直观展示绘制的过程，每个点绘制后睡眠 0.2 秒~0.5 秒。
*/

#include<stdio.h>
#include<Windows.h>
#include<time.h>
#include<math.h>

CRITICAL_SECTION cs;//定义临界区全局变量

//画圆
DWORD WINAPI circle(LPVOID n) {
	/*EnterCriticalSection(&cs);*/
	double m = 0;
	int cx = 0, cy = 0;

	for (cy = 10; cy >= -10; cy--) {
		EnterCriticalSection(&cs);
		m = 2.5*sqrt(100 - cy * cy);			
		for (cx = 1; cx < 100 - m; cx++)        
			printf(" ");
		printf(".");
		Sleep(100);
		for (; cx < 100 + m; cx++)              
			printf(" ");
		printf(".\n");

		LeaveCriticalSection(&cs);
		Sleep(100);
	}
	//LeaveCriticalSection(&cs);
	return 0;
}

//画方
DWORD WINAPI tangle(LPVOID n) {
	//EnterCriticalSection(&cs);
	int x = 0, y = 0;
	for (y = 20; y >= 0; y--)
	{
		EnterCriticalSection(&cs);
		if (y == 20 || y == 0) {
			for (x = 1; x <= 20; x++)
			{
				printf(" ");
				printf(".");
				Sleep(100);
				printf(" ");
			}
			printf("\n");
		}
		else if (y != 20 || y != 0)
		{
			printf(" ");
			printf(".");
			Sleep(100);
			printf(" ");
			for (x = 2; x < 20; x++)
			{
				printf(" ");
				printf(" ");
				printf(" ");
			}
			printf(" ");
			printf(".");
			Sleep(100);
			printf(" ");
			printf("\n");

		}
		LeaveCriticalSection(&cs);
		Sleep(100);
	}
	//LeaveCriticalSection(&cs);
	return 0;
}

int main() {
	//初始化临界区
	InitializeCriticalSection(&cs);

	HANDLE hThread[2];
	//创建线程，并调用函数打印输出
	hThread[0] = CreateThread(NULL, 0, circle, (LPVOID)0, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, tangle, (LPVOID)0, 0, NULL);

	//等待所有线程结束
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	//删除临界区
	DeleteCriticalSection(&cs);
	char c = getchar();

	return 0;
}