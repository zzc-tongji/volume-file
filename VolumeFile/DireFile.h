#pragma once
#pragma warning(disable:4996)

#include "Image_InodeTable.h"

/*
	目录项
*/
class DireEntry
{
	public:

		//一个目录项所占据的空间大小：32（务必确保！）
		static const unsigned int DIRECTORY_ENTRY_SIZE = 32;
		//目录项中路径部分的最大字符串长度：28
		static const unsigned int MAX_FILE_NAME_SIZE = 28;

		//目录项中Inode编号部分
		unsigned int m_ino;
		//目录项中路径名部分
		char m_name[MAX_FILE_NAME_SIZE];

		//构造函数
		DireEntry()
		{
			m_ino = -1;
			m_name[0] = '\0';
		}

		//析构函数
		~DireEntry()
		{

		}
};

/*
	目录文件读写器
*/
class DireFile
{
	public:

		//一个目录文件固定占用盘块数：6
		static const unsigned int FIXED_SIZE_BLOCK = 6;
		//一个目录文件的目录项的最大数量：96
		static const unsigned int DIRECTORY_ENTRY_NUM = Image_SuperBlock::BLOCK_CAP / DireEntry::DIRECTORY_ENTRY_SIZE * FIXED_SIZE_BLOCK;

		//指向文件系统超级块的指针（这个值每次运行都会更新）
		Image_SuperBlock *super_block;

		//该目录文件的Inode在Inode索引表中的位置
		unsigned int inode_no;
		//该目录文件的Inode的地址
		Inode *inode_pt;
		//该目录文件占用的首个“物理盘块”编号
		unsigned int phy_blk;
		//该目录文件的编辑位置
		DireEntry *content;

		//当前正在编辑的子文件的目录项编号
		unsigned int directory_no_child;
		//当前正在编辑的子文件的Inode在Inode索引表中的位置
		unsigned int inode_no_child;
		//当前正在编辑的子文件的Inode的地址
		Inode *inode_pt_child;
		//当前正在编辑的子文件占用的首个“物理盘块”编号
		unsigned int phy_blk_child;
		//如果子文件是目录文件，那么下列变量表示子文件的编辑位置
		DireEntry *content_child;

		//构造函数
		DireFile()
		{
			super_block = nullptr;
			inode_no = 0;
			inode_pt = nullptr;
			phy_blk = 0;
			content = nullptr;
			directory_no_child = 0;
			inode_no_child = 0;
			inode_pt_child = nullptr;
			phy_blk_child = 0;
			content_child = nullptr;
		}

		//析构函数
		~DireFile()
		{

		}

		/*
			目录文件读写准备
			参数：
				s_b - 指向文件系统超级块的指针
				ino - 该目录文件在Inode索引表中的位置
		*/
		void Prepare(Image_SuperBlock *s_b, unsigned ino)
		{
			//super_block
			super_block = s_b;
			//inode_no
			inode_no = ino;
			//inode_pt
			inode_pt = super_block->inode_table->ToAddress(inode_no);
			//phy_blk
			phy_blk = inode_pt->addr[0];
			//content
			content = (DireEntry *)(super_block->user_data->ToAddress(phy_blk));
		}

		/*
			新建空数据文件或空目录文件
			参数：
				s   - 待新建文件或目录的名称
				dir - 待新建的是否为目录
			返回：
				1          - 成功
				0xFFFFFFFF - 该目录文件的容量已达上限，无法添加目录项，失败
				0xFFFFFFFE - Inode节点表已满，无法新建文件，失败
				0xFFFFFFFD - “磁盘”已满，无法新建文件，失败
				0xFFFFFFFB - 该目录下存在同名的文件或目录，失败
		*/
		unsigned int CreateFileDir(string s, bool dir)
		{
			const char *c = s.c_str();
			//检查是否有同名数据文件或目录文件
			for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//从有定义的目录项中搜索
				if (content[i].m_ino != 0xFFFFFFFF)
				{
					//名称是否相同
					if (strcmp(content[i].m_name, c) == 0)
					{
						//类型（数据文件/目录文件）是否相同
						if ((super_block->inode_table->content[content[i].m_ino]).GetModeDirectory() == dir)
						{
							return 0xFFFFFFFB;
						}
					}
				}
			}
			//开始新建文件操作
			for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//如果找到空的目录项
				if (content[i].m_ino == 0xFFFFFFFF)
				{
					//设定即将分配的目录项的编号
					directory_no_child = i;
					//为需要新建的文件分配一个空的Inode节点
					inode_no_child = super_block->inode_table->AllocateEmptyInode();
					if (inode_no_child == 0xFFFFFFFF)
					{
						return 0xFFFFFFFE;
					}
					inode_pt_child = super_block->inode_table->ToAddress(inode_no_child);
					inode_pt_child->Occupy();
					//针对目录文件和数据文件的不同处理
					if (dir)
					{
						//设定标志位
						inode_pt_child->SetModeDirectory(true);
						//分配“物理盘块”
						if (inode_pt_child->RedistributeBlock(FIXED_SIZE_BLOCK) == 0xFFFFFFFF)
						{
							return 0xFFFFFFFD;
						}
						//目录文件读写准备（对新建的目录文件）
						phy_blk_child = inode_pt_child->addr[0];
						content_child = (DireEntry *)(super_block->user_data->ToAddress(phy_blk_child));
						//初始化操作——所有m_ino设置为0xFFFFFFFF（否则后续操作会出错）
						for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
						{
							content_child[i].m_ino = 0xFFFFFFFF;
						}
						//新建目录项：“.”
						content_child[0].m_ino = inode_no_child;
						strcpy(content_child[0].m_name, ".");
						//新建目录项：“..”
						content_child[1].m_ino = inode_no;
						strcpy(content_child[1].m_name, "..");
					}
					else
					{
						//设定标志位
						inode_pt_child->SetModeDirectory(false);
						//分配“物理盘块”
						if (inode_pt_child->RedistributeBlock(0) == 0xFFFFFFFF)
						{
							return 0xFFFFFFFD;
						}
					}
					//将已经新建的文件登记到目录项表中
					content[directory_no_child].m_ino = inode_no_child;
					strcpy(content[directory_no_child].m_name, c);
					//更新访问时间
					super_block->inode_table->content[inode_no].UpdateAtime();
					return 1;
				}
			}
			return 0xFFFFFFFF;
		}

		/*
			删除数据文件或空目录文件
			参数：
				s   - 待删除文件或目录的名称
				dir - 待删除的是否为目录
			返回：
				1          - 成功
				0xFFFFFFFE - 找不到文件，失败
				0xFFFFFFFD - 需要删除的目录文件不为空，失败
		*/
		unsigned int RemoveFileDir(string s, bool dir)
		{
			const char *c = s.c_str();
			//开始删除文件操作
			for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//在非空目录项中搜索
				if (content[i].m_ino != 0xFFFFFFFF)
				{
					//设定待回收目录项的编号
					directory_no_child = i;
					//设定待回收文件的Inode节点
					inode_no_child = content[directory_no_child].m_ino;
					inode_pt_child = super_block->inode_table->ToAddress(inode_no_child);
					//文件名称是否符合
					if (strcmp(c, content[directory_no_child].m_name) != 0)
					{
						continue;
					}
					//文件类型（数据文件/目录文件）是否符合
					if (inode_pt_child->GetModeDirectory() != dir)
					{
						continue;
					}
					//待删除的是目录文件
					if (dir)
					{
						phy_blk_child = inode_pt_child->addr[0];
						content_child = (DireEntry *)(super_block->user_data->ToAddress(phy_blk_child));
						//判断目录文件是否为空（空的目录文件只仅含有“.”“..”两个目录项的目录文件）
						unsigned int temp = 0;
						for (unsigned int j = 0; j <= DIRECTORY_ENTRY_NUM - 1; j++)
						{
							if (content_child[j].m_ino != 0xFFFFFFFF)
							{
								temp += 1;
							}
						}
						if (temp > 2)
						{
							return 0xFFFFFFFD;
						}
					}
					//释放该文件占据的所有“物理盘块”（包括文件内容和索引表）
					inode_pt_child->RedistributeBlock(0);
					//释放该文件的Inode节点
					inode_pt_child->Release();
					//回收目录项
					content[directory_no_child].m_ino = 0xFFFFFFFF;
					//更新访问时间
					super_block->inode_table->content[inode_no].UpdateAtime();
					return 1;
				}
			}
			return 0xFFFFFFFE;
		}

		/*
			查找该目录下的文件
			参数：
				s   - 待查找文件或目录的名称
				dir - 待查找的是否为目录
			返回：
				0 ~ 10680  - 文件在Inode节点表中的编号，成功
				0xFFFFFFFE - 找不到文件，失败
		*/
		unsigned int SearchFile(string s, bool dir)
		{
			const char *c = s.c_str();
			//开始查找文件操作
			for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//在非空目录项中搜索
				if (content[i].m_ino != 0xFFFFFFFF)
				{
					//设定待比对目录项的编号
					directory_no_child = i;
					//设定待比对文件的Inode节点
					inode_no_child = content[directory_no_child].m_ino;
					inode_pt_child = super_block->inode_table->ToAddress(inode_no_child);
					//文件名称是否符合
					if (strcmp(c, content[directory_no_child].m_name) != 0)
					{
						continue;
					}
					//文件类型（数据文件/目录文件）是否符合
					if (inode_pt_child->GetModeDirectory() != dir)
					{
						continue;
					}
					return inode_no_child;
				}
			}
			return 0xFFFFFFFE;
		}
};
