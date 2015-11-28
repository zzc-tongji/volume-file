#pragma once

#include "Image.h"

/*
	路径名称分解项
*/
class DecomPathItem
{
	public:

		//前一斜杠的位置
		unsigned int last_sprit = -1;
		//分解路径
		string sub_path = "";
		//后一斜杠的位置
		unsigned int next_sprit = -1;

		//构造函数
		DecomPathItem()
		{
			Init();
		}

		//析构函数
		~DecomPathItem()
		{

		}

		//初始化
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
	文件系统操作界面
*/
class FileSystem_UI
{
	public:

		//路径名称分解项表的最大项数（输入路径的最大层数）：16
		static const unsigned int MAX_DIR_DEEPTH = 16;

		//进行操作的磁盘
		Image *image;

		//目录文件读写器
		DireFile dire_file;
		//根目录文件的Inode在Inode节点表中的编号
		unsigned int root_dire_ino_no;
		//根目录文件的Inode的地址
		Inode * root_dire_ino_pt;
		//当前目录文件的Inode在Inode节点表中的编号
		unsigned int curr_dire_ino_no;
		//当前目录文件的Inode的地址
		Inode *curr_dire_ino_pt;
		//当前目录文件的Inode在Inode节点表中的编号（备份）
		unsigned int curr_dire_ino_no_back_up;
		//当前数据文件的Inode的地址（备份）
		Inode *curr_dire_ino_pt_back_up;

		//数据文件读写器
		DataFile data_file;
		//当前数据文件的Inode在Inode节点表中的编号
		unsigned int curr_data_ino_no;
		//当前数据文件的Inode的地址
		Inode *curr_data_ino_pt;

		//操作命令
		string command;
		/*
			任何时候，下列变量的取值只有两种情况：
			1. 均为false
			2. 只有一个为true
		*/
		//待执行操作：转到目录，然后显示目录内容
		bool cd;
		//待执行操作：新建数据文件或目录文件
		bool create;
		//待执行操作：删除数据文件或者空目录文件（仅含“.”“..”两项的目录文件）
		bool remove;
		//待执行操作：打开数据文件
		bool open;
		//待执行操作：关闭系统
		bool shutdown;
		//待执行操作：自动写回设定
		bool sync;

		/*
			路径：
			1. 含有目标文件；
			2. 如果目标文件是目录文件，那么会在文件名后加/结尾
		*/
		string input_path;
		//路径的长度
		unsigned int input_path_lenth;

		/*
			路径分解项表
			1. 分解后单级路径的长度不超过27个英文字符；
			2. 层数不超过16
		*/
		DecomPathItem decom_path[MAX_DIR_DEEPTH];
		//分解后的路径名层数（取值范围：1-16）
		unsigned int decom_num;
		//输入是否为根目录
		bool root;
		//输入是否为相对路径
		bool relative;
		//输入是否为目录
		bool directory;

		//自动写回临界时间
		time_t sync_limit;
		//上次写回的时刻
		time_t time_last;
		//当前时刻
		time_t time_now;

		//构造函数
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

		//析构函数
		~FileSystem_UI()
		{

		}

		//启动文件系统
		void Run()
		{
			cout << "---------------------" << endl;
			cout << " 文件系统已启动！ " << endl;
			cout << "---------------------" << endl;
			cout << endl;
			while (shutdown == false)
			{
				unsigned int status = 0;
				//初始化输入信息
				InitInput();
				//容错输入
				while (status != 1)
				{
					cout << "--------------------------" << endl;
					if (sync)
					{
						cout << " 自动写回时间间隔：" << sync_limit << "秒" << endl;
					}
					else
					{
						cout << " 自动写回：已停用" << endl;
					}
					cout << "--------------------------" << endl;
					cout << "当前目录";
					if (curr_dire_ino_no == root_dire_ino_no)
					{
						cout << "（根目录）";
					}
					cout << "下的文件：" << endl;
					//输出当前目录下的文件
					OutputDir();
					cout << endl;
					//输出提示
					cout << "提醒：输入路径不能超过16层，输入路径中的任何目录名称或目录名称不能超过27字节。" << endl;
					cout << endl;
					//输入命令
					cin >> command;
					if (command == "cd" || command == "create" || command == "remove" || command == "open")
					{
						//输入路径
						cin >> input_path;
					}
					//对输入的内容进行分析
					status = AnalysisInput();
					if (status == 0xFFFFFFFF)
					{
						cout << "VolumeFile: 命令不合法！" << endl;
						cout << endl;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "VolumeFile: 路径名为空！" << endl;
						cout << endl;
					}
					else if (status == 0xFFFFFFFD)
					{
						cout << "VolumeFile: 路径名中存在连续出现的'/'字符！" << endl;
						cout << endl;
					}
					else if (status == 0xFFFFFFFC)
					{
						cout << "VolumeFile: 路径的层数过多！" << endl;
						cout << endl;
					}
					else if (status == 0xFFFFFFFB)
					{
						cout << "VolumeFile: 路径中存在过长的文件名！" << endl;
						cout << endl;
					}
				}
				//转到目录，然后显示目录内容
				if (cd)
				{
					status = SetCurrDireFile(false);
					if (status == 0xFFFFFFFF)
					{
						cout << "cd: 参数不是目录，无法操作！" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "cd: 找不到指定目录！" << endl;
						cout << endl;
						continue;
					}
					cout << "cd: 成功变更到指定目录！" << endl;
					cout << endl;
				}
				//新建数据文件或目录文件
				else if (create)
				{
					status = CreatingFile();
					if (status == 0xFFFFFFFA)
					{
						cout << "create: 找不到新建位置！" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFB)
					{
						cout << "create: 存在同名的文件或目录！" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFD)
					{
						cout << "create: “磁盘”已满，无法新建文件！" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "create: Inode节点表已满，无法新建文件！" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFF)
					{
						cout << "create:  待操作的目录已满，无法新建文件！" << endl;
						cout << endl;
						continue;
					}
					cout << "create: 成功新建文件！" << endl;
					cout << endl;
				}
				//删除数据文件或者空目录文件（仅含“.”“..”两项的目录文件）
				else if (remove)
				{
					status = RemovingFile();
					if (status == 0xFFFFFFFC)
					{
						cout << "remove: 找不到删除位置！" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFD)
					{
						cout << "remove: 需要删除的目录文件不为空！" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "remove: 找不到需要删除的文件！" << endl;
						cout << endl;
						continue;
					}
					cout << "remove: 成功删除文件！" << endl;
					cout << endl;
				}
				//打开数据文件
				else if (open)
				{
					//定位到数据文件
					status = SetCurrDataFile();
					if (status == 0xFFFFFFFF)
					{
						cout << "open: 参数不是文件！" << endl;
						cout << endl;
						continue;
					}
					else if (status == 0xFFFFFFFE)
					{
						cout << "open: 找不到文件！" << endl;
						cout << endl;
						continue;
					}
					//打开操作
					OpenFile();
				}
				time_now = time(NULL);
				/*
					进行自动写回需满足下列条件全部：
					  1. 自动写回已启用；
					  2. 在从上次自动写回到现在的这段时间里，“磁盘”状态发生了更改；
					  3. 执行自动写回后，文件系统不关闭（不会立即进行关闭写回）；
					  4. 从上次自动写回到现在的这段时间，超过了自动写回的临界时间
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
								cout << " 自动写回进行中... " << endl;
								cout << "-------------------" << endl;
								cout << endl;
								image->SyncOut();
								time_last = time(NULL);
								cout << "-------------------" << endl;
								cout << " 自动写回已完成！  " << endl;
								cout << "-------------------" << endl;
								cout << endl;
							}
						}
					}
				}
			}
			//退出前写回
			cout << "---------------------" << endl;
			cout << " 文件系统正在关闭... " << endl;
			cout << "---------------------" << endl;
			cout << endl;
			image->SyncOut();
		}

		//初始化输入信息
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
			输入分析
			返回：
				1          - 成功
				0xFFFFFFFF - 命令不合法，失败
				0xFFFFFFFE - 路径名称为空，失败
				0xFFFFFFFD - 路径名称中存在连续出现的'/'字符，失败
				0xFFFFFFFC - 路径层数超过16，失败
				0xFFFFFFFB - 路径中存在长度超过28的文件名称，失败
		*/
		unsigned int AnalysisInput()
		{
			/*
				命令分析
			*/
			if (command == "cd")			//转到目录，然后显示目录内容
			{
				cd = true;
			}
			else if (command == "create")	//新建数据文件或目录文件
			{
				create = true;
			}
			else if (command == "remove")	//删除数据文件或者空目录文件（仅含“.”“..”两项的目录文件）
			{
				remove = true;
			}
			else if (command == "open")		//打开数据文件
			{
				open = true;
			}
			else if (command == "sync")		//自动写回设定
			{
				AutoSyncOutSetting();
				cout << endl;
				return 1;
			}
			else if (command == "shutdown" || command == "exit")	//关闭系统
			{
				shutdown = true;
				cout << "VolumeFile: 关闭文件系统！" << endl;
				cout << endl;
				return 1;
			}
			else
			{
				InitInput();
				return 0xFFFFFFFF;
			}
			/*
				路径名称分析
			*/
			input_path_lenth = input_path.length();
			//是否为根目录
			if (input_path == "/")
			{
				root = true;
				return 1;
			}
			//是否为空
			if (input_path.empty())
			{
				return 0xFFFFFFFE;
			}
			//是否有连续出现的'/'字符
			if (input_path.find("//") != 0xFFFFFFFF)
			{
				return 0xFFFFFFFD;
			}
			//是否为相对路径
			if (input_path[0] != '/')
			{
				relative = true;
			}
			else
			{
				relative = false;
			}
			//是否为目录
			if (input_path[input_path_lenth - 1] == '/')
			{
				directory = true;
			}
			else
			{
				directory = false;
			}
			//分解输入路径
			unsigned int temp_1 = 0;
			unsigned int temp_2 = 0;
			if (relative)
			{
				//相对路径的开头没有'/'字符，需单独分析
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
				//如果temp_1 == string::npos，那么直接跳出
				if (temp_1 >= input_path_lenth - 1)
				{
					break;
				}
				//搜素分解字符串前的'/'字符
				temp_1 = input_path.find('/', temp_1);
				//如果temp_1 == string::npos，那么直接跳出
				if (temp_1 >= input_path_lenth - 1)
				{
					break;
				}
				//搜素分解字符串后的'/'字符
				temp_2 = input_path.find('/', temp_1 + 1);
				if (decom_num <= MAX_DIR_DEEPTH - 1)
				{
					//如果路径名称分解项表未满，那么记录分解信息
					decom_path[decom_num].set(temp_1, temp_2, input_path.substr(temp_1 + 1, temp_2 - temp_1 - 1));
				}
				else
				{
					//如果路径名称分解项表已满，那么路径层数一定超出了上限值
					return 0xFFFFFFFC;
				}
				temp_1 = temp_2;
				decom_num += 1;
			}
			//确保分解得到的所有文件名的长度不超过上限值
			for (unsigned int i = 0; i <= decom_num - 1; i++)
			{
				if (decom_path[i].sub_path.length() >= DireEntry::MAX_FILE_NAME_SIZE)
				{
					return 0xFFFFFFFB;
				}
			}
			return 1;
		}

		//自动写回设置
		void AutoSyncOutSetting()
		{
			string temp;
			cout << "启用或停用自动写回（Y|y-启用，N|n-停用）：" << endl;
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
				cout << "请设定自动写回的最小时间间隔（单位：秒）：" << endl;
				cin >> sync_limit;
				if (sync_limit > 1000)
				{
					cout << "设定值：" << endl;
					cout << "1000" << endl;
					sync_limit = 1000;
				}
				if (sync_limit < 5)
				{
					cout << "设定值：" << endl;
					cout << "5" << endl;
					sync_limit = 5;
				}
			}
			cout << "设定成功！" << endl;
		}

		//输出目录文件的内容
		void OutputDir()
		{
			Inode *inode_sub;
			string file_name;
			time_t time_value;
			tm *local_time;
			//目录文件读写准备
			dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
			//遍历所有目录项
			for (unsigned int i = 0; i <= DireFile::DIRECTORY_ENTRY_NUM - 1; i++)
			{
				//读取非空目录项
				if (dire_file.content[i].m_ino != 0xFFFFFFFF)
				{
					//子文件定位
					inode_sub = &(image->inode_table.content[dire_file.content[i].m_ino]);
					//取出子文件的文件名
					file_name = dire_file.content[i].m_name;
					// 1. 输出文件的访问权限：lrwxrwxrwx
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
					// 2. 输出文件的所有者
					cout << "User_" << inode_sub->uid << " ";
					cout << "Group_" << inode_sub->gid << " ";
					// 3. 输出文件的大小
					cout << setw(8) << setfill(' ') << setiosflags(ios::right) << inode_sub->size_byte << " ";
					// 4. 输出文件的占用空间
					cout << setw(8) << setfill(' ') << setiosflags(ios::right) << inode_sub->size_block * (Image_SuperBlock::BLOCK_CAP) << " ";
					// 5. 输出文件的最后访问时间
					time_value = time_t(inode_sub->atime);
					local_time = localtime(&time_value);
					local_time->tm_year += 1900;
					cout << setw(4) << setfill('0') << local_time->tm_year << "-";
					cout << setw(2) << setfill('0') << local_time->tm_mon << "-";
					cout << setw(2) << setfill('0') << local_time->tm_mday << " ";
					cout << setw(2) << setfill('0') << local_time->tm_hour << ":";
					cout << setw(2) << setfill('0') << local_time->tm_min << ":";
					cout << setw(2) << setfill('0') << local_time->tm_sec << " ";
					// 6. 输出文件名
					cout << file_name;
					//换行
					cout << endl;
				}
			}
		}

		/*
			新建文件
			返回：
				1          - 成功
				0xFFFFFFFF - 待操作的目录文件的容量已达上限，无法添加目录项，失败
				0xFFFFFFFE - Inode节点表已满，无法新建文件，失败
				0xFFFFFFFD - “磁盘”已满，无法新建文件，失败
				0xFFFFFFFB - 待操作的目录下存在同名的文件或目录，失败
				0xFFFFFFFA - 找不到待操作的目录，失败
		*/
		unsigned int CreatingFile()
		{
			unsigned int status;
			//备份当前目录信息
			SaveCurrDire();
			//定位需要操作的目录文件
			status = SetCurrDireFile(true);
			if (status == 1)
			{
				status = 0;
				//目录文件读写准备
				dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
				//新建空数据文件或空目录文件
				status = dire_file.CreateFileDir(decom_path[decom_num - 1].sub_path, directory);
				//恢复当前目录信息
				LoadCurrDire();
				return status;
			}
			else
			{
				//恢复当前目录信息
				LoadCurrDire();
				return 0xFFFFFFFA;
			}
		}

		/*
			删除文件
			返回：
				1          - 成功
				0xFFFFFFFE - 找不到待删除的文件，失败
				0xFFFFFFFD - 需要删除的目录文件不为空，失败
				0xFFFFFFFC - 找不到待操作的目录，失败
		*/
		unsigned int RemovingFile()
		{
			unsigned int status;
			//备份当前目录信息
			SaveCurrDire();
			//定位需要操作的目录文件
			status = SetCurrDireFile(true);
			if (status == 1)
			{
				//目录文件读写准备
				dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
				//新建空数据文件或空目录文件
				status = dire_file.RemoveFileDir(decom_path[decom_num - 1].sub_path, directory);
				//恢复当前目录信息
				LoadCurrDire();
				return status;
			}
			else
			{
				//恢复当前目录信息
				LoadCurrDire();
				return 0xFFFFFFFC;
			}
		}

		//打开数据文件
		void OpenFile()
		{
			string sub_proc;
			//数据文件读写准备
			data_file.Prepare(&(image->super_block), curr_data_ino_no);
			//“读出”到“数据文件读写器”
			data_file.Reading();
			//使用notepad打开大文件可能会崩溃，这时本程序也将崩溃
			//为了防止的数据损失，打开任何大于50K的文件之前需要强制写回
			if (curr_data_ino_pt->size_byte > 51200)
			{
				cout << endl;
				cout << "-------------------" << endl;
				cout << " 强制写回进行中... " << endl;
				cout << "-------------------" << endl;
				cout << endl;
				image->SyncOut();
				time_last = time(NULL);
				cout << "-------------------" << endl;
				cout << " 强制写回已完成！  " << endl;
				cout << "-------------------" << endl;
				cout << endl;
			}
			//显示
			cout << "open: 成功打开文件！请在notepad里查看和编辑文件。" << endl;
			cout << endl;
			cout << "VolumeFile：暂停！若要继续，请关闭notepad窗口。" << endl;
			cout << endl;
			//创建notepad子进程，用于编辑文件
			sub_proc = "notepad ";
			sub_proc += image->editor_path;
			system(sub_proc.c_str());
			//显示
			cout << "VolumeFile：继续！" << endl;
			cout << endl;;
			//“写入”到“磁盘”
			if (data_file.Writing() == 0xFFFFFFFF)
			{
				cout << "open: 文件过大，无法存储！" << endl;
				cout << endl;
			}
		}

		/*
			根据路径找出目录文件
			参数：
				previous   - true:搜索到上级目录，false:搜索到本级目录文件
			返回：
				1          - 已修改当前目录文件信息，成功
				0xFFFFFFFF - 路径对应数据文件，未修改当前目录文件信息，失败
				0xFFFFFFFE - 找不到文件，未修改当前目录文件信息，失败
		*/
		unsigned int SetCurrDireFile(bool previous)
		{
			//根目录：直接设定
			if (root)
			{
				curr_dire_ino_no = root_dire_ino_no;
				curr_dire_ino_pt = image->super_block.inode_table->ToAddress(curr_dire_ino_no);
				return 1;
			}
			if (previous)
			{
				//搜索到上级目录
				if (relative)
				{
					//相对路径
					dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
				}
				else
				{
					//绝对路径
					dire_file.Prepare(&(image->super_block), root_dire_ino_no);
				}
				unsigned int temp = curr_dire_ino_no;
				for (unsigned int i = 0; i <= decom_num - 1 - 1; i++)
				{
					//如果输入路径只有一层，直接跳出
					if (decom_num - 1 - 1 == 0xFFFFFFFF)
					{
						break;
					}
					//递进式搜索
					temp = dire_file.SearchFile(decom_path[i].sub_path, true);
					if (temp == 0xFFFFFFFE)
					{
						//找不到
						return 0xFFFFFFFE;
					}
					dire_file.Prepare(&(image->super_block), temp);
				}
				//设定当前目录文件信息
				curr_dire_ino_no = temp;
				curr_dire_ino_pt = image->super_block.inode_table->ToAddress(curr_dire_ino_no);
				return 1;
			}
			else
			{
				//搜索到本级目录
				if (directory == false)
				{
					return 0xFFFFFFFF;
				}
				else
				{
					if (relative)
					{
						//相对路径
						dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
					}
					else
					{
						//绝对路径
						dire_file.Prepare(&(image->super_block), root_dire_ino_no);
					}
					unsigned int temp = curr_dire_ino_no;
					for (unsigned int i = 0; i <= decom_num - 1; i++)
					{
						//如果输入路径为空，直接跳出
						if (decom_num - 1 == 0xFFFFFFFF)
						{
							break;
						}
						//递进式搜索
						temp = dire_file.SearchFile(decom_path[i].sub_path, true);
						if (temp == 0xFFFFFFFE)
						{
							//找不到
							return 0xFFFFFFFE;
						}
						dire_file.Prepare(&(image->super_block), temp);
					}
					//设定当前目录文件信息
					curr_dire_ino_no = temp;
					curr_dire_ino_pt = image->super_block.inode_table->ToAddress(curr_dire_ino_no);
					return 1;
				}
			}
		}

		/*
			根据路径找出数据文件
			返回：
				1          - 已修改当前数据文件信息，成功
				0xFFFFFFFF - 路径对应目录文件，未修改当前数据文件信息，失败
				0xFFFFFFFE - 找不到文件，未修改当前数据文件信息，失败
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
					//相对路径
					dire_file.Prepare(&(image->super_block), curr_dire_ino_no);
				}
				else
				{
					//绝对路径
					dire_file.Prepare(&(image->super_block), root_dire_ino_no);
				}
				unsigned int temp = curr_data_ino_no;
				for (unsigned int i = 0; i <= decom_num - 1; i++)
				{
					//如果输入路径为空，直接跳出
					if (decom_num - 1 == 0xFFFFFFFF)
					{
						break;
					}
					if (i == decom_num - 1)
					{
						//路径的末尾级，搜索数据文件
						temp = dire_file.SearchFile(decom_path[i].sub_path, false);
					}
					else
					{
						///路径的中间级，搜索目录文件
						temp = dire_file.SearchFile(decom_path[i].sub_path, true);
					}
					if (temp == 0xFFFFFFFE)
					{
						//找不到
						return 0xFFFFFFFE;
					}
					dire_file.Prepare(&(image->super_block), temp);
				}
				//设定当前数据文件信息
				curr_data_ino_no = temp;
				curr_data_ino_pt = image->super_block.inode_table->ToAddress(curr_data_ino_no);
				return 1;
			}
		}

		//备份当前目录文件信息
		void SaveCurrDire()
		{
			curr_dire_ino_no_back_up = curr_dire_ino_no;
			curr_dire_ino_pt_back_up = curr_dire_ino_pt;
		}

		//恢复当前目录文件信息
		void LoadCurrDire()
		{
			curr_dire_ino_no = curr_dire_ino_no_back_up;
			curr_dire_ino_pt = curr_dire_ino_pt_back_up;
		}
};
