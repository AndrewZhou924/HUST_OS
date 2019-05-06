#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <winbase.h>

//待测试运行时间的函数
void test() {
	int i = 0;
	while (i < 150000000) {
		i++;
	}
}

void method_1() {
	clock_t start = clock();
	//=================================================
	test(); //待测试函数
	//=================================================
	clock_t end = clock();
	double runtime = (double)(end - start) / CLOCKS_PER_SEC;
	printf("runtime in method_1 is %f\n", runtime);
}

void method_2() {
	LARGE_INTEGER BegainTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&BegainTime);
	//=================================================
	test(); //待测试函数
	//=================================================
	QueryPerformanceCounter(&EndTime);
	double runtime = (double)(EndTime.QuadPart - BegainTime.QuadPart) / Frequency.QuadPart;

	printf("runtime in method_2 is %f\n", runtime);
}

void method_3() {

	//返回毫秒数
	DWORD start_time = GetTickCount();
	//=================================================
	test(); //待测试函数
	//=================================================
	DWORD end_time = GetTickCount();

	printf("runtime in method_3 is %f\n", (double)(end_time - start_time)/1000);
}


int main() {
	method_1();
	Sleep(1000);

	method_2();
	Sleep(1000);

	method_3();
	getchar();
}
