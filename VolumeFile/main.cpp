// main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "FileSystem_UI.h"

int main(int argc, _TCHAR* argv[])
{
	cout << "---------------------" << endl;
	cout << " 文件系统正在启动... " << endl;
	cout << "---------------------" << endl;
	cout << endl;

	FileSystem_UI ui;
	ui.Run();

	cout << "---------------------" << endl;
	cout << " 文件系统已关闭！ " << endl;
	cout << "---------------------" << endl;
	cout << endl;

	system("pause");
	return 0;
}
