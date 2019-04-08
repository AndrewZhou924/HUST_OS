/*
实验三：
在 windows 环境下，利用高级语言编程环境（限定为 VS 环境或 VC 环境）调用 CreateThread 函数实现以下的功能:
创建 2 个线程 A 和 B。线程 A 在屏幕上用 while 循环顺序递增地输出 1 - 1000 的自然数；线程 B 在屏幕上
用 while 循环顺序递减地输出 1000 - 1 之间的自然数。为避免输出太快，每隔 0.5
秒输出一个数。
*/

#include<stdio.h>
#include<Windows.h>
#include<time.h>

//线程函数1，输出0-1000自然数
DWORD WINAPI ThreadA(LPVOID n) {
	int num = (int)n;
	int i = 0;
	time_t lasttime;
	while (i < num)
	{
		time(&lasttime);
		while (true)
		{
			time_t nowtime;
			time(&nowtime);
			if (nowtime - lasttime >= 0.5)
			{
				printf("In Thread A -> %d\n", i);
				i++;
				break;
			}
		}

	}
	printf("\n");
	return 0;
}

//线程函数2，输出1000-0的自然数
DWORD WINAPI ThreadB(LPVOID n) {
	int num = (int)n;
	int i = 1000;
	while (i > num)
	{
		printf("In Thread B -> %d\n", i);
		i--;
		Sleep(500);
	}
	printf("\n");
	return 0;
}


/*
HANDLE CreateThread(
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	DWORD dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags,
	LPDWORD lpThreadID );

参数的含义如下：
lpThreadAttrivutes： 指向SECURITY_ATTRIBUTES的指针，用于定义新线程的安全属性，一般设置成NULL；
dwStackSize： 分配以字节数表示的线程堆栈的大小，默认值是0；
lpStartAddress： 指向一个线程函数地址。每个线程都有自己的线程函数，线程函数是线程具体的执行代码；
lpParameter： 传递给线程函数的参数；
dwCreationFlags： 表示创建线程的运行状态，其中CREATE_SUSPEND表示挂起当前创建的线程，而0表示立即执行当前创建的进程；
lpThreadID： 返回新创建的线程的ID编号；
*/

int main()
{
	//定义线程句柄，我们可以通过句柄来操作线程
	HANDLE hThread[2];

	//创建线程，并调用函数打印输出
	hThread[0] = CreateThread(NULL, 0, ThreadA, (LPVOID)1000, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, ThreadB, (LPVOID)1, 0, NULL);

	//等待所有线程结束
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	//关闭线程句柄对象 
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	return 0;
}
