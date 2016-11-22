#pragma once

#include "Image.h"

/*
	·�����Ʒֽ���
*/
class DecomPathItem
{
	public:

		//ǰһб�ܵ�λ��
		unsigned int last_sprit = -1;
		//�ֽ�·��
		string sub_path = "";
		//��һб�ܵ�λ��
		unsigned int next_sprit = -1;

		//���캯��
		DecomPathItem()
		{
			Init();
		}

		//��������
		~DecomPathItem()
		{

		}

		//��ʼ��
		void Init()
		{
			last_sprit = string::npos;
			sub_path = "";
			next_sprit = string::npos;
		}

		void set(unsigned int l, unsigned int n, string s)
		{
			last_sprit = l;
			sub_path = s;
			next_sprit = n;
		}
};

/*
	�ļ�ϵͳ��������
*/
class FileSystem_UI
{
	public:

		//·�����Ʒֽ������������������·��������������16
		static const unsigned int MAX_DIR_DEEPTH = 16;

		//���в����Ĵ���
		Image *image;

		//Ŀ¼�ļ���д��
		DireFile dire_file;
		//��Ŀ¼�ļ���Inode��Inode�ڵ���еı��
		unsigned int root_dire_ino_no;
		//��Ŀ¼�ļ���Inode�ĵ�ַ
		Inode * root_dire_ino_pt;
		//��ǰĿ¼�ļ���Inode��Inode�ڵ���еı��
		unsigned int curr_dire_ino_no;
		//��ǰĿ¼�ļ���Inode�ĵ�ַ
		Inode *curr_dire_ino_pt;
		//��ǰĿ¼�ļ���Inode��Inode�ڵ���еı�ţ����ݣ�
		unsigned int curr_dire_ino_no_back_up;
		//��ǰ�����ļ���Inode�ĵ�ַ�����ݣ�
		Inode *curr_dire_ino_pt_back_up;

		//�����ļ���д��
		DataFile data_file;
		//��ǰ�����ļ���Inode��Inode�ڵ���еı��
		unsigned int curr_data_ino_no;
		//��ǰ�����ļ���Inode�ĵ�ַ
		Inode *curr_data_ino_pt;

		//��������
		string command;
		/*
			�κ�ʱ�����б�����ȡֵֻ�����������
			1. ��Ϊfalse
			2. ֻ��һ��Ϊtrue
		*/
		//��ִ�в�����ת��Ŀ¼��Ȼ����ʾĿ¼����
		bool cd;
		//��ִ�в������½������ļ���Ŀ¼�ļ�
		bool create;
		//��ִ�в�����ɾ�������ļ����߿�Ŀ¼�ļ���������.����..�������Ŀ¼�ļ���
		bool remove;
		//��ִ�в������������ļ�
		bool open;
		//��ִ�в������ر�ϵͳ
		bool shutdown;
		//��ִ�в������Զ�д���趨
		bool sync;

		/*
			·����
			1. ����Ŀ���ļ���
			2. ���Ŀ���ļ���Ŀ¼�ļ�����ô�����ļ������/��β
		*/
		string input_path;
		//·���ĳ���
		unsigned int input_path_lenth;

		/*
			·���ֽ����
			1. �ֽ�󵥼�·���ĳ��Ȳ�����27��Ӣ���ַ���
			2. ����������16
		*/
		DecomPathItem decom_path[MAX_DIR_DEEPTH];
		//�ֽ���·����������ȡֵ��Χ��1-16��
		unsigned int decom_num;
		//�����Ƿ�Ϊ��Ŀ¼
		bool root;
		//�����Ƿ�Ϊ���·��
		bool relative;
		//�����Ƿ�ΪĿ¼
		bool directory;

		//�Զ�д���ٽ�ʱ��
		time_t sync_limit;
		//�ϴ�д�ص�ʱ��
		time_t time_last;
		//��ǰʱ��
		time_t time_now;

		//���캯��
		FileSystem_UI()
		{
			image = new Image();
			root_dire_ino_no = 0;
			root_dire_ino_pt = image->super_block.inode_table->ToAddress(root_dire_ino_no);
			curr_dire_ino_no = root_dire_ino_no;
			curr_dire_ino_pt = root_dire_ino_pt;
			curr_data_ino_no = -1;
			curr_data_ino_pt = nullptr;
			SaveCurrDire();
			InitInput();
			shutdown = false;
			sync = true;
			sync_limit = 60;
			time_last = time(NULL);
			time_now = time(NULL);
		}

		//��������
		~FileSystem_UI()
		{

		}

		//�����ļ�ϵͳ
		void Run()
		{
			cout << "---------------------" << endl;
			cout << " �ļ�ϵͳ�������� " << endl;
			cout << "---------------------" << endl;
			cout << endl;
			while (shutdown == false)
			{
				unsigned int status = 0;
				//��ʼ��������Ϣ
				InitInput();
				//�ݴ�����
				while (status != 1)
				{
					cout << "--------------------------" << endl;
					if (sync)
					{
						cout << " �Զ�д��ʱ������" << sync_limit << "��" << endl;
					}
					else
					{
						cout << " �Զ�д�أ���ͣ��" << endl;
					}
					cout << "--------------------------" << endl;
					cout << "��ǰĿ¼";
					if (curr_dire_ino_no == root_dire_ino_no)
					{
						cout << "����Ŀ¼��";
					}
					cout << "�µ��ļ���" << endl;
					//�����ǰĿ¼�µ��ļ�
					OutputDir();
					cout << endl;
					//�����ʾ
					cout << "���ѣ�����·�����ܳ���16�㣬����·���е��κ�Ŀ¼���ƻ�Ŀ¼���Ʋ��ܳ���27�ֽڡ�" << endl;
					cout << endl;
					//��������
					cin >> command;
					if (command == "cd" || command == "create" || command == "remove" || command == "open")
					{
						//����·��
						cin >> input_path;
					}
					//����������ݽ��з���
					status = AnalysisInput();
					if (status == 0xFFFFFFFF)
					{
						cout << "VolumeFile: ����Ϸ���" << endl;
						cout << endl;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "VolumeFile: ·����Ϊ�գ�" << endl;
						cout << endl;
					}
					else if (status == 0xFFFFFFFD)
					{
						cout << "VolumeFile: ·�����д����������ֵ�'/'�ַ���" << endl;
						cout << endl;
					}
					else if (status == 0xFFFFFFFC)
					{
						cout << "VolumeFile: ·���Ĳ������࣡" << endl;
						cout << endl;
					}
					else if (status == 0xFFFFFFFB)
					{
						cout << "VolumeFile: ·���д��ڹ������ļ�����" << endl;
						cout << endl;
					}
				}
				//ת��Ŀ¼��Ȼ����ʾĿ¼����
				if (cd)
				{
					status = SetCurrDireFile(false);
					if (status == 0xFFFFFFFF)
					{
						cout << "cd: ��������Ŀ¼���޷�������" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "cd: �Ҳ���ָ��Ŀ¼��" << endl;
						cout << endl;
						continue;
					}
					cout << "cd: �ɹ������ָ��Ŀ¼��" << endl;
					cout << endl;
				}
				//�½������ļ���Ŀ¼�ļ�
				else if (create)
				{
					status = CreatingFile();
					if (status == 0xFFFFFFFA)
					{
						cout << "create: �Ҳ����½�λ�ã�" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFB)
					{
						cout << "create: ����ͬ�����ļ���Ŀ¼��" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFD)
					{
						cout << "create: �����̡��������޷��½��ļ���" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "create: Inode�ڵ���������޷��½��ļ���" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFF)
					{
						cout << "create:  ��������Ŀ¼�������޷��½��ļ���" << endl;
						cout << endl;
						continue;
					}
					cout << "create: �ɹ��½��ļ���" << endl;
					cout << endl;
				}
				//ɾ�������ļ����߿�Ŀ¼�ļ���������.����..�������Ŀ¼�ļ���
				else if (remove)
				{
					status = RemovingFile();
					if (status == 0xFFFFFFFC)
					{
						cout << "remove: �Ҳ���ɾ��λ�ã�" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFD)
					{
						cout << "remove: ��Ҫɾ����Ŀ¼�ļ���Ϊ�գ�" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "remove: �Ҳ�����Ҫɾ�����ļ���" << endl;
						cout << endl;
						continue;
					}
					cout << "remove: �ɹ�ɾ���ļ���" << endl;
					cout << endl;
				}
				//�������ļ�
				else if (open)
				{
					//��λ�������ļ�
					status = SetCurrDataFile();
					if (status == 0xFFFFFFFF)
					{
						cout << "open: ���������ļ���" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "open: �Ҳ����ļ���" << endl;
						cout << endl;
						continue;
					}
					//�򿪲���
					OpenFile();
				}
				time_now = time(NULL);
				/*
					�����Զ�д����������������ȫ����
					  1. �Զ�д�������ã�
					  2. �ڴ��ϴ��Զ�д�ص����ڵ����ʱ��������̡�״̬�����˸��ģ�
					  3. ִ���Զ�д�غ��ļ�ϵͳ���رգ������������йر�д�أ���
					  4. ���ϴ��Զ�д�ص����ڵ����ʱ�䣬�������Զ�д�ص��ٽ�ʱ��
				*/
				if (sync)
				{
					if (create || remove || open)
					{
						if (shutdown == false)
						{
							if (time_now - time_last >= sync_limit)
							{
								cout << "-------------------" << endl;
								cout << " �Զ�д�ؽ�����... " << endl;
								cout << "-------------------" << endl;
								cout << endl;
								image->SyncOut();
								time_last = time(NULL);
								cout << "-------------------" << endl;
								cout << " �Զ�д������ɣ�  " << endl;
								cout << "-------------------" << endl;
								cout << endl;
							}
						}
					}
				}
			}
			//�˳�ǰд��
			cout << "---------------------" << endl;
			cout << " �ļ�ϵͳ���ڹر�... " << endl;
			cout << "---------------------" << endl;
			cout << endl;
			image->SyncOut();
		}

		//��ʼ��������Ϣ
		void InitInput()
		{
			command = "";
			cd = false;
			create = false;
			remove = false;
			open = false;
			input_path = "";
			for (unsigned int i = 0; i <= MAX_DIR_DEEPTH - 1; i++)
			{
				decom_path[i].Init();
			}
			decom_num = 0;
			root = false;
			relative = false;
			directory = false;
		}

		/*
			�������
			���أ�
				1          - �ɹ�
				0xFFFFFFFF - ����Ϸ���ʧ��
				0xFFFFFFFE - ·������Ϊ�գ�ʧ��
				0xFFFFFFFD - ·�������д����������ֵ�'/'�ַ���ʧ��
				0xFFFFFFFC - ·����������16��ʧ��
				0xFFFFFFFB - ·���д��ڳ��ȳ���28���ļ����ƣ�ʧ��
		*/
		unsigned int AnalysisInput()
		{
			/*
				�������
			*/
			if (command == "cd")			//ת��Ŀ¼��Ȼ����ʾĿ¼����
			{
				cd = true;
			}
			else if (command == "create")	//�½������ļ���Ŀ¼�ļ�
			{
				create = true;
			}
			else if (command == "remove")	//ɾ�������ļ����߿�Ŀ¼�ļ���������.����..�������Ŀ¼�ļ���
			{
				remove = true;
			}
			else if (command == "open")		//�������ļ�
			{
				open = true;
			}
			else if (command == "sync")		//�Զ�д���趨
			{
				AutoSyncOutSetting();
				cout << endl;
				return 1;
			}
			else if (command == "shutdown" || command == "exit")	//�ر�ϵͳ
			{
				shutdown = true;
				cout << "VolumeFile: �ر��ļ�ϵͳ��" << endl;
				cout << endl;
				return 1;
			}
			else
			{
				InitInput();
				return 0xFFFFFFFF;
			}
			/*
				·�����Ʒ���
			*/
			input_path_lenth = input_path.length();
			//�Ƿ�Ϊ��Ŀ¼
			if (input_path == "/")
			{
				root = true;
				return 1;
			}
			//�Ƿ�Ϊ��
			if (input_path.empty())
			{
				return 0xFFFFFFFE;
			}
			//�Ƿ����������ֵ�'/'�ַ�
			if (input_path.find("//") != 0xFFFFFFFF)
			{
				return 0xFFFFFFFD;
			}
			//�Ƿ�Ϊ���·��
			if (input_path[0] != '/')
			{
				relative = true;
			}
			else
			{
				relative = false;
			}
			//�Ƿ�ΪĿ¼
			if (input_path[input_path_lenth - 1] == '/')
			{
				directory = true;
			}
			else
			{
				directory = false;
			}
			//�ֽ�����·��
			unsigned int temp_1 = 0;
			unsigned int temp_2 = 0;
			if (relative)
			{
				//���·���Ŀ�ͷû��'/'�ַ����赥������
				temp_2 = input_path.find('/');
				decom_path[0].set(string::npos, temp_2, input_path.substr(0, temp_2));
				decom_num = 1;
			}
			else
			{
				decom_num = 0;
			}
			while (true)
			{
				//���temp_1 == string::npos����ôֱ������
				if (temp_1 >= input_path_lenth - 1)
				{
					break;
				}
				//���طֽ��ַ���ǰ��'/'�ַ�
				temp_1 = input_path.find('/', temp_1);
				//���temp_1 == string::npos����ôֱ������
				if (temp_1 >= input_path_lenth - 1)
				{
					break;
				}
				//���طֽ��ַ������'/'�ַ�
				temp_2 = input_path.find('/', temp_1 + 1);
				if (decom_num <= MAX_DIR_DEEPTH - 1)
				{
					//���·�����Ʒֽ����δ������ô��¼�ֽ���Ϣ
					decom_path[decom_num].set(temp_1, temp_2, input_path.substr(temp_1 + 1, temp_2 - temp_1 - 1));
				}
				else
				{
					//���·�����Ʒֽ������������ô·������һ������������ֵ
					return 0xFFFFFFFC;
				}
				temp_1 = temp_2;
				decom_num += 1;
			}
			//ȷ���ֽ�õ��������ļ����ĳ��Ȳ���������ֵ
			for (unsigned int i = 0; i <= decom_num - 1; i++)
			{
				if (decom_path[i].sub_path.length() >= DireEntry::MAX_FILE_NAME_SIZE)
				{
					return 0xFFFFFFFB;
				}
			}
			return 1;
		}

		//�Զ�д������
		void AutoSyncOutSetting()
		{
			string temp;
			cout << "���û�ͣ���Զ�д�أ�Y|y-���ã�N|n-ͣ�ã���" << endl;
			cin >> temp;
			if (temp == "Y" || temp == "y")
			{
				sync = true;
			}
			else
			{
				sync = false;
			}
			if (sync)
			{
				cout << "���趨�Զ�д�ص���Сʱ��������λ���룩��" << endl;
				cin >> sync_limit;
				if (sync_limit > 1000)
				{
					cout << "�趨ֵ��" << endl;
					cout << "1000" << endl;
					sync_limit = 1000;
				}
				if (sync_limit < 5)
				{
					cout << "�趨ֵ��" << endl;
					cout << "5" << endl;
					sync_limit = 5;
				}
			}
			cout << "�趨�ɹ���" << endl;
		}

		//���Ŀ¼�ļ�������
		void OutputDir()
		{
			Inode *inode_sub;
			string file_name;
			time_t time_value;
			tm *local_time;
			//Ŀ¼�ļ���д׼��
			dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
			//��������Ŀ¼��
			for (unsigned int i = 0; i <= DireFile::DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//��ȡ�ǿ�Ŀ¼��
				if (dire_file.content[i].m_ino != 0xFFFFFFFF)
				{
					//���ļ���λ
					inode_sub = &(image->inode_table.content[dire_file.content[i].m_ino]);
					//ȡ�����ļ����ļ���
					file_name = dire_file.content[i].m_name;
					// 1. ����ļ��ķ���Ȩ�ޣ�lrwxrwxrwx
					if (inode_sub->GetModeDirectory())
					{
						cout << "l";
					}
					else
					{
						cout << "-";
					}
					for (unsigned k = 15; k >= 7; k--)
					{
						if (inode_sub->GetMode(k) == true)
						{
							if (k % 3 == 0)
							{
								cout << "r";
							}
							else if (k % 3 == 2)
							{
								cout << "w";
							}
							else
							{
								cout << "x";
							}
						}
						else
						{
							cout << "-";
						}
					}
					cout << " ";
					// 2. ����ļ���������
					cout << "User_" << inode_sub->uid << " ";
					cout << "Group_" << inode_sub->gid << " ";
					// 3. ����ļ��Ĵ�С
					cout << setw(8) << setfill(' ') << setiosflags(ios::right) << inode_sub->size_byte << " ";
					// 4. ����ļ���ռ�ÿռ�
					cout << setw(8) << setfill(' ') << setiosflags(ios::right) << inode_sub->size_block * (Image_SuperBlock::BLOCK_CAP) << " ";
					// 5. ����ļ���������ʱ��
					time_value = time_t(inode_sub->atime);
					local_time = localtime(&time_value);
					local_time->tm_year += 1900;
					cout << setw(4) << setfill('0') << local_time->tm_year << "-";
					cout << setw(2) << setfill('0') << local_time->tm_mon << "-";
					cout << setw(2) << setfill('0') << local_time->tm_mday << " ";
					cout << setw(2) << setfill('0') << local_time->tm_hour << ":";
					cout << setw(2) << setfill('0') << local_time->tm_min << ":";
					cout << setw(2) << setfill('0') << local_time->tm_sec << " ";
					// 6. ����ļ���
					cout << file_name;
					//����
					cout << endl;
				}
			}
		}

		/*
			�½��ļ�
			���أ�
				1          - �ɹ�
				0xFFFFFFFF - ��������Ŀ¼�ļ��������Ѵ����ޣ��޷����Ŀ¼�ʧ��
				0xFFFFFFFE - Inode�ڵ���������޷��½��ļ���ʧ��
				0xFFFFFFFD - �����̡��������޷��½��ļ���ʧ��
				0xFFFFFFFB - ��������Ŀ¼�´���ͬ�����ļ���Ŀ¼��ʧ��
				0xFFFFFFFA - �Ҳ�����������Ŀ¼��ʧ��
		*/
		unsigned int CreatingFile()
		{
			unsigned int status;
			//���ݵ�ǰĿ¼��Ϣ
			SaveCurrDire();
			//��λ��Ҫ������Ŀ¼�ļ�
			status = SetCurrDireFile(true);
			if (status == 1)
			{
				status = 0;
				//Ŀ¼�ļ���д׼��
				dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
				//�½��������ļ����Ŀ¼�ļ�
				status = dire_file.CreateFileDir(decom_path[decom_num - 1].sub_path, directory);
				//�ָ���ǰĿ¼��Ϣ
				LoadCurrDire();
				return status;
			}
			else
			{
				//�ָ���ǰĿ¼��Ϣ
				LoadCurrDire();
				return 0xFFFFFFFA;
			}
		}

		/*
			ɾ���ļ�
			���أ�
				1          - �ɹ�
				0xFFFFFFFE - �Ҳ�����ɾ�����ļ���ʧ��
				0xFFFFFFFD - ��Ҫɾ����Ŀ¼�ļ���Ϊ�գ�ʧ��
				0xFFFFFFFC - �Ҳ�����������Ŀ¼��ʧ��
		*/
		unsigned int RemovingFile()
		{
			unsigned int status;
			//���ݵ�ǰĿ¼��Ϣ
			SaveCurrDire();
			//��λ��Ҫ������Ŀ¼�ļ�
			status = SetCurrDireFile(true);
			if (status == 1)
			{
				//Ŀ¼�ļ���д׼��
				dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
				//�½��������ļ����Ŀ¼�ļ�
				status = dire_file.RemoveFileDir(decom_path[decom_num - 1].sub_path, directory);
				//�ָ���ǰĿ¼��Ϣ
				LoadCurrDire();
				return status;
			}
			else
			{
				//�ָ���ǰĿ¼��Ϣ
				LoadCurrDire();
				return 0xFFFFFFFC;
			}
		}

		//�������ļ�
		void OpenFile()
		{
			string sub_proc;
			//�����ļ���д׼��
			data_file.Prepare(&(image->super_block), curr_data_ino_no);
			//�����������������ļ���д����
			data_file.Reading();
			//ʹ��notepad�򿪴��ļ����ܻ��������ʱ������Ҳ������
			//Ϊ�˷�ֹ��������ʧ�����κδ���50K���ļ�֮ǰ��Ҫǿ��д��
			if (curr_data_ino_pt->size_byte > 51200)
			{
				cout << endl;
				cout << "-------------------" << endl;
				cout << " ǿ��д�ؽ�����... " << endl;
				cout << "-------------------" << endl;
				cout << endl;
				image->SyncOut();
				time_last = time(NULL);
				cout << "-------------------" << endl;
				cout << " ǿ��д������ɣ�  " << endl;
				cout << "-------------------" << endl;
				cout << endl;
			}
			//��ʾ
			cout << "open: �ɹ����ļ�������notepad��鿴�ͱ༭�ļ���" << endl;
			cout << endl;
			cout << "VolumeFile����ͣ����Ҫ��������ر�notepad���ڡ�" << endl;
			cout << endl;
			//����notepad�ӽ��̣����ڱ༭�ļ�
			sub_proc = "notepad ";
			sub_proc += image->editor_path;
			system(sub_proc.c_str());
			//��ʾ
			cout << "VolumeFile��������" << endl;
			cout << endl;;
			//��д�롱�������̡�
			if (data_file.Writing() == 0xFFFFFFFF)
			{
				cout << "open: �ļ������޷��洢��" << endl;
				cout << endl;
			}
		}

		/*
			����·���ҳ�Ŀ¼�ļ�
			������
				previous   - true:�������ϼ�Ŀ¼��false:����������Ŀ¼�ļ�
			���أ�
				1          - ���޸ĵ�ǰĿ¼�ļ���Ϣ���ɹ�
				0xFFFFFFFF - ·����Ӧ�����ļ���δ�޸ĵ�ǰĿ¼�ļ���Ϣ��ʧ��
				0xFFFFFFFE - �Ҳ����ļ���δ�޸ĵ�ǰĿ¼�ļ���Ϣ��ʧ��
		*/
		unsigned int SetCurrDireFile(bool previous)
		{
			//��Ŀ¼��ֱ���趨
			if (root)
			{
				curr_dire_ino_no = root_dire_ino_no;
				curr_dire_ino_pt = image->super_block.inode_table->ToAddress(curr_dire_ino_no);
				return 1;
			}
			if (previous)
			{
				//�������ϼ�Ŀ¼
				if (relative)
				{
					//���·��
					dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
				}
				else
				{
					//����·��
					dire_file.Prepare(&(image->super_block), root_dire_ino_no);
				}
				unsigned int temp = curr_dire_ino_no;
				for (unsigned int i = 0; i <= decom_num - 1 - 1; i++)
				{
					//�������·��ֻ��һ�㣬ֱ������
					if (decom_num - 1 - 1 == 0xFFFFFFFF)
					{
						break;
					}
					//�ݽ�ʽ����
					temp = dire_file.SearchFile(decom_path[i].sub_path, true);
					if (temp == 0xFFFFFFFE)
					{
						//�Ҳ���
						return 0xFFFFFFFE;
					}
					dire_file.Prepare(&(image->super_block), temp);
				}
				//�趨��ǰĿ¼�ļ���Ϣ
				curr_dire_ino_no = temp;
				curr_dire_ino_pt = image->super_block.inode_table->ToAddress(curr_dire_ino_no);
				return 1;
			}
			else
			{
				//����������Ŀ¼
				if (directory == false)
				{
					return 0xFFFFFFFF;
				}
				else
				{
					if (relative)
					{
						//���·��
						dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
					}
					else
					{
						//����·��
						dire_file.Prepare(&(image->super_block), root_dire_ino_no);
					}
					unsigned int temp = curr_dire_ino_no;
					for (unsigned int i = 0; i <= decom_num - 1; i++)
					{
						//�������·��Ϊ�գ�ֱ������
						if (decom_num - 1 == 0xFFFFFFFF)
						{
							break;
						}
						//�ݽ�ʽ����
						temp = dire_file.SearchFile(decom_path[i].sub_path, true);
						if (temp == 0xFFFFFFFE)
						{
							//�Ҳ���
							return 0xFFFFFFFE;
						}
						dire_file.Prepare(&(image->super_block), temp);
					}
					//�趨��ǰĿ¼�ļ���Ϣ
					curr_dire_ino_no = temp;
					curr_dire_ino_pt = image->super_block.inode_table->ToAddress(curr_dire_ino_no);
					return 1;
				}
			}
		}

		/*
			����·���ҳ������ļ�
			���أ�
				1          - ���޸ĵ�ǰ�����ļ���Ϣ���ɹ�
				0xFFFFFFFF - ·����ӦĿ¼�ļ���δ�޸ĵ�ǰ�����ļ���Ϣ��ʧ��
				0xFFFFFFFE - �Ҳ����ļ���δ�޸ĵ�ǰ�����ļ���Ϣ��ʧ��
		*/
		unsigned int SetCurrDataFile()
		{
			if (directory)
			{
				return 0xFFFFFFFF;
			}
			else
			{
				if (relative)
				{
					//���·��
					dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
				}
				else
				{
					//����·��
					dire_file.Prepare(&(image->super_block), root_dire_ino_no);
				}
				unsigned int temp = curr_data_ino_no;
				for (unsigned int i = 0; i <= decom_num - 1; i++)
				{
					//�������·��Ϊ�գ�ֱ������
					if (decom_num - 1 == 0xFFFFFFFF)
					{
						break;
					}
					if (i == decom_num - 1)
					{
						//·����ĩβ�������������ļ�
						temp = dire_file.SearchFile(decom_path[i].sub_path, false);
					}
					else
					{
						///·�����м伶������Ŀ¼�ļ�
						temp = dire_file.SearchFile(decom_path[i].sub_path, true);
					}
					if (temp == 0xFFFFFFFE)
					{
						//�Ҳ���
						return 0xFFFFFFFE;
					}
					dire_file.Prepare(&(image->super_block), temp);
				}
				//�趨��ǰ�����ļ���Ϣ
				curr_data_ino_no = temp;
				curr_data_ino_pt = image->super_block.inode_table->ToAddress(curr_data_ino_no);
				return 1;
			}
		}

		//���ݵ�ǰĿ¼�ļ���Ϣ
		void SaveCurrDire()
		{
			curr_dire_ino_no_back_up = curr_dire_ino_no;
			curr_dire_ino_pt_back_up = curr_dire_ino_pt;
		}

		//�ָ���ǰĿ¼�ļ���Ϣ
		void LoadCurrDire()
		{
			curr_dire_ino_no = curr_dire_ino_no_back_up;
			curr_dire_ino_pt = curr_dire_ino_pt_back_up;
		}
};
