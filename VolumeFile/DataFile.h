#pragma once

#include "Image_InodeTable.h"

/*
	�����ļ���д��
*/
class DataFile
{
	public:

		//ָ���ļ�ϵͳ�������ָ�루���ֵÿ�����ж�����£�
		Image_SuperBlock *super_block;

		//�������ļ���Inode��Inode�ڵ���е�λ��
		unsigned int inode_no;
		//�������ļ���Inode�ĵ�ַ
		Inode *inode_pt;
		//��ʱ����
		unsigned int *temp;
		//�������̿顱���
		unsigned int phy_blk;
		//��д��
		char *content;

		//�������ļ���д������дͷ
		fstream fs_editor;

		//���캯��
		DataFile()
		{
			super_block = nullptr;
			inode_no = 0;
			inode_pt = nullptr;
			temp = nullptr;
			phy_blk = 0xFFFFFFFF;
			content = nullptr;
		}

		//��������
		~DataFile()
		{

		}

		/*
			�����ļ���д׼��
			������
				s_b - ָ���ļ�ϵͳ�������ָ��
				ino - ��Ŀ¼�ļ���Inode�������е�λ��
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

		//�����������ļ���д����
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

		//�رա������ļ���д����
		void FsEditorClose()
		{
			fs_editor.close();
		}

		//�������̡��ϵ�һ�������ļ������������������ļ���д�������û�����ʹ��notepad���в鿴�ͱ༭��
		void Reading()
		{
			//��������ա������ļ���д����
			FsEditorOpen(true);
			for (unsigned int i = 0; i <= (inode_pt->size_block) - 1; i++)
			{
				//���ڱ༭ǰΪ�յ������ļ�����ִ�С�����������
				if ((inode_pt->size_block) - 1 == 0xFFFFFFFF)
				{
					break;
				}
				//��ȡ��������ļ��ġ������̿顱�ı��
				phy_blk = inode_pt->Bmap(i, temp);
				//��ȡ��Ӧ�Ķ�д��
				content = super_block->user_data->ToAddress(phy_blk);
				if (i == (inode_pt->size_block) - 1)
				{
					//������ڲ������Ǹ������ļ������һ���������̿顱,
					//��ô��Ҫ��������ļ�ʣ�ಿ�ֵĳ��ȣ�ȡ�ࣩ��Ȼ��ʣ�ಿ�֡���������
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
					//������ڲ����Ĳ��Ǹ������ļ������һ���������̿顱��
					//��ôֻ��Ҫ������������̿顱���塰��������
					fs_editor.write(content, Image_SuperBlock::BLOCK_CAP);
				}
			}
			//�رա������ļ���д����
			FsEditorClose();
		}

		/*
			���������ļ���д�����е����ݡ�д�롱�������̡��ϵ�һ�������ļ�
			���أ�
				1          - �ɹ�
				0xFFFFFFFF - ����д�롱�����ݹ����������ļ�����С��ʧ��
		*/
		unsigned int Writing()
		{
			if (false)
			{
				//��ϣУ����һ��
				return 1;
			}
			//�����������ļ���д����
			FsEditorOpen(false);
			//���·��������ļ�����ġ������̿顱
			if (fs_editor.tellg() > Inode::MAX_SIZE_BYTE)
			{
				return 0xFFFFFFFF;
			}
			inode_pt->RedistributeBlock((unsigned int)(fs_editor.tellg()));
			//��ָ�븴λ
			fs_editor.seekg(0);
			for (unsigned int i = 0; i <= (inode_pt->size_block) - 1; i++)
			{
				//���ڱ༭��Ϊ�յ������ļ�����ִ�С�д�롱����
				if ((inode_pt->size_block) - 1 == 0xFFFFFFFF)
				{
					break;
				}
				//��ȡ��������ļ��ġ������̿顱�ı��
				phy_blk = inode_pt->Bmap(i, temp);
				//��ȡ��Ӧ�Ķ�д��
				content = super_block->user_data->ToAddress(phy_blk);
				if (i == (inode_pt->size_block) - 1)
				{
					//������ڲ������Ǹ������ļ������һ���������̿顱,
					//��ô��Ҫ��������ļ�ʣ�ಿ�ֵĳ��ȣ�ȡ�ࣩ��Ȼ��ʣ�ಿ�֡�д�롱��
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
					//������ڲ����Ĳ��Ǹ������ļ������һ���������̿顱��
					//��ôֻ��Ҫ������������̿顱���塰д�롱��
					fs_editor.read(content, Image_SuperBlock::BLOCK_CAP);
				}
			}
			//���������ļ���������ʱ��
			inode_pt->UpdateAtime();
			//�رա������ļ���д��
			FsEditorClose();
			return 1;
		}
};