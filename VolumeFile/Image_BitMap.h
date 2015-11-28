#pragma once

#include "Image_SuperBlock.h"

/*
	Part 4: 盘块位示图：256KiB
*/
class Image_BitMap
{
	public:

		//用于描述系统（BIOS与操作系统、文件系统超级块、InodeTable、位示图）盘块状态的字节数：256
		static const unsigned int SYSTEM_BLOCK_DESC = Image_SuperBlock::SYSTEM_BLOCK_NUM / 8;
		//用于描述全部盘块状态的字节数：262144
		static const unsigned int ALL_BLOCK_DESC = Image_SuperBlock::ALL_BLOCK_NUM / 8;
		//盘块搜索起始点初值
		static const unsigned int SEARCH_START_INIT = Image_SuperBlock::USER_BLOCK_START / 8;

		//读写内容
		char *content;

		//位示图定位：字节序号（0 ~ 262143）
		unsigned int byteLoc;
		//位示图定位：位序号（0 ~ 7）
		unsigned int bitLoc;
		//1个空闲盘块搜索起始点
		unsigned int SearchStart_1;
		//6个连续空闲盘块搜索起始点
		unsigned int SearchStart_6;

		//指向文件系统超级块的指针（这个值每次运行都会更新）
		Image_SuperBlock *super_block;

		//构造函数
		Image_BitMap()
		{
			/*
				申请长度为256KiB的空间，用构建盘块位示图的内存映像
			*/
			content = new char[ALL_BLOCK_DESC];
			if (content == nullptr)
			{
				cout << "无法申请用于存放盘块位示图映像的内存空间" << endl;
				system("pause");
				exit(-1);
			}
			//将系统（BIOS与操作系统、文件系统超级块、InodeTable、位示图）盘块设定为占用
			for (unsigned int i = 0; i <= SYSTEM_BLOCK_DESC - 1; i++)
			{
				content[i] = '\xFF';
			}
			//将所有用户数据盘块设为空闲
			for (unsigned int i = SYSTEM_BLOCK_DESC; i <= ALL_BLOCK_DESC - 1; i++)
			{
				content[i] = '\x00';
			}
			byteLoc = 0;
			bitLoc = 0;
			SearchStart_1 = SEARCH_START_INIT;
			SearchStart_6 = SEARCH_START_INIT;
		}

		//析构函数
		~Image_BitMap()
		{
			delete[]content;
		}

		//设定指向文件系统超级块的指针
		void SetSuperBlock(Image_SuperBlock *s_b)
		{
			super_block = s_b;
		}

		//占用编号为b的盘块
		void AllocateBlock(unsigned int b)
		{
			if (b <= SYSTEM_BLOCK_DESC - 1 || b >= ALL_BLOCK_DESC)
			{
				return;
			}
			byteLoc = b / 8;
			bitLoc = b % 8;
			switch (bitLoc)
			{
				case 0:
					content[byteLoc] = content[byteLoc] | 0x1;
					break;
				case 1:
					content[byteLoc] = content[byteLoc] | 0x2;
					break;
				case 2:
					content[byteLoc] = content[byteLoc] | 0x4;
					break;
				case 3:
					content[byteLoc] = content[byteLoc] | 0x8;
					break;
				case 4:
					content[byteLoc] = content[byteLoc] | 0x10;
					break;
				case 5:
					content[byteLoc] = content[byteLoc] | 0x20;
					break;
				case 6:
					content[byteLoc] = content[byteLoc] | 0x40;
					break;
				case 7:
					content[byteLoc] = content[byteLoc] | 0x80;
					break;
				default:
					break;
			}
		}

		/*
			释放编号为b的盘块
			内含保护措施：
				如果输入的盘块号对应用户数据区的盘块，那么释放该盘块。
				如果输入的盘块号非法，或者对应非用户数据区的盘块，那么不做任何操作。
		*/
		void ReleaseBlock(unsigned int b)
		{
			if (b <= SYSTEM_BLOCK_DESC - 1 || b >= ALL_BLOCK_DESC)
			{
				return;
			}
			byteLoc = b / 8;
			bitLoc = b % 8;
			switch (bitLoc)
			{
				case 0:
					content[byteLoc] = content[byteLoc] & 0xFE;
					break;
				case 1:
					content[byteLoc] = content[byteLoc] & 0xFD;
					break;
				case 2:
					content[byteLoc] = content[byteLoc] & 0xFB;
					break;
				case 3:
					content[byteLoc] = content[byteLoc] & 0xF7;
					break;
				case 4:
					content[byteLoc] = content[byteLoc] & 0xEF;
					break;
				case 5:
					content[byteLoc] = content[byteLoc] & 0xDF;
					break;
				case 6:
					content[byteLoc] = content[byteLoc] & 0xBF;
					break;
				case 7:
					content[byteLoc] = content[byteLoc] & 0x7F;
					break;
				default:
					break;
			}
		}

		/*
			搜索空闲盘块
			参数：
				search_6 - false:搜索1个空闲盘块，true:搜索6个连续空闲盘块，用于存放目录文件
			返回：
				不小于2048的整数 - 一个空闲盘块的编号
				0xFFFFFFFF       - 找不到空闲盘块
			采用“循环首次适应算法”,原因：
				1. 最大文件大小为16M，而存储容量为1G，也就是说没有什么所谓的“大文件”
				2. 在这种情况下，循环次数最少，查找效率最高
		*/
		unsigned int SearchEmptyBlock(bool search_6)
		{
			if (search_6)
			{
				//实际上，如果判定条件满足，那么自返回值起的8个盘块都是空闲的
				for (unsigned int i = SearchStart_6; i <= ALL_BLOCK_DESC - 1; i++)
				{
					if (content[i] == '\x00')
					{
						SearchStart_6 = i;
						return i * 8;
					}
				}
				for (unsigned int i = SEARCH_START_INIT; i <= SearchStart_6 - 1; i++)
				{
					if (content[i] == '\x00')
					{
						SearchStart_6 = i;
						return i * 8;
					}
				}
				return 0xFFFFFFFF;
			}
			else
			{
				for (unsigned int i = SearchStart_1; i <= ALL_BLOCK_DESC - 1; i++)
				{
					if (content[i] != '\xFF')
					{
						unsigned int r[8];					//用于提取字节中的特定位
						r[0] = content[i] & 0x01;			//最低位
						r[1] = (content[i] & 0x02) >> 1;
						r[2] = (content[i] & 0x04) >> 2;
						r[3] = (content[i] & 0x08) >> 3;
						r[4] = (content[i] & 0x10) >> 4;
						r[5] = (content[i] & 0x20) >> 5;
						r[6] = (content[i] & 0x40) >> 6;
						r[7] = (content[i] & 0x80) >> 7;	//最高位

						for (unsigned int j = 0; j <= 7; j++)
						{
							if (r[j] == 0)
							{
								SearchStart_1 = i;
								return i * 8 + j;
							}
						}
					}
				}
				for (unsigned int i = SEARCH_START_INIT; i <= SearchStart_1 - 1; i++)
				{
					if (content[i] != '\xFF')
					{
						unsigned int r[8];					//用于提取字节中的特定位
						r[0] = content[i] & 0x01;			//最低位
						r[1] = (content[i] & 0x02) >> 1;
						r[2] = (content[i] & 0x04) >> 2;
						r[3] = (content[i] & 0x08) >> 3;
						r[4] = (content[i] & 0x10) >> 4;
						r[5] = (content[i] & 0x20) >> 5;
						r[6] = (content[i] & 0x40) >> 6;
						r[7] = (content[i] & 0x80) >> 7;	//最高位

						for (unsigned int j = 0; j <= 7; j++)
						{
							if (r[j] == 1)
							{
								return i * 8 + j;
							}
						}
					}
				}
				return 0xFFFFFFFF;
			}
		}
};
