#pragma once

#include "Image_SuperBlock.h"

/*
	Part 1: BIOS�����ϵͳ��100KiB
*/
class Image_OS
{
	public:

		//��д����
		char *content;

		//���캯��
		Image_OS()
		{
			content = new char[Image_SuperBlock::OS_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP];
			if (content == nullptr)
			{
				cout << "�޷��������ڴ�Ŵ��̡�BIOS�����ϵͳ��ӳ����ڴ�ռ�" << endl;
				system("pause");
				exit(-1);
			}
			for (unsigned int i = 0; i <= Image_SuperBlock::OS_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP - 1; i++)
			{
				content[i] = '\x11';
			}
		}

		//��������
		~Image_OS()
		{
			delete[]content;
		}
};
