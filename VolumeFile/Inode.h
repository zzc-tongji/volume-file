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

		//һ��Inode�ڵ���ռ�ݵĿռ��С��64�����ȷ������
		static const unsigned int INODE_SIZE = 64;

		//��ʼ��ʱ���ñ�־λ
		static const unsigned short MODE_INIT = 0x0000;
		//ռ��ʱ���ñ�־λ��110 110 110 1 1 000 0 1��
		static const unsigned short MODE_OCC = 0xDB61;

		//UID�趨ֵ
		static const unsigned short UID_INIT = 1;
		//GID�趨ֵ
		static const unsigned short GID_INIT = 1;

		//һ�������̿�����ɵ����������������128
		static const unsigned int MAX_INDEX_NUM = Image_SuperBlock::BLOCK_CAP / sizeof(unsigned int);
		//С���ļ�����С���飩��6
		static const unsigned int SMALL_MAX_SIZE_BLOCK = 6;
		//�����ļ�����С���飩��262
		static const unsigned int LARGE_MAX_SIZE_BLOCK = 6 + 2 * MAX_INDEX_NUM;
		//�����ͣ��ļ�����С���飩��33030
		static const unsigned int MAX_SIZE_BLOCK = 6 + 2 * MAX_INDEX_NUM + 2 * MAX_INDEX_NUM * MAX_INDEX_NUM;
		//�����ͣ��ļ�����С���ֽڣ���16911360 Bit��16515 KiB��Լ16.12 MiB��
		static const unsigned int MAX_SIZE_BYTE = MAX_SIZE_BLOCK * Image_SuperBlock::BLOCK_CAP;

		/*
			״̬��־λ��[15]~[0]
			[15][14][13]��	�ļ������߶��ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[12][11][10]��	�ļ������ߵ�ͬ�������û����ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[9][8][7]��		�����û����ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[6]��			uid��ʾ�ļ������ߵ��û���ʶ��
			[5]��			gid��ʾ�ļ����������ڵ��û����ʶ��
			[4][3][2]��		���ļ���000��С���ļ���001�������ļ���011�������ļ���111
			[1]��			��ͨ�ļ�-0��Ŀ¼�ļ�-1
			[0]��			�ڵ�δ����-0���ڵ��ѷ���-1

			ps�����ｫ˳�򵹹�����Ϊ���������Ч�ʣ���Ϊ�����SetMode()��GetMode()���Ǵӵ�λ�˿�ʼѡ��ġ�
		*/
		unsigned short mode;
		unsigned short uid;				//�ļ������ߵ��û���ʶ��
		unsigned short gid;				//�ļ������ߵ����ʶ��
		unsigned short padding;			//��ռλ��
		unsigned int size_byte;			//�ļ���С���ֽڣ�
		unsigned int size_block;		//�ļ���С���飬����һ����һ�飩
		unsigned int atime;				//������ʱ��
		unsigned int addr[10];			//�����ļ��߼���ź�������ת���Ļ���������
		Image_SuperBlock *super_block;	//ָ���ļ�ϵͳ�����飨���ֵÿ�����ж�����£�

		//���캯��
		Inode()
		{
			Release();
			for (unsigned int i = 0; i < 10; i++)
			{
				addr[i] = -1;
			}
			super_block = nullptr;
		}

		//��������
		~Inode()
		{

		}

		//�趨���Ƿ�Ϊ�ļ���
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

		//��ȡ���Ƿ�Ϊ�ļ���
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

		//����size_block
		void CalculateSizeBlock()
		{
			size_block = (unsigned int)(ceil(double(size_byte) / double(Image_SuperBlock::BLOCK_CAP)));
		}

		//����������ʱ��
		void UpdateAtime()
		{
			atime = (unsigned int)time(NULL);	//������90��֮������BUG��2014��8��11�գ�
		}

		/*
			ռ�ýڵ��˳��ռ�ýڵ� -> �������ļ�������Ŀռ� -> �������ļ����ݵĿռ�
			�����ڵ��˳���ͷŴ���ļ����ݵĿռ� -> �ͷŴ���ļ�������Ŀռ� -> �����ڵ�

			�мǣ���˳��һ�����ܴ���
		*/

		//ռ�ýڵ�
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

		//�ͷŽڵ�
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
			���·����ļ��̿�
			������
				s - ���·������ļ���С���ֽڣ���Χ��0~16911360��
			���أ�
				1          - �ɹ�
				0xFFFFFFFF - �����̡��������޷����䡰�����̿顱��ʧ��
			ע�⣺
				������Ǹ�Ŀ¼�ļ�����ôsΪ6��������Ч��
		*/
		unsigned int RedistributeBlock(unsigned int s)
		{
			//�ͷ��̿�
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
			//�����ļ���С
			if (GetModeDirectory())
			{
				size_byte = 3072;
			}
			else
			{
				size_byte = s;
			}
			CalculateSizeBlock();
			//�޸���Ӧ��־λ
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
			//�����̿�
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
			����addr[0]~addr[5]���ļ��������̿�
			���أ�
				1          - �ɹ�
				0xFFFFFFFF - �����̡��������޷����䡰�����̿顱��ʧ��
			ע�⣺
				����������״̬��־λ�Ĳ�����
				������Ǵ���ļ���������̿飬�����Ǵ���ļ����ݵ��̿飡
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
			����addr[6]~addr[7]���ļ��������̿�
			���أ�
				1          - �ɹ�
				0xFFFFFFFF - �����̡��������޷����䡰�����̿顱��ʧ��
			ע�⣺
				����������״̬��־λ�Ĳ�����
				������Ǵ���ļ���������̿飬�����Ǵ���ļ����ݵ��̿飡
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
				//addr[i]��һ��
				addr_sub = (unsigned int *)(super_block->user_data->ToAddress(addr[i]));
				for (unsigned int j = 0; j <= MAX_INDEX_NUM - 1; j++)
				{
					addr_sub[j] = 0xFFFFFFFF;
				}
			}
			return 1;
		}

		/*
			����addr[8]~addr[9]���ļ��������̿�
			���أ�
				1          - �ɹ�
				0xFFFFFFFF - �����̡��������޷����䡰�����̿顱��ʧ��
			ע�⣺
				����������״̬��־λ�Ĳ�����
				������Ǵ���ļ���������̿飬�����Ǵ���ļ����ݵ��̿飡
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
				//addr[i]��һ��
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
					//addr[8]������
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
			�������ļ����ݵ��̿�
			���أ�
				1           - �ɹ�
				0xFFFFFFFF  - �����̡��������޷����䡰�����̿顱��ʧ��
			ע�⣺
				����������״̬��־λ�Ĳ�����
				�����������������̿�Ĳ������ļ��������̿������ִ�б�����֮ǰ����ã������������ش���
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
			�ͷŴ���ļ����ݵ��̿�
			ע�⣺
				����������״̬��־λ�Ĳ�����
				��ִ�б�����֮�󣬲����ͷ��ļ��������̿飬������ɴ洢�ռ�й¶��
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
			�ͷ�addr[0]~addr[5]���ļ��������̿�
			ע�⣺
				����������״̬��־λ�Ĳ�����
				�ͷŵ��Ǵ���ļ���������̿飬�����Ǵ���ļ����ݵ��̿飡
		*/
		void ReleaseIndexBlock05()
		{
			for (unsigned int i = 0; i <= 5; i++)
			{
				addr[i] = -1;
			}
		}

		/*
			�ͷ�addr[6]~addr[7]���ļ��������̿�
			ע�⣺
				����������״̬��־λ�Ĳ�����
				�ͷŵ��Ǵ���ļ���������̿飬�����Ǵ���ļ����ݵ��̿飡
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
			�ͷ�addr[8]~addr[9]���ļ��������̿�
			ע�⣺
				����������״̬��־λ�Ĳ�����
				�ͷŵ��Ǵ���ļ���������̿飬�����Ǵ���ļ����ݵ��̿飡
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
			���߼��̿��ת��Ϊ�����̿��
			������
				lbn - �߼��̿��
				p - ��������̿�ŵ��޷��������ĵ�ַ�������
			���أ�
				2048 ~ 1097151 - �����̿�ţ�
			                     ��ʱ��pointerΪ��������̿�ŵ��޷��������ĵ�ַ��
				0xFFFFFFFF     - ����߼��̿����δ��Ӧ�����̿�ţ��������ļ������̿��ǻ������һ�������
			                     ��ʱ��pointerΪ��������̿�ŵ��޷��������ĵ�ַ��
			ǿ���˳���
				�������������Ӧ����û����ݵġ������̿顱����ô���������Ϣ����ǿ���˳���
		*/
		unsigned int Bmap(unsigned int lbn, unsigned int *(&pointer))
		{
			unsigned int r0, r1, r2;
			pointer = nullptr;
			//С���ļ�
			if (lbn < 6)
			{
				BmapFunc0(lbn, r0);
				pointer = addr + r0;
			}
			//�����ļ�
			else if (lbn < 6 + 2 * MAX_INDEX_NUM)
			{
				BmapFunc1(lbn, r0, r1);
				pointer = addr + r0;
				pointer = (unsigned int*)(super_block->user_data->ToAddress(*pointer));
				pointer += r1;
			}
			//�����ļ�
			else
			{
				BmapFunc2(lbn, r0, r1, r2);
				pointer = addr + r0;
				pointer = (unsigned int*)(super_block->user_data->ToAddress(*pointer));
				pointer += r1;
				pointer = (unsigned int*)(super_block->user_data->ToAddress(*pointer));
				pointer += r2;
			}
			//�������������Ӧ����û����ݵġ������̿顱����ô���������Ϣ����ǿ���˳���
			if (*pointer < Image_SuperBlock::USER_BLOCK_START || *pointer >= Image_SuperBlock::ALL_BLOCK_NUM)
			{
				if (*pointer != 0xFFFFFFFF)
				{
					cout << "����" << endl;
					cout << "Bmap�㷨����ȷ��" << endl;
					cout << endl;
					cout << "�����˳�..." << endl;
					system("pause");
					exit(-1);
				}
			}
			return *pointer;
		}

		//Bmap��ֱ��������������
		void BmapFunc0(unsigned int l, unsigned int &r0)
		{
			r0 = l;
		}

		//Bmap��һ��������������
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

		//Bmap������������������
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
			��mode���а�λ��ȡ
			������
				bit - ��Ҫ�����λ
			���أ�
				true  - 1
				false - 0
			TIPS:
				��������bit����15����ôһ�ɷ���false

			״̬��־λ��[15]~[0]
			[15][14][13]��	�ļ������߶��ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[12][11][10]��	�ļ������ߵ�ͬ�������û����ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[9][8][7]��		�����û����ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[6]��			uid��ʾ�ļ������ߵ��û���ʶ��
			[5]��			gid��ʾ�ļ����������ڵ��û����ʶ��
			[4][3][2]��		���ļ���000��С���ļ���001�������ļ���011�������ļ���111
			[1]��			��ͨ�ļ�-0��Ŀ¼�ļ�-1
			[0]��			�ڵ�δ����-0���ڵ��ѷ���-1

			ִ��Occupy()���ȡֵ���Ϊ������ҵͣ���110 110 110 1 1 000 0 1
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
			��mode���а�λд��
			������
				bit - ��Ҫ������λ
				set - true:��1��false:��0
			TIPS��
				��������bit����15����ô�趨��Ч

			״̬��־λ��[15]~[0]
			[15][14][13]��	�ļ������߶��ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[12][11][10]��	�ļ������ߵ�ͬ�������û����ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[9][8][7]��		�����û����ļ��ķ���Ȩ��rwx����Ȩ��-1��
			[6]��			uid��ʾ�ļ������ߵ��û���ʶ��
			[5]��			gid��ʾ�ļ����������ڵ��û����ʶ��
			[4][3][2]��		���ļ���000��С���ļ���001�������ļ���011�������ļ���111
			[1]��			��ͨ�ļ�-0��Ŀ¼�ļ�-1
			[0]��			�ڵ�δ����-0���ڵ��ѷ���-1

			ִ��Occupy()���ȡֵ���Ϊ������ҵͣ���110 110 110 1 1 000 0 1
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
