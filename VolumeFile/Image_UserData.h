#pragma once

#include "Image_SuperBlock.h"

/*
	Part 5: 用户数据：1023MiB
*/
class Image_UserData
{
	public:

		//读写内容
		char *content;

		//指向文件系统超级块的指针（这个值每次运行都会更新）
		Image_SuperBlock *super_block;

		//构造函数
		Image_UserData()
		{
			content = new char[Image_SuperBlock::USER_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP];
			if (content == nullptr)
			{
				cout << "无法申请用于存放用户数据映像的内存空间" << endl;
				system("pause");
				exit(-1);
			}
		}

		//析构函数
		~Image_UserData()
		{
			delete[]content;
		}

		//设定指向文件系统超级块的指针
		void SetSuperBlock(Image_SuperBlock *s_b)
		{
			super_block = s_b;
		}

		//将盘块号转化为内存映像的指针
		char* ToAddress(unsigned int blk)
		{
			if (blk >= Image_SuperBlock::USER_BLOCK_START)
			{
				return content + (blk - Image_SuperBlock::USER_BLOCK_START) * Image_SuperBlock::BLOCK_CAP;
			}
			else
			{
				return nullptr;
			}
		}
};
