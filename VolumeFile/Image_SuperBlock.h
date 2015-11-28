#pragma once

#include "stdafx.h"

//用到指针，提前声明
class Image_InodeTable;
class Image_BitMap;
class Image_UserData;

/*
	Part 2: 文件系统超级块：0.5KiB
*/
class Image_SuperBlock
{
	public:

		//一个盘块的容量：512
		static const unsigned int BLOCK_CAP = 512;
		//盘块总数：2097152（共1GB的存储空间）
		static const unsigned int ALL_BLOCK_NUM = 2 * 1024 * 1024;

		//存放BIOS和操作系统的起始盘块号：0
		static const unsigned int OS_BLOCK_START = 0;
		//存放BIOS和操作系统的盘块数：200
		static const unsigned int OS_BLOCK_NUM = 200;

		//存放Image_SuperBlock的起始盘块号：200
		static const unsigned int SB_BLOCK_START = OS_BLOCK_START + OS_BLOCK_NUM;
		//存放Image_SuperBlock的盘块数：1
		static const unsigned int SB_BLOCK_NUM = 1;

		//存放Inode的起始盘块号：201
		static const unsigned int IT_BLOCK_START = SB_BLOCK_START + SB_BLOCK_NUM;
		//存放Inode的盘块数：1335（可以存放10680个文件或文件夹）
		static const unsigned int IT_BLOCK_NUM = 1335;

		//存放盘块位视图的起始盘块号：1536
		static const unsigned int BM_BLOCK_START = IT_BLOCK_START + IT_BLOCK_NUM;
		//存放盘块位视图的盘块数：512
		static const unsigned int BM_BLOCK_NUM = 512;

		//存放系统数据的起始盘块号：0
		static const unsigned int SYSTEM_BLOCK_START = 0;
		//存放系统数据的盘块数：2048
		static const unsigned int SYSTEM_BLOCK_NUM = OS_BLOCK_NUM + SB_BLOCK_NUM + IT_BLOCK_NUM + BM_BLOCK_NUM;

		//存放用户数据的起始盘块号：2048
		static const unsigned int USER_BLOCK_START = SYSTEM_BLOCK_START + SYSTEM_BLOCK_NUM;
		//存放用户数据的盘块数：2095104
		static const unsigned int USER_BLOCK_NUM = ALL_BLOCK_NUM - SYSTEM_BLOCK_NUM;

		//指向Image对象.inode_table对象的指针（这个值每次运行都会更新）
		Image_InodeTable *inode_table;
		//指向Image对象.bit_map对象的指针（这个值每次运行都会更新）
		Image_BitMap *bit_map;
		//指向Image对象.user_data对象的指针（这个值每次运行都会更新）
		Image_UserData *user_data;

		//工作路径：桌面\VolumeFile
		char *work_path;
		//“磁盘”路径：桌面\VolumeFile\Disk.dat
		char *disk_path;
		//“数据文件读写缓冲区”路径：桌面\VolumeFile\editor.txt
		char *editor_path;

		//填充
		unsigned int content[122];

		//构造函数
		Image_SuperBlock()
		{
			inode_table = nullptr;
			bit_map = nullptr;
			user_data = nullptr;

			for (unsigned int i = 0; i <= 124; i++)
			{
				content[i] = 0x22222222;
			}
		}

		//析构函数
		~Image_SuperBlock()
		{

		}

		//设定工作目录
		void SetWorkPath(char *w_p, char *d_p, char *e_p)
		{
			work_path = w_p;
			disk_path = d_p;
			editor_path = e_p;
		}
};
