#pragma once

#include "Image_InodeTable.h"

/*
	数据文件读写器
*/
class DataFile
{
	public:

		//指向文件系统超级块的指针（这个值每次运行都会更新）
		Image_SuperBlock *super_block;

		//该数据文件的Inode在Inode节点表中的位置
		unsigned int inode_no;
		//该数据文件的Inode的地址
		Inode *inode_pt;
		//临时变量
		unsigned int *temp;
		//“物理盘块”编号
		unsigned int phy_blk;
		//读写点
		char *content;

		//“数据文件读写区”读写头
		fstream fs_editor;

		//构造函数
		DataFile()
		{
			super_block = nullptr;
			inode_no = 0;
			inode_pt = nullptr;
			temp = nullptr;
			phy_blk = 0xFFFFFFFF;
			content = nullptr;
		}

		//析构函数
		~DataFile()
		{

		}

		/*
			数据文件读写准备
			参数：
				s_b - 指向文件系统超级块的指针
				ino - 该目录文件在Inode索引表中的位置
		*/
		void Prepare(Image_SuperBlock *s_b, unsigned ino)
		{
			//image
			super_block = s_b;
			//inode_no
			inode_no = ino;
			//inode_pt
			inode_pt = super_block->inode_table->ToAddress(inode_no);
		}

		//开启“数据文件读写区”
		void FsEditorOpen(bool clean)
		{
			if (clean)
			{
				fs_editor.open(super_block->editor_path, ios::out | ios::binary);
			}
			else
			{
				fs_editor.open(super_block->editor_path, ios::in | ios::binary | ios::ate);
			}
		}

		//关闭“数据文件读写区”
		void FsEditorClose()
		{
			fs_editor.close();
		}

		//将“磁盘”上的一个数据文件“读出”到“数据文件读写器”，用户可以使用notepad进行查看和编辑。
		void Reading()
		{
			//开启并清空“数据文件读写区”
			FsEditorOpen(true);
			for (unsigned int i = 0; i <= (inode_pt->size_block) - 1; i++)
			{
				//对于编辑前为空的数据文件，不执行“读出”操作
				if ((inode_pt->size_block) - 1 == 0xFFFFFFFF)
				{
					break;
				}
				//获取存放数据文件的“物理盘块”的编号
				phy_blk = inode_pt->Bmap(i, temp);
				//获取相应的读写点
				content = super_block->user_data->ToAddress(phy_blk);
				if (i == (inode_pt->size_block) - 1)
				{
					//如果正在操作的是该数据文件的最后一个“物理盘块”,
					//那么需要首先算出文件剩余部分的长度（取余），然后将剩余部分“读出”。
					unsigned int surplus = 0;
					surplus = (inode_pt->size_byte) % Image_SuperBlock::BLOCK_CAP;
					if (surplus == 0)
					{
						surplus = Image_SuperBlock::BLOCK_CAP;
					}
					fs_editor.write(content, surplus);
				}
				else
				{
					//如果正在操作的不是该数据文件的最后一个“物理盘块”，
					//那么只需要将这个“物理盘块”整体“读出”。
					fs_editor.write(content, Image_SuperBlock::BLOCK_CAP);
				}
			}
			//关闭“数据文件读写区”
			FsEditorClose();
		}

		/*
			将“数据文件读写器”中的内容“写入”到“磁盘”上的一个数据文件
			返回：
				1          - 成功
				0xFFFFFFFF - 待“写入”的内容过长，超过文件最大大小，失败
		*/
		unsigned int Writing()
		{
			if (false)
			{
				//哈希校验码一致
				return 1;
			}
			//开启“数据文件读写区”
			FsEditorOpen(false);
			//重新分配数据文件所需的“物理盘块”
			if (fs_editor.tellg() > Inode::MAX_SIZE_BYTE)
			{
				return 0xFFFFFFFF;
			}
			inode_pt->RedistributeBlock((unsigned int)(fs_editor.tellg()));
			//读指针复位
			fs_editor.seekg(0);
			for (unsigned int i = 0; i <= (inode_pt->size_block) - 1; i++)
			{
				//对于编辑后为空的数据文件，不执行“写入”操作
				if ((inode_pt->size_block) - 1 == 0xFFFFFFFF)
				{
					break;
				}
				//获取存放数据文件的“物理盘块”的编号
				phy_blk = inode_pt->Bmap(i, temp);
				//获取相应的读写点
				content = super_block->user_data->ToAddress(phy_blk);
				if (i == (inode_pt->size_block) - 1)
				{
					//如果正在操作的是该数据文件的最后一个“物理盘块”,
					//那么需要首先算出文件剩余部分的长度（取余），然后将剩余部分“写入”。
					unsigned int surplus = 0;
					surplus = (inode_pt->size_byte) % Image_SuperBlock::BLOCK_CAP;
					if (surplus == 0)
					{
						surplus = Image_SuperBlock::BLOCK_CAP;
					}
					fs_editor.read(content, surplus);
				}
				else
				{
					//如果正在操作的不是该数据文件的最后一个“物理盘块”，
					//那么只需要将这个“物理盘块”整体“写入”。
					fs_editor.read(content, Image_SuperBlock::BLOCK_CAP);
				}
			}
			//更新数据文件的最后访问时间
			inode_pt->UpdateAtime();
			//关闭“数据文件读写区
			FsEditorClose();
			return 1;
		}
};