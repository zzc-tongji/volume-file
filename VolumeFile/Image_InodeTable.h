#pragma once

#include "Inode.h"

/*
	Part 3: Inode�ڵ��667.5KiB
*/
class Image_InodeTable
{
	public:

		//Inode�ڵ������ɵ�Inode�ڵ�������������򣺱��ļ�ϵͳ���������ɵ��ļ���������10680
		static const unsigned int MAX_INODE_NUM = Image_SuperBlock::BLOCK_CAP / Inode::INODE_SIZE * Image_SuperBlock::IT_BLOCK_NUM;

		//��д����
		Inode *content;

		//ָ���ļ�ϵͳ�������ָ�루���ֵÿ�����ж�����£�
		Image_SuperBlock *super_block;

		//���캯��
		Image_InodeTable()
		{
			content = new Inode[MAX_INODE_NUM];
			if (content == nullptr)
			{
				cout << "�޷��������ڴ�Ŵ��̡�Inode�ڵ��ӳ����ڴ�ռ�" << endl;
				system("pause");
				exit(-1);
			}
		}

		//��������
		~Image_InodeTable()
		{
			delete[]content;
		}

		//�趨ָ���ļ�ϵͳ�������ָ�루Image_InodeTable���� + ���������е�Inode�ڵ����
		void SetSuperBlockAll(Image_SuperBlock *s_b)
		{
			super_block = s_b;
			for (unsigned int i = 0; i <= MAX_INODE_NUM - 1; i++)
			{
				content[i].super_block = s_b;
			}
		}

		/*
			��ȡһ�����е�Inode
			���أ�
				0 ~ 10679  - ����Inode��ţ��ɹ�
				0xFFFFFFFF - Inode�ڵ���������޷����䣬ʧ��
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

		//��Inode�ڵ����ת��ΪInode��ָ��
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
