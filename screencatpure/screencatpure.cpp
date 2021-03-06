// screencatpure.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <stdio.h>

int CaptureImage(HWND hwnd, CHAR *filename);

int main(int argc, char* argv[])
{
	if (argc != 2) {
		return 0;
	}

	remove(argv[1]);
	CaptureImage(GetDesktopWindow(), argv[1]);
}

int CaptureImage(HWND hwnd, CHAR *filename)
{
	HANDLE hDIB;
	//HANDLE hFile;
	FILE *fp = NULL;
	DWORD dwBmpSize;
	DWORD dwSizeofDIB;
	DWORD dwBytesWritten;
	CHAR FilePath[MAX_PATH];
	HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bi;
	CHAR *lpbitmap;
	INT width = GetSystemMetrics(SM_CXSCREEN);  // 屏幕宽
	INT height = GetSystemMetrics(SM_CYSCREEN);  // 屏幕高
	HDC hdcScreen = GetDC(NULL); // 全屏幕DC
	HDC hdcMemDC = CreateCompatibleDC(hdcScreen); // 创建兼容内存DC

	if (!hdcMemDC)
	{
		printf("%s\r\n", "Error occurs when call  CreateCompatibleDC");
		goto done;
	}

	// 通过窗口DC 创建一个兼容位图
	hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);

	if (!hbmScreen)
	{
		printf("%s\r\n","Error occurs when call  CreateCompatibleBitmap");
		goto done;
	}

	// 将位图块传送到我们兼容的内存DC中
	SelectObject(hdcMemDC, hbmScreen);
	if (!BitBlt(
		hdcMemDC,    // 目的DC
		0, 0,        // 目的DC的 x,y 坐标
		width, height, // 目的 DC 的宽高
		hdcScreen,   // 来源DC
		0, 0,        // 来源DC的 x,y 坐标
		SRCCOPY))    // 粘贴方式
	{
		printf("%s\r\n", "BitBlt has failed");
		goto done;
	}

	// 获取位图信息并存放在 bmpScreen 中
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	// 在 32-bit Windows 系统上, GlobalAlloc 和 LocalAlloc 是由 HeapAlloc 封装来的
	// handle 指向进程默认的堆. 所以开销比 HeapAlloc 要大
	hDIB = GlobalAlloc(GHND, dwBmpSize);
	lpbitmap = (char *)GlobalLock(hDIB);

	// 获取兼容位图的位并且拷贝结果到一个 lpbitmap 中.
	GetDIBits(
		hdcScreen,  // 设备环境句柄
		hbmScreen,  // 位图句柄
		0,          // 指定检索的第一个扫描线
		(UINT)bmpScreen.bmHeight, // 指定检索的扫描线数
		lpbitmap,   // 指向用来检索位图数据的缓冲区的指针
		(BITMAPINFO *)&bi, // 该结构体保存位图的数据格式
		DIB_RGB_COLORS // 颜色表由红、绿、蓝（RGB）三个直接值构成
	);

	memset(FilePath, 0, MAX_PATH);
	strcpy_s(FilePath, MAX_PATH - 1, filename);

	// 创建一个文件来保存文件截图
	/*
	hFile = CreateFile(
		FilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	*/
	if (fopen_s(&fp, FilePath, "a+") != NULL) {
		printf("%s\r\n", "fopen has failed");
		goto done;
	}

	// 将 图片头(headers)的大小, 加上位图的大小来获得整个文件的大小
	dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// 设置 Offset 偏移至位图的位(bitmap bits)实际开始的地方
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	// 文件大小
	bmfHeader.bfSize = dwSizeofDIB;

	// 位图的 bfType 必须是字符串 "BM"
	bmfHeader.bfType = 0x4D42; //BM

	dwBytesWritten = 0;
	/*
	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
	*/

	fwrite((void *)&bmfHeader, sizeof(BITMAPFILEHEADER),1, fp);
	fwrite((void *)&bi, sizeof(BITMAPINFOHEADER), 1, fp);
	fwrite((void *)lpbitmap, dwBmpSize, 1, fp);

	// 解锁堆内存并释放
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	// 关闭文件句柄
	fclose(fp);

	// 清理资源
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);

	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
