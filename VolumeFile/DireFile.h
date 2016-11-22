#pragma once
#pragma warning(disable:4996)

#include "Image_InodeTable.h"

/*
	Ŀ¼��
*/
class DireEntry
{
	public:

		//һ��Ŀ¼����ռ�ݵĿռ��С��32�����ȷ������
		static const unsigned int DIRECTORY_ENTRY_SIZE = 32;
		//Ŀ¼����·�����ֵ�����ַ������ȣ�28
		static const unsigned int MAX_FILE_NAME_SIZE = 28;

		//Ŀ¼����Inode��Ų���
		unsigned int m_ino;
		//Ŀ¼����·��������
		char m_name[MAX_FILE_NAME_SIZE];

		//���캯��
		DireEntry()
		{
			m_ino = -1;
			m_name[0] = '\0';
		}

		//��������
		~DireEntry()
		{

		}
};

/*
	Ŀ¼�ļ���д��
*/
class DireFile
{
	public:

		//һ��Ŀ¼�ļ��̶�ռ���̿�����6
		static const unsigned int FIXED_SIZE_BLOCK = 6;
		//һ��Ŀ¼�ļ���Ŀ¼������������96
		static const unsigned int DIRECTORY_ENTRY_NUM = Image_SuperBlock::BLOCK_CAP / DireEntry::DIRECTORY_ENTRY_SIZE * FIXED_SIZE_BLOCK;

		//ָ���ļ�ϵͳ�������ָ�루���ֵÿ�����ж�����£�
		Image_SuperBlock *super_block;

		//��Ŀ¼�ļ���Inode��Inode�������е�λ��
		unsigned int inode_no;
		//��Ŀ¼�ļ���Inode�ĵ�ַ
		Inode *inode_pt;
		//��Ŀ¼�ļ�ռ�õ��׸��������̿顱���
		unsigned int phy_blk;
		//��Ŀ¼�ļ��ı༭λ��
		DireEntry *content;

		//��ǰ���ڱ༭�����ļ���Ŀ¼����
		unsigned int directory_no_child;
		//��ǰ���ڱ༭�����ļ���Inode��Inode�������е�λ��
		unsigned int inode_no_child;
		//��ǰ���ڱ༭�����ļ���Inode�ĵ�ַ
		Inode *inode_pt_child;
		//��ǰ���ڱ༭�����ļ�ռ�õ��׸��������̿顱���
		unsigned int phy_blk_child;
		//������ļ���Ŀ¼�ļ�����ô���б�����ʾ���ļ��ı༭λ��
		DireEntry *content_child;

		//���캯��
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

		//��������
		~DireFile()
		{

		}

		/*
			Ŀ¼�ļ���д׼��
			������
				s_b - ָ���ļ�ϵͳ�������ָ��
				ino - ��Ŀ¼�ļ���Inode�������е�λ��
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
			�½��������ļ����Ŀ¼�ļ�
			������
				s   - ���½��ļ���Ŀ¼������
				dir - ���½����Ƿ�ΪĿ¼
			���أ�
				1          - �ɹ�
				0xFFFFFFFF - ��Ŀ¼�ļ��������Ѵ����ޣ��޷����Ŀ¼�ʧ��
				0xFFFFFFFE - Inode�ڵ���������޷��½��ļ���ʧ��
				0xFFFFFFFD - �����̡��������޷��½��ļ���ʧ��
				0xFFFFFFFB - ��Ŀ¼�´���ͬ�����ļ���Ŀ¼��ʧ��
		*/
		unsigned int CreateFileDir(string s, bool dir)
		{
			const char *c = s.c_str();
			//����Ƿ���ͬ�������ļ���Ŀ¼�ļ�
			for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//���ж����Ŀ¼��������
				if (content[i].m_ino != 0xFFFFFFFF)
				{
					//�����Ƿ���ͬ
					if (strcmp(content[i].m_name, c) == 0)
					{
						//���ͣ������ļ�/Ŀ¼�ļ����Ƿ���ͬ
						if ((super_block->inode_table->content[content[i].m_ino]).GetModeDirectory() == dir)
						{
							return 0xFFFFFFFB;
						}
					}
				}
			}
			//��ʼ�½��ļ�����
			for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//����ҵ��յ�Ŀ¼��
				if (content[i].m_ino == 0xFFFFFFFF)
				{
					//�趨���������Ŀ¼��ı��
					directory_no_child = i;
					//Ϊ��Ҫ�½����ļ�����һ���յ�Inode�ڵ�
					inode_no_child = super_block->inode_table->AllocateEmptyInode();
					if (inode_no_child == 0xFFFFFFFF)
					{
						return 0xFFFFFFFE;
					}
					inode_pt_child = super_block->inode_table->ToAddress(inode_no_child);
					inode_pt_child->Occupy();
					//���Ŀ¼�ļ��������ļ��Ĳ�ͬ����
					if (dir)
					{
						//�趨��־λ
						inode_pt_child->SetModeDirectory(true);
						//���䡰�����̿顱
						if (inode_pt_child->RedistributeBlock(FIXED_SIZE_BLOCK) == 0xFFFFFFFF)
						{
							return 0xFFFFFFFD;
						}
						//Ŀ¼�ļ���д׼�������½���Ŀ¼�ļ���
						phy_blk_child = inode_pt_child->addr[0];
						content_child = (DireEntry *)(super_block->user_data->ToAddress(phy_blk_child));
						//��ʼ��������������m_ino����Ϊ0xFFFFFFFF������������������
						for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
						{
							content_child[i].m_ino = 0xFFFFFFFF;
						}
						//�½�Ŀ¼���.��
						content_child[0].m_ino = inode_no_child;
						strcpy(content_child[0].m_name, ".");
						//�½�Ŀ¼���..��
						content_child[1].m_ino = inode_no;
						strcpy(content_child[1].m_name, "..");
					}
					else
					{
						//�趨��־λ
						inode_pt_child->SetModeDirectory(false);
						//���䡰�����̿顱
						if (inode_pt_child->RedistributeBlock(0) == 0xFFFFFFFF)
						{
							return 0xFFFFFFFD;
						}
					}
					//���Ѿ��½����ļ��Ǽǵ�Ŀ¼�����
					content[directory_no_child].m_ino = inode_no_child;
					strcpy(content[directory_no_child].m_name, c);
					//���·���ʱ��
					super_block->inode_table->content[inode_no].UpdateAtime();
					return 1;
				}
			}
			return 0xFFFFFFFF;
		}

		/*
			ɾ�������ļ����Ŀ¼�ļ�
			������
				s   - ��ɾ���ļ���Ŀ¼������
				dir - ��ɾ�����Ƿ�ΪĿ¼
			���أ�
				1          - �ɹ�
				0xFFFFFFFE - �Ҳ����ļ���ʧ��
				0xFFFFFFFD - ��Ҫɾ����Ŀ¼�ļ���Ϊ�գ�ʧ��
		*/
		unsigned int RemoveFileDir(string s, bool dir)
		{
			const char *c = s.c_str();
			//��ʼɾ���ļ�����
			for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//�ڷǿ�Ŀ¼��������
				if (content[i].m_ino != 0xFFFFFFFF)
				{
					//�趨������Ŀ¼��ı��
					directory_no_child = i;
					//�趨�������ļ���Inode�ڵ�
					inode_no_child = content[directory_no_child].m_ino;
					inode_pt_child = super_block->inode_table->ToAddress(inode_no_child);
					//�ļ������Ƿ����
					if (strcmp(c, content[directory_no_child].m_name) != 0)
					{
						continue;
					}
					//�ļ����ͣ������ļ�/Ŀ¼�ļ����Ƿ����
					if (inode_pt_child->GetModeDirectory() != dir)
					{
						continue;
					}
					//��ɾ������Ŀ¼�ļ�
					if (dir)
					{
						phy_blk_child = inode_pt_child->addr[0];
						content_child = (DireEntry *)(super_block->user_data->ToAddress(phy_blk_child));
						//�ж�Ŀ¼�ļ��Ƿ�Ϊ�գ��յ�Ŀ¼�ļ�ֻ�����С�.����..������Ŀ¼���Ŀ¼�ļ���
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
					//�ͷŸ��ļ�ռ�ݵ����С������̿顱�������ļ����ݺ�������
					inode_pt_child->RedistributeBlock(0);
					//�ͷŸ��ļ���Inode�ڵ�
					inode_pt_child->Release();
					//����Ŀ¼��
					content[directory_no_child].m_ino = 0xFFFFFFFF;
					//���·���ʱ��
					super_block->inode_table->content[inode_no].UpdateAtime();
					return 1;
				}
			}
			return 0xFFFFFFFE;
		}

		/*
			���Ҹ�Ŀ¼�µ��ļ�
			������
				s   - �������ļ���Ŀ¼������
				dir - �����ҵ��Ƿ�ΪĿ¼
			���أ�
				0 ~ 10680  - �ļ���Inode�ڵ���еı�ţ��ɹ�
				0xFFFFFFFE - �Ҳ����ļ���ʧ��
		*/
		unsigned int SearchFile(string s, bool dir)
		{
			const char *c = s.c_str();
			//��ʼ�����ļ�����
			for (unsigned int i = 0; i <= DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//�ڷǿ�Ŀ¼��������
				if (content[i].m_ino != 0xFFFFFFFF)
				{
					//�趨���ȶ�Ŀ¼��ı��
					directory_no_child = i;
					//�趨���ȶ��ļ���Inode�ڵ�
					inode_no_child = content[directory_no_child].m_ino;
					inode_pt_child = super_block->inode_table->ToAddress(inode_no_child);
					//�ļ������Ƿ����
					if (strcmp(c, content[directory_no_child].m_name) != 0)
					{
						continue;
					}
					//�ļ����ͣ������ļ�/Ŀ¼�ļ����Ƿ����
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
