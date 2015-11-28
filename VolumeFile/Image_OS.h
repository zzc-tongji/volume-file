#pragma once

#include "Image_SuperBlock.h"

/*
	Part 1: BIOS与操作系统：100KiB
*/
class Image_OS
{
	public:

		//读写内容
		char *content;

		//构造函数
		Image_OS()
		{
			content = new char[Image_SuperBlock::OS_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP];
			if (content == nullptr)
			{
				cout << "无法申请用于存放磁盘“BIOS与操作系统”映像的内存空间" << endl;
				system("pause");
				exit(-1);
			}
			for (unsigned int i = 0; i <= Image_SuperBlock::OS_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP - 1; i++)
			{
				content[i] = '\x11';
			}
		}

		//析构函数
		~Image_OS()
		{
			delete[]content;
		}
};
