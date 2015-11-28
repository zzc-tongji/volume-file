#pragma once

#include "Inode.h"

/*
	Part 3: Inode节点表：667.5KiB
*/
class Image_InodeTable
{
	public:

		//Inode节点表可容纳的Inode节点的最大数量（亦或：本文件系统最多可以容纳的文件个数）：10680
		static const unsigned int MAX_INODE_NUM = Image_SuperBlock::BLOCK_CAP / Inode::INODE_SIZE * Image_SuperBlock::IT_BLOCK_NUM;

		//读写内容
		Inode *content;

		//指向文件系统超级块的指针（这个值每次运行都会更新）
		Image_SuperBlock *super_block;

		//构造函数
		Image_InodeTable()
		{
			content = new Inode[MAX_INODE_NUM];
			if (content == nullptr)
			{
				cout << "无法申请用于存放磁盘“Inode节点表”映像的内存空间" << endl;
				system("pause");
				exit(-1);
			}
		}

		//析构函数
		~Image_InodeTable()
		{
			delete[]content;
		}

		//设定指向文件系统超级块的指针（Image_InodeTable对象 + 其下属所有的Inode节点对象）
		void SetSuperBlockAll(Image_SuperBlock *s_b)
		{
			super_block = s_b;
			for (unsigned int i = 0; i <= MAX_INODE_NUM - 1; i++)
			{
				content[i].super_block = s_b;
			}
		}

		/*
			获取一个空闲的Inode
			返回：
				0 ~ 10679  - 空闲Inode编号，成功
				0xFFFFFFFF - Inode节点表已满，无法分配，失败
		*/
		unsigned int AllocateEmptyInode()
		{
			for (unsigned int i = 0; i <= MAX_INODE_NUM - 1; i++)
			{
				if (content[i].GetMode(0) == false)
				{
					return i;
				}
			}
			return 0xFFFFFFFF;
		}

		//将Inode节点序号转化为Inode的指针
		Inode* ToAddress(unsigned int no)
		{
			if (no >= 0 && no <= MAX_INODE_NUM - 1)
			{
				return content + no;
			}
			else
			{
				return nullptr;
			}
		}
};
