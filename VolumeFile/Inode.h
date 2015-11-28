#pragma once

#include "Image_SuperBlock.h"
#include "Image_BitMap.h"
#include "Image_UserData.h"

/*
	Inode
*/
class Inode
{
	public:

		//一个Inode节点所占据的空间大小：64（务必确保！）
		static const unsigned int INODE_SIZE = 64;

		//初始化时设置标志位
		static const unsigned short MODE_INIT = 0x0000;
		//占用时设置标志位（110 110 110 1 1 000 0 1）
		static const unsigned short MODE_OCC = 0xDB61;

		//UID设定值
		static const unsigned short UID_INIT = 1;
		//GID设定值
		static const unsigned short GID_INIT = 1;

		//一个索引盘块可容纳的索引的最大数量：128
		static const unsigned int MAX_INDEX_NUM = Image_SuperBlock::BLOCK_CAP / sizeof(unsigned int);
		//小型文件最大大小（块）：6
		static const unsigned int SMALL_MAX_SIZE_BLOCK = 6;
		//大型文件最大大小（块）：262
		static const unsigned int LARGE_MAX_SIZE_BLOCK = 6 + 2 * MAX_INDEX_NUM;
		//（巨型）文件最大大小（块）：33030
		static const unsigned int MAX_SIZE_BLOCK = 6 + 2 * MAX_INDEX_NUM + 2 * MAX_INDEX_NUM * MAX_INDEX_NUM;
		//（巨型）文件最大大小（字节）：16911360 Bit（16515 KiB，约16.12 MiB）
		static const unsigned int MAX_SIZE_BYTE = MAX_SIZE_BLOCK * Image_SuperBlock::BLOCK_CAP;

		/*
			状态标志位：[15]~[0]
			[15][14][13]：	文件所有者对文件的访问权（rwx，有权限-1）
			[12][11][10]：	文件所有者的同组其他用户对文件的访问权（rwx，有权限-1）
			[9][8][7]：		其他用户对文件的访问权（rwx，有权限-1）
			[6]：			uid表示文件所有者的用户标识数
			[5]：			gid表示文件所有者所在的用户组标识数
			[4][3][2]：		空文件：000，小型文件：001，大型文件：011，巨型文件：111
			[1]：			普通文件-0，目录文件-1
			[0]：			节点未分配-0，节点已分配-1

			ps：这里将顺序倒过来是为了提高搜索效率，因为后面的SetMode()、GetMode()都是从低位端开始选择的。
		*/
		unsigned short mode;
		unsigned short uid;				//文件所有者的用户标识数
		unsigned short gid;				//文件所有者的组标识数
		unsigned short padding;			//（占位）
		unsigned int size_byte;			//文件大小（字节）
		unsigned int size_block;		//文件大小（块，不足一块算一块）
		unsigned int atime;				//最后访问时间
		unsigned int addr[10];			//用于文件逻辑块号和物理块号转换的基本索引表
		Image_SuperBlock *super_block;	//指向文件系统超级块（这个值每次运行都会更新）

		//构造函数
		Inode()
		{
			Release();
			for (unsigned int i = 0; i < 10; i++)
			{
				addr[i] = -1;
			}
			super_block = nullptr;
		}

		//析构函数
		~Inode()
		{

		}

		//设定：是否为文件夹
		void SetModeDirectory(bool d)
		{
			if (d)
			{
				SetMode(1, true);
			}
			else
			{
				SetMode(1, false);
			}
		}

		//读取：是否为文件夹
		bool GetModeDirectory()
		{
			if (GetMode(1) == true)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		//计算size_block
		void CalculateSizeBlock()
		{
			size_block = (unsigned int)(ceil(double(size_byte) / double(Image_SuperBlock::BLOCK_CAP)));
		}

		//更新最后访问时间
		void UpdateAtime()
		{
			atime = (unsigned int)time(NULL);	//这里在90年之后会出现BUG（2014年8月11日）
		}

		/*
			占用节点的顺序：占用节点 -> 分配存放文件索引表的空间 -> 分配存放文件内容的空间
			放弃节点的顺序：释放存放文件内容的空间 -> 释放存放文件索引表的空间 -> 放弃节点

			切记！！顺序一定不能错！！
		*/

		//占用节点
		void Occupy()
		{
			mode = MODE_OCC;
			uid = UID_INIT;
			gid = GID_INIT;
			padding = 0;
			size_byte = 0;
			CalculateSizeBlock();
			UpdateAtime();
		}

		//释放节点
		void Release()
		{
			mode = MODE_INIT;
			uid = 0;
			gid = 0;
			padding = 0;
			size_byte = 0;
			CalculateSizeBlock();
			atime = 0;
		}

		/*
			重新分配文件盘块
			参数：
				s - 重新分配后的文件大小（字节，范围：0~16911360）
			返回：
				1          - 成功
				0xFFFFFFFF - “磁盘”已满，无法分配“物理盘块”，失败
			注意：
				如果这是个目录文件，那么s为6，输入无效！
		*/
		unsigned int RedistributeBlock(unsigned int s)
		{
			//释放盘块
			if (GetMode(2) == true)
			{
				ReleaseDataBlock();
			}
			if (GetMode(2) == true)
			{
				ReleaseIndexBlock05();
			}
			if (GetMode(3) == true)
			{
				ReleaseIndexBlock67();
			}
			if (GetMode(4) == true)
			{
				ReleaseIndexBlock89();
			}
			//调整文件大小
			if (GetModeDirectory())
			{
				size_byte = 3072;
			}
			else
			{
				size_byte = s;
			}
			CalculateSizeBlock();
			//修改相应标志位
			if (size_block == 0)
			{
				SetMode(2, false);
				SetMode(3, false);
				SetMode(4, false);
			}
			else if (size_block >= 1 && size_block <= SMALL_MAX_SIZE_BLOCK)
			{
				SetMode(2, true);
				SetMode(3, false);
				SetMode(4, false);
			}
			else if (size_block >= 7 && size_block <= LARGE_MAX_SIZE_BLOCK)
			{
				SetMode(2, true);
				SetMode(3, true);
				SetMode(4, false);
			}
			else if (size_block >= 263 && size_block <= MAX_SIZE_BLOCK)
			{
				SetMode(2, true);
				SetMode(3, true);
				SetMode(4, true);
			}
			//分配盘块
			if (GetMode(2) == true)
			{
				if (AllocateIndexBlock05() == 0xFFFFFFFF)
				{
					return 0xFFFFFFFF;
				}
			}
			if (GetMode(3) == true)
			{
				if (AllocateIndexBlock67() == 0xFFFFFFFF)
				{
					return 0xFFFFFFFF;
				}
			}
			if (GetMode(4) == true)
			{
				if (AllocateIndexBlock89() == 0xFFFFFFFF)
				{
					return 0xFFFFFFFF;
				}
			}
			if (GetMode(2) == true)
			{
				if (AllocateDataBlock() == 0xFFFFFFFF)
				{
					return 0xFFFFFFFF;
				}
			}
			return 1;
		}

		/*
			分配addr[0]~addr[5]的文件索引表盘块
			返回：
				1          - 成功
				0xFFFFFFFF - “磁盘”已满，无法分配“物理盘块”，失败
			注意：
				不包含更改状态标志位的操作！
				分配的是存放文件索引表的盘块，而不是存放文件内容的盘块！
		*/
		unsigned int AllocateIndexBlock05()
		{
			for (unsigned int i = 0; i <= 5; i++)
			{
				addr[i] = 0xFFFFFFFF;
			}
			return 1;
		}

		/*
			分配addr[6]~addr[7]的文件索引表盘块
			返回：
				1          - 成功
				0xFFFFFFFF - “磁盘”已满，无法分配“物理盘块”，失败
			注意：
				不包含更改状态标志位的操作！
				分配的是存放文件索引表的盘块，而不是存放文件内容的盘块！
		*/
		unsigned int AllocateIndexBlock67()
		{
			unsigned int temp = 0;
			unsigned int *addr_sub = nullptr;
			for (unsigned int i = 6; i <= 7; i++)
			{
				//addr[i]
				temp = super_block->bit_map->SearchEmptyBlock(false);
				if (temp == 0xFFFFFFFF)
				{
					return 0xFFFFFFFF;
				}
				super_block->bit_map->AllocateBlock(temp);
				addr[i] = temp;
				//addr[i]：一级
				addr_sub = (unsigned int *)(super_block->user_data->ToAddress(addr[i]));
				for (unsigned int j = 0; j <= MAX_INDEX_NUM - 1; j++)
				{
					addr_sub[j] = 0xFFFFFFFF;
				}
			}
			return 1;
		}

		/*
			分配addr[8]~addr[9]的文件索引表盘块
			返回：
				1          - 成功
				0xFFFFFFFF - “磁盘”已满，无法分配“物理盘块”，失败
			注意：
				不包含更改状态标志位的操作！
				分配的是存放文件索引表的盘块，而不是存放文件内容的盘块！
		*/
		unsigned int AllocateIndexBlock89()
		{
			unsigned int temp = 0;
			unsigned int *addr_sub = nullptr;
			unsigned int *addr_sub_sub = nullptr;
			for (unsigned int i = 8; i <= 9; i++)
			{
				//addr[i]
				temp = super_block->bit_map->SearchEmptyBlock(false);
				if (temp == 0xFFFFFFFF)
				{
					return 0xFFFFFFFF;
				}
				super_block->bit_map->AllocateBlock(temp);
				addr[i] = temp;
				//addr[i]：一级
				addr_sub = (unsigned int *)(super_block->user_data->ToAddress(addr[i]));
				for (unsigned int j = 0; j <= MAX_INDEX_NUM - 1; j++)
				{
					temp = super_block->bit_map->SearchEmptyBlock(false);
					if (temp == 0xFFFFFFFF)
					{
						return 0xFFFFFFFF;
					}
					super_block->bit_map->AllocateBlock(temp);
					addr_sub[j] = temp;
					//addr[8]：二级
					addr_sub_sub = (unsigned int *)(super_block->user_data->ToAddress(addr_sub[j]));
					for (unsigned int k = 0; k <= MAX_INDEX_NUM - 1; k++)
					{
						addr_sub_sub[k] = 0xFFFFFFFF;
					}
				}
			}
			return 1;
		}

		/*
			申请存放文件数据的盘块
			返回：
				1           - 成功
				0xFFFFFFFF  - “磁盘”已满，无法分配“物理盘块”，失败
			注意：
				不包含更改状态标志位的操作！
				不包含申请索引表盘块的操作！文件索引表盘块必须在执行本函数之前申请好，否则会出现严重错误！
		*/
		unsigned int AllocateDataBlock()
		{
			unsigned int temp_v;
			unsigned int temp_v2;
			unsigned int *temp_p;
			bool dir;
			dir = GetModeDirectory();
			if (dir)
			{
				temp_v2 = super_block->bit_map->SearchEmptyBlock(true);
			}
			if (size_block != 0)
			{
				for (unsigned int i = 0; i <= size_block - 1; i++)
				{
					temp_v = Bmap(i, temp_p);
					if (dir)
					{
						super_block->bit_map->AllocateBlock(temp_v2 + i);
						*temp_p = temp_v2 + i;
					}
					else
					{
						temp_v = super_block->bit_map->SearchEmptyBlock(false);
						if (temp_v == 0xFFFFFFFF)
						{
							return 0xFFFFFFFF;
						}
						super_block->bit_map->AllocateBlock(temp_v);
						*temp_p = temp_v;
					}
				}
			}
			return 1;
		}

		/*
			释放存放文件数据的盘块
			注意：
				不包含更改状态标志位的操作！
				在执行本函数之后，才能释放文件索引表盘块，否则将造成存储空间泄露！
		*/
		void ReleaseDataBlock()
		{
			if (size_block != 0)
			{
				unsigned int temp_v = 0;
				unsigned int *temp_p = nullptr;
				for (unsigned int i = 0; i <= size_block - 1; i++)
				{
					temp_v = Bmap(i, temp_p);
					super_block->bit_map->ReleaseBlock(temp_v);
					*temp_p = -1;
				}
			}
		}

		/*
			释放addr[0]~addr[5]的文件索引表盘块
			注意：
				不包含更改状态标志位的操作！
				释放的是存放文件索引表的盘块，而不是存放文件内容的盘块！
		*/
		void ReleaseIndexBlock05()
		{
			for (unsigned int i = 0; i <= 5; i++)
			{
				addr[i] = -1;
			}
		}

		/*
			释放addr[6]~addr[7]的文件索引表盘块
			注意：
				不包含更改状态标志位的操作！
				释放的是存放文件索引表的盘块，而不是存放文件内容的盘块！
		*/
		void ReleaseIndexBlock67()
		{
			for (unsigned int i = 6; i <= 7; i++)
			{
				super_block->bit_map->ReleaseBlock(addr[i]);
				addr[i] = -1;
			}
		}

		/*
			释放addr[8]~addr[9]的文件索引表盘块
			注意：
				不包含更改状态标志位的操作！
				释放的是存放文件索引表的盘块，而不是存放文件内容的盘块！
		*/
		void ReleaseIndexBlock89()
		{
			unsigned int temp_v = -1;
			unsigned int *temp_p = nullptr;
			for (unsigned int i = 8; i <= 9; i++)
			{
				temp_p = (unsigned int *)(super_block->user_data->ToAddress(addr[i]));
				for (unsigned int j = 1; j <= MAX_INDEX_NUM - 1; j++)
				{
					super_block->bit_map->ReleaseBlock(temp_p[j]);
				}
				super_block->bit_map->ReleaseBlock(addr[i]);
				addr[i] = -1;
			}
		}

		/*
			将逻辑盘块号转化为物理盘块号
			参数：
				lbn - 逻辑盘块号
				p - 存放物理盘块号的无符号整数的地址（输出）
			返回：
				2048 ~ 1097151 - 物理盘块号；
			                     此时，pointer为存放物理盘块号的无符号整数的地址。
				0xFFFFFFFF     - 这个逻辑盘块号暂未对应物理盘块号（在申请文件内容盘块是会出现这一情况）；
			                     此时，pointer为存放物理盘块号的无符号整数的地址。
			强制退出：
				如果计算结果不对应存放用户数据的“物理盘块”，那么输出错误信息并且强制退出。
		*/
		unsigned int Bmap(unsigned int lbn, unsigned int *(&pointer))
		{
			unsigned int r0, r1, r2;
			pointer = nullptr;
			//小型文件
			if (lbn < 6)
			{
				BmapFunc0(lbn, r0);
				pointer = addr + r0;
			}
			//大型文件
			else if (lbn < 6 + 2 * MAX_INDEX_NUM)
			{
				BmapFunc1(lbn, r0, r1);
				pointer = addr + r0;
				pointer = (unsigned int*)(super_block->user_data->ToAddress(*pointer));
				pointer += r1;
			}
			//巨型文件
			else
			{
				BmapFunc2(lbn, r0, r1, r2);
				pointer = addr + r0;
				pointer = (unsigned int*)(super_block->user_data->ToAddress(*pointer));
				pointer += r1;
				pointer = (unsigned int*)(super_block->user_data->ToAddress(*pointer));
				pointer += r2;
			}
			//如果计算结果不对应存放用户数据的“物理盘块”，那么输出错误信息并且强制退出。
			if (*pointer < Image_SuperBlock::USER_BLOCK_START || *pointer >= Image_SuperBlock::ALL_BLOCK_NUM)
			{
				if (*pointer != 0xFFFFFFFF)
				{
					cout << "错误！" << endl;
					cout << "Bmap算法不正确。" << endl;
					cout << endl;
					cout << "程序退出..." << endl;
					system("pause");
					exit(-1);
				}
			}
			return *pointer;
		}

		//Bmap：直接索引辅助函数
		void BmapFunc0(unsigned int l, unsigned int &r0)
		{
			r0 = l;
		}

		//Bmap：一级索引辅助函数
		void BmapFunc1(unsigned int l, unsigned int &r0, unsigned int &r1)
		{
			if (l < 6 + MAX_INDEX_NUM)
			{
				r0 = 6;
			}
			else
			{
				r0 = 7;
			}
			r1 = l - (6 + (r0 - 6) * MAX_INDEX_NUM);
		}

		//Bmap：二级索引辅助函数
		void BmapFunc2(unsigned int l, unsigned int &r0, unsigned int &r1, unsigned int &r2)
		{
			if (l < 6 + 2 * MAX_INDEX_NUM + MAX_INDEX_NUM * MAX_INDEX_NUM)
			{
				r0 = 8;
			}
			else
			{
				r0 = 9;
			}
			r1 = (l - (6 + 2 * MAX_INDEX_NUM + (r0 - 8) * MAX_INDEX_NUM * MAX_INDEX_NUM)) / MAX_INDEX_NUM;
			r2 = l - (6 + 2 * MAX_INDEX_NUM + (r0 - 8) * MAX_INDEX_NUM * MAX_INDEX_NUM) - r1 * MAX_INDEX_NUM;
		}

		/*
			对mode进行按位读取
			参数：
				bit - 需要输出的位
			返回：
				true  - 1
				false - 0
			TIPS:
				如果输入的bit大于15，那么一律返回false

			状态标志位：[15]~[0]
			[15][14][13]：	文件所有者对文件的访问权（rwx，有权限-1）
			[12][11][10]：	文件所有者的同组其他用户对文件的访问权（rwx，有权限-1）
			[9][8][7]：		其他用户对文件的访问权（rwx，有权限-1）
			[6]：			uid表示文件所有者的用户标识数
			[5]：			gid表示文件所有者所在的用户组标识数
			[4][3][2]：		空文件：000，小型文件：001，大型文件：011，巨型文件：111
			[1]：			普通文件-0，目录文件-1
			[0]：			节点未分配-0，节点已分配-1

			执行Occupy()后的取值情况为（左高右低）：110 110 110 1 1 000 0 1
		*/
		bool GetMode(unsigned int bit)
		{
			if (bit < 0 || bit > 15)
			{
				return 0;
			}
			unsigned short temp = 0x0001;
			for (unsigned int i = 1; i <= bit; i++)
			{
				temp = temp << 1;
			}
			if ((mode & temp) == 0)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}

		/*
			对mode进行按位写入
			参数：
				bit - 需要操作的位
				set - true:置1，false:置0
			TIPS：
				如果输入的bit大于15，那么设定无效

			状态标志位：[15]~[0]
			[15][14][13]：	文件所有者对文件的访问权（rwx，有权限-1）
			[12][11][10]：	文件所有者的同组其他用户对文件的访问权（rwx，有权限-1）
			[9][8][7]：		其他用户对文件的访问权（rwx，有权限-1）
			[6]：			uid表示文件所有者的用户标识数
			[5]：			gid表示文件所有者所在的用户组标识数
			[4][3][2]：		空文件：000，小型文件：001，大型文件：011，巨型文件：111
			[1]：			普通文件-0，目录文件-1
			[0]：			节点未分配-0，节点已分配-1

			执行Occupy()后的取值情况为（左高右低）：110 110 110 1 1 000 0 1
		*/
		void SetMode(unsigned int bit, bool set)
		{
			if (bit < 0 || bit > 15)
			{
				return;
			}
			else
			{
				unsigned short temp = 0x0001;
				for (unsigned int i = 1; i <= bit; i++)
				{
					temp = temp << 1;
				}
				if (set)
				{
					mode = mode | temp;
				}
				else
				{
					mode = mode & (0xFFFF - temp);
				}
			}
		}
};
