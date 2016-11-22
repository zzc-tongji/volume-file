#pragma once
#pragma warning(disable:4996)

#include "Image_OS.h"
#include "Image_InodeTable.h"
#include "DireFile.h"
#include "DataFile.h"

/*
	�����̡����ڴ�ӳ��1GiB

	Part 1: BIOS�����ϵͳ��100KiB
	Part 2: �ļ�ϵͳ�����飺0.5KiB
	Part 3: Inode�ڵ��667.5KiB
	Part 4: �̿�λʾͼ��256KiB
	Part 5: �û����ݣ�1023MiB
*/
class Image
{
	public:

		//Part 1: BIOS�����ϵͳ
		Image_OS operating_system;
		//Part 2: �ļ�ϵͳ������
		Image_SuperBlock super_block;
		//Part 3: Inode�ڵ��
		Image_InodeTable inode_table;
		//Part 4: �̿�λʾͼ
		Image_BitMap bit_map;
		//Part 5: �û�����
		Image_UserData user_data;

		//����·��������\VolumeFile
		char work_path[600];
		//�����̡�·��������\VolumeFile\Disk.dat
		char disk_path[600];
		//�������ļ���д��������·��������\VolumeFile\editor.dat
		char editor_path[600];
		//�����̡���дͷ
		fstream fs_disk;
		//ָ����±�־
		bool UpdatePointer;

		//���캯��
		Image()
		{
			/* �ļ���д׼�� */

			//����·��
			CreatePath();
			//�򿪡����̡���Ϊ�����Ķ�д��׼��
			fs_disk.open(disk_path, ios::in | ios::out | ios::binary | ios::_Nocreate);
			if (!fs_disk)
			{
				CreateDisk();
				fs_disk.open(disk_path, ios::in | ios::out | ios::binary | ios::_Nocreate);
				if (!fs_disk)
				{
					cout << "�޷��򿪴����ļ��������˳���" << endl;
					system("pause");
					exit(-1);
				}
			}
			//��ָ�븴λ
			fs_disk.seekg(0);
			//дָ�븴λ
			fs_disk.seekp(0);
			//����ָ����±�־
			UpdatePointer = false;

			/* ���ԣ��Ƿ���Ҫ��ʽ������ */

			Inode test1;
			DireEntry *test2;
			char *test3;
			bool format;
			//�ƶ���201#�̿���ʼ��
			fs_disk.seekg((Image_SuperBlock::IT_BLOCK_START) * Image_SuperBlock::BLOCK_CAP);
			//��ȡInode[0]
			fs_disk.read((char*)&test1, Inode::INODE_SIZE);
			//������1����Ŀ¼�Ƿ���ڣ�Inode[0]��mode��Ա��[0][1]λ�Ƿ��Ϊ1��
			if ((test1.GetMode(0) == true) && (test1.GetMode(1) == true))
			{
				format = true;
			}
			else
			{
				format = false;
			}
			if (format)
			{
				//�ƶ���2048#�̿���ʼ��
				fs_disk.seekg(Image_SuperBlock::USER_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
				//������Կռ�
				test2 = new DireEntry[2];
				//��ȡǰ����Ŀ¼��
				fs_disk.read((char *)test2, 2 * sizeof(DireEntry));//������3����Ŀ¼�Ƿ���ȷ
				format = false;
				if (test2[0].m_ino == 0)
				{
					if (strcmp(test2[0].m_name, ".") == 0)
					{
						if (test2[1].m_ino == 0)
						{
							if (strcmp(test2[1].m_name, "..") == 0)
							{
								format = true;
							}
						}
					}
				}
				//�ͷŲ��Կռ�
				delete[]test2;
				if (format)
				{
					//�ƶ���1536#�̿���ʼ��
					fs_disk.seekg(Image_SuperBlock::BM_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
					//������Կռ�
					test3 = new char[Image_BitMap::SYSTEM_BLOCK_DESC];
					//��ȡImage_BitMap��ǰ256���ֽ�
					fs_disk.read(test3, Image_BitMap::SYSTEM_BLOCK_DESC);
					//������3��ϵͳ�̿��Ƿ��Ѿ���ռ�ã��ж�Image_BitMap��ǰ256���ֽ��Ƿ��Ϊ��\0xFF����
					for (int i = 0; format && i <= Image_BitMap::SYSTEM_BLOCK_DESC - 1; i++)
					{
						if (test3[i] != '\xFF')
						{
							format = false;
						}
					}
					//�ͷŲ��Կռ�
					delete[]test3;
				}
			}

			/*
				����

				1. �ڴ���Ĵ���ӳ���Ǹ�ʽ�������û���κ����ݵġ�
				2. ��������Ѿ���ʽ������ôִ�ж�������������ڴ��д������µĴ���ӳ��
				3. �������û�и�ʽ������ôִ��д����������ʵ�ִ��̸�ʽ����
			*/

			if (format)
			{
				SyncIn();
			}
			else
			{
				Format();
			}

			/* ����ָ�� */

			if (UpdatePointer == false)
			{
				//����super_block�����ָ��
				super_block.inode_table = &inode_table;
				super_block.bit_map = &bit_map;
				super_block.user_data = &user_data;
				super_block.SetWorkPath(work_path, disk_path, editor_path);
				//����ָ��super_block�����ָ��
				inode_table.SetSuperBlockAll(&super_block);
				bit_map.SetSuperBlock(&super_block);
				user_data.SetSuperBlock(&super_block);
				//����ָ����±�־
				UpdatePointer = true;
			}
		}

		//��������
		~Image()
		{
			fs_disk.close();
		}

		//��������·��
		void CreatePath()
		{
			//��ȡϵͳ����·��
			TCHAR path[260];
			if (SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, false) == false)
			{
				cout << "�޷���ȡ����·���������˳�" << endl;
				system("pause");
				exit(-1);
			}
			wcstombs(work_path, path, 600);
			//�������ϴ�����VolumeFile���ļ���
			strcat(work_path, "\\VolumeFile");
			if (_access(work_path, 0) == -1)
			{
				if (_mkdir(work_path) == -1)
				{
					cout << "�޷�������VolumeFile���ļ��У������˳���" << endl;
					system("pause");
					exit(-1);
				}
			}
			//��ȡ��disk.dat��·����δ�����ļ���
			strcpy(disk_path, work_path);
			strcat(disk_path, "\\disk.dat");
			//��ȡ��editor.dat��·����δ�����ļ���
			strcpy(editor_path, work_path);
			strcat(editor_path, "\\editor.dat");
		}

		//���������̡�
		void CreateDisk()
		{
			ofstream fout;
			char one_block[Image_SuperBlock::BLOCK_CAP];
			for (int i = 0; i <= Image_SuperBlock::BLOCK_CAP - 1; i++)
			{
				one_block[i] = '\0';
			}
			fout.open(disk_path, ios::out | ios::binary);
			if (!fout)
			{
				cout << "���������̡�ʧ�ܣ������˳���" << endl;
				system("pause");
				exit(-1);
			}
			for (int i = 0; i <= Image_SuperBlock::ALL_BLOCK_NUM - 1; i++)
			{
				fout.write(one_block, Image_SuperBlock::BLOCK_CAP);
			}
			fout.close();
		}

		//��ʽ������
		void Format()
		{
			/* ��ʾ���ʱ */

			cout << "��ʼ��ʽ�������ļ���" << endl;
			cout << "���Ժ�..." << endl;
			LARGE_INTEGER tick, fc_begin, fc_end;
			QueryPerformanceFrequency(&tick);		//���ʱ��Ƶ��
			QueryPerformanceCounter(&fc_begin);		//��ó�ʼӲ����ʱ������

			/* ����ָ�� */

			if (UpdatePointer == false)
			{
				//����super_block�����ָ��
				super_block.inode_table = &inode_table;
				super_block.bit_map = &bit_map;
				super_block.user_data = &user_data;
				super_block.SetWorkPath(work_path, disk_path, editor_path);
				//����ָ��super_block�����ָ��
				inode_table.SetSuperBlockAll(&super_block);
				bit_map.SetSuperBlock(&super_block);
				user_data.SetSuperBlock(&super_block);
				//����ָ����±�־
				UpdatePointer = true;
			}

			/* �����ļ�ϵͳ�ĸ�Ŀ¼ */

			CreateRootDirectory();

			/* ϵͳ��BIOS�����ϵͳ���ļ�ϵͳ�����顢InodeTable��λʾͼ���̿��ʼ�� */

			//дָ�븴λ
			fs_disk.seekp(0);
			//0#~199#��100KiB����BIOS�����ϵͳ
			fs_disk.write(operating_system.content, Image_SuperBlock::OS_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP);
			//200#��0.5KiB�����ļ�ϵͳ������
			fs_disk.write((char *)&(super_block), Image_SuperBlock::SB_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP);
			//201#~1535#��667.5KiB����Inode�ڵ��
			for (int i = 0; i <= Image_InodeTable::MAX_INODE_NUM - 1; i++)
			{
				fs_disk.write((char*)(&(inode_table.content[i])), Inode::INODE_SIZE);
			}
			//1536#~2047#��256KiB�����̿�λʾͼ
			fs_disk.write(bit_map.content, Image_BitMap::ALL_BLOCK_DESC);

			/* �û��̿��ʼ�� */

			//2048#~2097151#��1023MiB�����û�����
			//ֻҪд��0#~5#�û��̿飨2048#~2053#�̿飩�������Ÿ�Ŀ¼������
			fs_disk.write(user_data.content, DireFile::FIXED_SIZE_BLOCK * Image_SuperBlock::BLOCK_CAP);

			/* ��ʾ���ʱ */

			QueryPerformanceCounter(&fc_end);		//�����ֹӲ����ʱ������
			cout << setiosflags(ios::fixed) << setprecision(3);
			cout << "��ʽ��������ʱ��" << double(fc_end.QuadPart - fc_begin.QuadPart) / tick.QuadPart << "��" << endl;
			cout << "��ʽ��������ɣ�" << endl;
			cout << endl;
		}

		//�����ļ�ϵͳ�ĸ�Ŀ¼
		void CreateRootDirectory()
		{
			unsigned int temp_v = 0;
			unsigned int *temp_p = nullptr;
			DireFile dir_file;

			//ռ��Inode�ڵ���0��Inode�ڵ�
			inode_table.content[0].Occupy();
			//�趨ΪĿ¼�ļ�
			inode_table.content[0].SetModeDirectory(true);
			//���䡰�����̿顱
			inode_table.content[0].RedistributeBlock(DireFile::FIXED_SIZE_BLOCK);
			//ȡ��0#�߼��̿��Ӧ�ġ������̿顱
			temp_v = inode_table.content[0].Bmap(0, temp_p);
			//Ŀ¼�ļ���д׼��
			dir_file.Prepare(&super_block, 0);
			//��ʼ��������������m_ino����Ϊ0xFFFFFFFF������������������
			for (unsigned int i = 0; i <= DireFile::DIRECTORY_ENTRY_NUM - 1; i++)
			{
				dir_file.content[i].m_ino = 0xFFFFFFFF;
			}
			//�½�Ŀ¼���.��
			dir_file.content[0].m_ino = 0;
			strcpy(dir_file.content[0].m_name, ".");
			//�½�Ŀ¼���..��
			dir_file.content[1].m_ino = 0;
			strcpy(dir_file.content[1].m_name, "..");
		}

		//�����̶��뵽�ڴ�ӳ��
		void SyncIn()
		{
			/* ��ʾ���ʱ */

			cout << "��ʼ�������ļ����뵽�ڴ�ӳ��" << endl;
			cout << "���Ժ�..." << endl;
			LARGE_INTEGER tick, fc_begin, fc_end;
			QueryPerformanceFrequency(&tick);		//���ʱ��Ƶ��
			QueryPerformanceCounter(&fc_begin);		//��ó�ʼӲ����ʱ������

			/* ����ϵͳ��BIOS�����ϵͳ���ļ�ϵͳ�����顢InodeTable��λʾͼ���̿� */

			//��ָ�븴λ
			fs_disk.seekg(0);
			//0#~199#��100KiB����BIOS�����ϵͳ��ֱ��������
			fs_disk.seekg(Image_SuperBlock::SB_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
			//200#��0.5KiB�����ļ�ϵͳ�����飨ֱ��������
			fs_disk.seekg(Image_SuperBlock::IT_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
			//201#~1535#��667.5KiB����Inode�ڵ��
			for (int i = 0; i <= Image_InodeTable::MAX_INODE_NUM - 1; i++)
			{
				fs_disk.read((char*)&(inode_table.content[i]), Inode::INODE_SIZE);
			}
			//1536#~2047#��256KiB�����̿�λʾͼ
			fs_disk.read(bit_map.content, Image_BitMap::ALL_BLOCK_DESC);

			/* �����û��̿� */

			//2048#~2097151#��1023MiB�����û�����
			fs_disk.read(user_data.content, Image_SuperBlock::USER_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP);

			/* ��ʾ���ʱ */

			QueryPerformanceCounter(&fc_end);		//�����ֹӲ����ʱ������
			cout << setiosflags(ios::fixed) << setprecision(3);
			cout << "���������ʱ��" << double(fc_end.QuadPart - fc_begin.QuadPart) / tick.QuadPart << "��" << endl;
			cout << "���������ɣ�" << endl;
			cout << endl;
		}

		//���ڴ�ӳ��д��������
		void SyncOut()
		{
			/* ��ʾ���ʱ */

			cout << "��ʼ���ڴ�ӳ��д���������ļ���" << endl;
			cout << "���Ժ�..." << endl;
			LARGE_INTEGER tick, fc_begin, fc_end;
			QueryPerformanceFrequency(&tick);		//���ʱ��Ƶ��
			QueryPerformanceCounter(&fc_begin);		//��ó�ʼӲ����ʱ������

			/* д��ϵͳ��BIOS�����ϵͳ���ļ�ϵͳ�����顢InodeTable��λʾͼ���̿� */

			//дָ�븴λ
			fs_disk.seekp(0);
			//0#~199#��100KiB����BIOS�����ϵͳ��ֱ��������
			fs_disk.seekp(Image_SuperBlock::SB_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
			//200#��0.5KiB�����ļ�ϵͳ�����飨ֱ��������
			fs_disk.seekp(Image_SuperBlock::IT_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
			//201#~1535#��667.5KiB����Inode�ڵ��
			for (int i = 0; i <= Image_InodeTable::MAX_INODE_NUM - 1; i++)
			{
				fs_disk.write((char*)&(inode_table.content[i]), Inode::INODE_SIZE);
			}
			//1536#~2047#��256KiB�����̿�λʾͼ
			fs_disk.write(bit_map.content, Image_BitMap::ALL_BLOCK_DESC);

			/* д���û��̿� */

			//2048#~2097151#��1023MiB�����û�����
			fs_disk.write(user_data.content, Image_SuperBlock::USER_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP);

			/* ��ʾ���ʱ */

			QueryPerformanceCounter(&fc_end);		//�����ֹӲ����ʱ������
			cout << setiosflags(ios::fixed) << setprecision(3);
			cout << "д��������ʱ��" << double(fc_end.QuadPart - fc_begin.QuadPart) / tick.QuadPart << "��" << endl;
			cout << "д��������ɣ�" << endl;
			cout << endl;
		}
};
