#pragma once

#include "stdafx.h"

//�õ�ָ�룬��ǰ����
class Image_InodeTable;
class Image_BitMap;
class Image_UserData;

/*
	Part 2: �ļ�ϵͳ�����飺0.5KiB
*/
class Image_SuperBlock
{
	public:

		//һ���̿��������512
		static const unsigned int BLOCK_CAP = 512;
		//�̿�������2097152����1GB�Ĵ洢�ռ䣩
		static const unsigned int ALL_BLOCK_NUM = 2 * 1024 * 1024;

		//���BIOS�Ͳ���ϵͳ����ʼ�̿�ţ�0
		static const unsigned int OS_BLOCK_START = 0;
		//���BIOS�Ͳ���ϵͳ���̿�����200
		static const unsigned int OS_BLOCK_NUM = 200;

		//���Image_SuperBlock����ʼ�̿�ţ�200
		static const unsigned int SB_BLOCK_START = OS_BLOCK_START + OS_BLOCK_NUM;
		//���Image_SuperBlock���̿�����1
		static const unsigned int SB_BLOCK_NUM = 1;

		//���Inode����ʼ�̿�ţ�201
		static const unsigned int IT_BLOCK_START = SB_BLOCK_START + SB_BLOCK_NUM;
		//���Inode���̿�����1335�����Դ��10680���ļ����ļ��У�
		static const unsigned int IT_BLOCK_NUM = 1335;

		//����̿�λ��ͼ����ʼ�̿�ţ�1536
		static const unsigned int BM_BLOCK_START = IT_BLOCK_START + IT_BLOCK_NUM;
		//����̿�λ��ͼ���̿�����512
		static const unsigned int BM_BLOCK_NUM = 512;

		//���ϵͳ���ݵ���ʼ�̿�ţ�0
		static const unsigned int SYSTEM_BLOCK_START = 0;
		//���ϵͳ���ݵ��̿�����2048
		static const unsigned int SYSTEM_BLOCK_NUM = OS_BLOCK_NUM + SB_BLOCK_NUM + IT_BLOCK_NUM + BM_BLOCK_NUM;

		//����û����ݵ���ʼ�̿�ţ�2048
		static const unsigned int USER_BLOCK_START = SYSTEM_BLOCK_START + SYSTEM_BLOCK_NUM;
		//����û����ݵ��̿�����2095104
		static const unsigned int USER_BLOCK_NUM = ALL_BLOCK_NUM - SYSTEM_BLOCK_NUM;

		//ָ��Image����.inode_table�����ָ�루���ֵÿ�����ж�����£�
		Image_InodeTable *inode_table;
		//ָ��Image����.bit_map�����ָ�루���ֵÿ�����ж�����£�
		Image_BitMap *bit_map;
		//ָ��Image����.user_data�����ָ�루���ֵÿ�����ж�����£�
		Image_UserData *user_data;

		//����·��������\VolumeFile
		char *work_path;
		//�����̡�·��������\VolumeFile\Disk.dat
		char *disk_path;
		//�������ļ���д��������·��������\VolumeFile\editor.txt
		char *editor_path;

		//���
		unsigned int content[122];

		//���캯��
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

		//��������
		~Image_SuperBlock()
		{

		}

		//�趨����Ŀ¼
		void SetWorkPath(char *w_p, char *d_p, char *e_p)
		{
			work_path = w_p;
			disk_path = d_p;
			editor_path = e_p;
		}
};
