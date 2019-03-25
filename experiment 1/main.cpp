/*
  操作系统原理上机一
  在 windows 环境下，利用高级语言编程环境（限定为 VS 环境或 VC 环境）
  调用相关的系统调用（即系统 API）实现一个包括“进程创建，文件读写”的应
  用程序。：
  有一个文本文件 CommandList.txt，第一行是说明性文字：本文件最后一次打开和运行日
  期是 20190325。第二行开始每行是一个可执行程序的名称（含路径）。
  编写一个应用程序能打开该文件，并顺序执行其中的每个程序，并更新文件第一行中的日期。
 */

#include <windows.h>
#include <string>
#pragma warning(disable:4996)

int main()
{
	char timeContent[100] = { 0 };
	char readBuffer[1000] = { 0 };
	char command[10][100] = { 0 };       //按行分开的命令
	int commandNum = 0;		             //命令个数

	DWORD dwRead = 0;
	DWORD dwWrite = 0;
	HANDLE hFile = CreateFile("./CommandList.txt", GENERIC_WRITE | GENERIC_READ, 0,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed!");
		CloseHandle(hFile);
		return -1;
	}

	//写入时间
	FILETIME createTime, lastAccessTime, lastWriteTime;
	GetFileTime(hFile, &createTime, &lastAccessTime, &lastWriteTime);
	SYSTEMTIME st;
	memset(&st, 0x0, sizeof(st));
	FileTimeToSystemTime(&lastWriteTime, &st);
	TIME_ZONE_INFORMATION tz;
	GetTimeZoneInformation(&tz);
	SYSTEMTIME localST;
	SystemTimeToTzSpecificLocalTime(&tz, &st, &localST);

	sprintf(timeContent, "last date:%4d%02d%02d\r\n", localST.wYear, localST.wMonth, localST.wDay);
	printf("last date:%4d%02d%02d\r\n", localST.wYear, localST.wMonth, localST.wDay);
	WriteFile(hFile, timeContent, strlen(timeContent), &dwWrite, NULL);

	//读取文本
	DWORD fileSize = GetFileSize(hFile, NULL);
	ReadFile(hFile, readBuffer, fileSize, &dwRead, NULL);
	readBuffer[fileSize] = '\0';
	//printf("%s", readBuffer);
	CloseHandle(hFile);

	//循环将读取的命令分隔开来
	char tmpBuffer[1000] = { 0 };
	for (int i = 0; i<fileSize; i++)
	{
		if ('\r' == readBuffer[i])
		{
			strncpy(command[commandNum], readBuffer, i);
			i = i + 2;
			commandNum++;
			strcpy(tmpBuffer, &readBuffer[i]);
			strcpy(readBuffer, tmpBuffer);
			i = 0;
		}
		else if ('\0' == readBuffer[i])
		{
			strncpy(command[commandNum], readBuffer, i);
			commandNum++;
			break;
		}
	}

	//循环创建进程
	for (int j = 0; j<commandNum; j++)
	{
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		si.dwFlags = STARTF_USESHOWWINDOW; // 指定wShowWindow成员有效
		si.wShowWindow = TRUE; // 此成员设为TRUE的话则显示新建进程的主窗
		BOOL bRet = CreateProcess(NULL,	// 不在此指定可执行文件的文件名
			command[j],// 命令行参数
			NULL,	// 默认进程安全性
			NULL,	// 默认进程安全性
			FALSE,	// 指定当前进程内句柄不可以被子进程继承
			CREATE_NEW_CONSOLE,	// 为新进程创建一个新的控制台窗口
			NULL,	// 使用本进程的环境变量
			NULL,	// 使用本进程的驱动器和目录
			&si, &pi);
		if (bRet)
		{ // 不使用的句柄最好关掉
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			//printf("\n新进程的ID号：%d", pi.dwProcessId);
			//printf("\n新进程的主线程ID号：%d\n", pi.dwThreadId);
		}
		Sleep(1000);
	}

	system("pause");
	return 0;
}
