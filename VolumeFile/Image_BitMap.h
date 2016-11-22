#pragma once

#include "Image_SuperBlock.h"

/*
	Part 4: �̿�λʾͼ��256KiB
*/
class Image_BitMap
{
	public:

		//��������ϵͳ��BIOS�����ϵͳ���ļ�ϵͳ�����顢InodeTable��λʾͼ���̿�״̬���ֽ�����256
		static const unsigned int SYSTEM_BLOCK_DESC = Image_SuperBlock::SYSTEM_BLOCK_NUM / 8;
		//��������ȫ���̿�״̬���ֽ�����262144
		static const unsigned int ALL_BLOCK_DESC = Image_SuperBlock::ALL_BLOCK_NUM / 8;
		//�̿�������ʼ���ֵ
		static const unsigned int SEARCH_START_INIT = Image_SuperBlock::USER_BLOCK_START / 8;

		//��д����
		char *content;

		//λʾͼ��λ���ֽ���ţ�0 ~ 262143��
		unsigned int byteLoc;
		//λʾͼ��λ��λ��ţ�0 ~ 7��
		unsigned int bitLoc;
		//1�������̿�������ʼ��
		unsigned int SearchStart_1;
		//6�����������̿�������ʼ��
		unsigned int SearchStart_6;

		//ָ���ļ�ϵͳ�������ָ�루���ֵÿ�����ж�����£�
		Image_SuperBlock *super_block;

		//���캯��
		Image_BitMap()
		{
			/*
				���볤��Ϊ256KiB�Ŀռ䣬�ù����̿�λʾͼ���ڴ�ӳ��
			*/
			content = new char[ALL_BLOCK_DESC];
			if (content == nullptr)
			{
				cout << "�޷��������ڴ���̿�λʾͼӳ����ڴ�ռ�" << endl;
				system("pause");
				exit(-1);
			}
			//��ϵͳ��BIOS�����ϵͳ���ļ�ϵͳ�����顢InodeTable��λʾͼ���̿��趨Ϊռ��
			for (unsigned int i = 0; i <= SYSTEM_BLOCK_DESC - 1; i++)
			{
				content[i] = '\xFF';
			}
			//�������û������̿���Ϊ����
			for (unsigned int i = SYSTEM_BLOCK_DESC; i <= ALL_BLOCK_DESC - 1; i++)
			{
				content[i] = '\x00';
			}
			byteLoc = 0;
			bitLoc = 0;
			SearchStart_1 = SEARCH_START_INIT;
			SearchStart_6 = SEARCH_START_INIT;
		}

		//��������
		~Image_BitMap()
		{
			delete[]content;
		}

		//�趨ָ���ļ�ϵͳ�������ָ��
		void SetSuperBlock(Image_SuperBlock *s_b)
		{
			super_block = s_b;
		}

		//ռ�ñ��Ϊb���̿�
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
			�ͷű��Ϊb���̿�
			�ں�������ʩ��
				���������̿�Ŷ�Ӧ�û����������̿飬��ô�ͷŸ��̿顣
				���������̿�ŷǷ������߶�Ӧ���û����������̿飬��ô�����κβ�����
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
			���������̿�
			������
				search_6 - false:����1�������̿飬true:����6�����������̿飬���ڴ��Ŀ¼�ļ�
			���أ�
				��С��2048������ - һ�������̿�ı��
				0xFFFFFFFF       - �Ҳ��������̿�
			���á�ѭ���״���Ӧ�㷨��,ԭ��
				1. ����ļ���СΪ16M�����洢����Ϊ1G��Ҳ����˵û��ʲô��ν�ġ����ļ���
				2. ����������£�ѭ���������٣�����Ч�����
		*/
		unsigned int SearchEmptyBlock(bool search_6)
		{
			if (search_6)
			{
				//ʵ���ϣ�����ж��������㣬��ô�Է���ֵ���8���̿鶼�ǿ��е�
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
						unsigned int r[8];					//������ȡ�ֽ��е��ض�λ
						r[0] = content[i] & 0x01;			//���λ
						r[1] = (content[i] & 0x02) >> 1;
						r[2] = (content[i] & 0x04) >> 2;
						r[3] = (content[i] & 0x08) >> 3;
						r[4] = (content[i] & 0x10) >> 4;
						r[5] = (content[i] & 0x20) >> 5;
						r[6] = (content[i] & 0x40) >> 6;
						r[7] = (content[i] & 0x80) >> 7;	//���λ

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
						unsigned int r[8];					//������ȡ�ֽ��е��ض�λ
						r[0] = content[i] & 0x01;			//���λ
						r[1] = (content[i] & 0x02) >> 1;
						r[2] = (content[i] & 0x04) >> 2;
						r[3] = (content[i] & 0x08) >> 3;
						r[4] = (content[i] & 0x10) >> 4;
						r[5] = (content[i] & 0x20) >> 5;
						r[6] = (content[i] & 0x40) >> 6;
						r[7] = (content[i] & 0x80) >> 7;	//���λ

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
