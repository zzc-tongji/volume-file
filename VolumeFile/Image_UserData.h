#pragma once

#include "Image_SuperBlock.h"

/*
	Part 5: �û����ݣ�1023MiB
*/
class Image_UserData
{
	public:

		//��д����
		char *content;

		//ָ���ļ�ϵͳ�������ָ�루���ֵÿ�����ж�����£�
		Image_SuperBlock *super_block;

		//���캯��
		Image_UserData()
		{
			content = new char[Image_SuperBlock::USER_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP];
			if (content == nullptr)
			{
				cout << "�޷��������ڴ���û�����ӳ����ڴ�ռ�" << endl;
				system("pause");
				exit(-1);
			}
		}

		//��������
		~Image_UserData()
		{
			delete[]content;
		}

		//�趨ָ���ļ�ϵͳ�������ָ��
		void SetSuperBlock(Image_SuperBlock *s_b)
		{
			super_block = s_b;
		}

		//���̿��ת��Ϊ�ڴ�ӳ���ָ��
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
