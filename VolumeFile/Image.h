#pragma once
#pragma warning(disable:4996)

#include "Image_OS.h"
#include "Image_InodeTable.h"
#include "DireFile.h"
#include "DataFile.h"

/*
	“磁盘”的内存映像：1GiB

	Part 1: BIOS与操作系统：100KiB
	Part 2: 文件系统超级块：0.5KiB
	Part 3: Inode节点表：667.5KiB
	Part 4: 盘块位示图：256KiB
	Part 5: 用户数据：1023MiB
*/
class Image
{
	public:

		//Part 1: BIOS与操作系统
		Image_OS operating_system;
		//Part 2: 文件系统超级块
		Image_SuperBlock super_block;
		//Part 3: Inode节点表
		Image_InodeTable inode_table;
		//Part 4: 盘块位示图
		Image_BitMap bit_map;
		//Part 5: 用户数据
		Image_UserData user_data;

		//工作路径：桌面\VolumeFile
		char work_path[600];
		//“磁盘”路径：桌面\VolumeFile\Disk.dat
		char disk_path[600];
		//“数据文件读写缓冲区”路径：桌面\VolumeFile\editor.dat
		char editor_path[600];
		//“磁盘”读写头
		fstream fs_disk;
		//指针更新标志
		bool UpdatePointer;

		//构造函数
		Image()
		{
			/* 文件读写准备 */

			//创建路径
			CreatePath();
			//打开“磁盘”，为后续的读写做准备
			fs_disk.open(disk_path, ios::in | ios::out | ios::binary | ios::_Nocreate);
			if (!fs_disk)
			{
				CreateDisk();
				fs_disk.open(disk_path, ios::in | ios::out | ios::binary | ios::_Nocreate);
				if (!fs_disk)
				{
					cout << "无法打开磁盘文件！程序退出。" << endl;
					system("pause");
					exit(-1);
				}
			}
			//读指针复位
			fs_disk.seekg(0);
			//写指针复位
			fs_disk.seekp(0);
			//更改指针更新标志
			UpdatePointer = false;

			/* 测试：是否需要格式化操作 */

			Inode test1;
			DireEntry *test2;
			char *test3;
			bool format;
			//移动到201#盘块起始处
			fs_disk.seekg((Image_SuperBlock::IT_BLOCK_START) * Image_SuperBlock::BLOCK_CAP);
			//读取Inode[0]
			fs_disk.read((char*)&test1, Inode::INODE_SIZE);
			//【测试1】根目录是否存在（Inode[0]的mode成员的[0][1]位是否均为1）
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
				//移动到2048#盘块起始处
				fs_disk.seekg(Image_SuperBlock::USER_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
				//申请测试空间
				test2 = new DireEntry[2];
				//读取前两个目录项
				fs_disk.read((char *)test2, 2 * sizeof(DireEntry));//【测试3】根目录是否正确
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
				//释放测试空间
				delete[]test2;
				if (format)
				{
					//移动到1536#盘块起始处
					fs_disk.seekg(Image_SuperBlock::BM_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
					//申请测试空间
					test3 = new char[Image_BitMap::SYSTEM_BLOCK_DESC];
					//读取Image_BitMap的前256个字节
					fs_disk.read(test3, Image_BitMap::SYSTEM_BLOCK_DESC);
					//【测试3】系统盘块是否已经被占用（判断Image_BitMap的前256个字节是否均为‘\0xFF’）
					for (int i = 0; format && i <= Image_BitMap::SYSTEM_BLOCK_DESC - 1; i++)
					{
						if (test3[i] != '\xFF')
						{
							format = false;
						}
					}
					//释放测试空间
					delete[]test3;
				}
			}

			/*
				操作

				1. 内存里的磁盘映像是格式化完毕且没有任何内容的。
				2. 如果磁盘已经格式化，那么执行读入操作，即在内存中创建最新的磁盘映像。
				3. 如果磁盘没有格式化，那么执行写出操作，即实现磁盘格式化。
			*/

			if (format)
			{
				SyncIn();
			}
			else
			{
				Format();
			}

			/* 更新指针 */

			if (UpdatePointer == false)
			{
				//更新super_block里面的指针
				super_block.inode_table = &inode_table;
				super_block.bit_map = &bit_map;
				super_block.user_data = &user_data;
				super_block.SetWorkPath(work_path, disk_path, editor_path);
				//更新指向super_block对象的指针
				inode_table.SetSuperBlockAll(&super_block);
				bit_map.SetSuperBlock(&super_block);
				user_data.SetSuperBlock(&super_block);
				//更改指针更新标志
				UpdatePointer = true;
			}
		}

		//析构函数
		~Image()
		{
			fs_disk.close();
		}

		//创建工作路径
		void CreatePath()
		{
			//获取系统桌面路径
			TCHAR path[260];
			if (SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, false) == false)
			{
				cout << "无法获取桌面路径，程序退出" << endl;
				system("pause");
				exit(-1);
			}
			wcstombs(work_path, path, 600);
			//在桌面上创建“VolumeFile”文件夹
			strcat(work_path, "\\VolumeFile");
			if (_access(work_path, 0) == -1)
			{
				if (_mkdir(work_path) == -1)
				{
					cout << "无法创建“VolumeFile”文件夹，程序退出！" << endl;
					system("pause");
					exit(-1);
				}
			}
			//获取“disk.dat”路径（未创建文件）
			strcpy(disk_path, work_path);
			strcat(disk_path, "\\disk.dat");
			//获取“editor.dat”路径（未创建文件）
			strcpy(editor_path, work_path);
			strcat(editor_path, "\\editor.dat");
		}

		//创建“磁盘”
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
				cout << "创建“磁盘”失败！程序退出。" << endl;
				system("pause");
				exit(-1);
			}
			for (int i = 0; i <= Image_SuperBlock::ALL_BLOCK_NUM - 1; i++)
			{
				fout.write(one_block, Image_SuperBlock::BLOCK_CAP);
			}
			fout.close();
		}

		//格式化磁盘
		void Format()
		{
			/* 显示与计时 */

			cout << "开始格式化磁盘文件！" << endl;
			cout << "请稍候..." << endl;
			LARGE_INTEGER tick, fc_begin, fc_end;
			QueryPerformanceFrequency(&tick);		//获得时钟频率
			QueryPerformanceCounter(&fc_begin);		//获得初始硬件定时器计数

			/* 更新指针 */

			if (UpdatePointer == false)
			{
				//更新super_block里面的指针
				super_block.inode_table = &inode_table;
				super_block.bit_map = &bit_map;
				super_block.user_data = &user_data;
				super_block.SetWorkPath(work_path, disk_path, editor_path);
				//更新指向super_block对象的指针
				inode_table.SetSuperBlockAll(&super_block);
				bit_map.SetSuperBlock(&super_block);
				user_data.SetSuperBlock(&super_block);
				//更改指针更新标志
				UpdatePointer = true;
			}

			/* 创建文件系统的根目录 */

			CreateRootDirectory();

			/* 系统（BIOS与操作系统、文件系统超级块、InodeTable、位示图）盘块初始化 */

			//写指针复位
			fs_disk.seekp(0);
			//0#~199#（100KiB）：BIOS与操作系统
			fs_disk.write(operating_system.content, Image_SuperBlock::OS_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP);
			//200#（0.5KiB）：文件系统超级块
			fs_disk.write((char *)&(super_block), Image_SuperBlock::SB_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP);
			//201#~1535#（667.5KiB）：Inode节点表
			for (int i = 0; i <= Image_InodeTable::MAX_INODE_NUM - 1; i++)
			{
				fs_disk.write((char*)(&(inode_table.content[i])), Inode::INODE_SIZE);
			}
			//1536#~2047#（256KiB）：盘块位示图
			fs_disk.write(bit_map.content, Image_BitMap::ALL_BLOCK_DESC);

			/* 用户盘块初始化 */

			//2048#~2097151#（1023MiB）：用户数据
			//只要写出0#~5#用户盘块（2048#~2053#盘块），里面存放根目录的内容
			fs_disk.write(user_data.content, DireFile::FIXED_SIZE_BLOCK * Image_SuperBlock::BLOCK_CAP);

			/* 显示与计时 */

			QueryPerformanceCounter(&fc_end);		//获得终止硬件定时器计数
			cout << setiosflags(ios::fixed) << setprecision(3);
			cout << "格式化操作用时：" << double(fc_end.QuadPart - fc_begin.QuadPart) / tick.QuadPart << "秒" << endl;
			cout << "格式化操作完成！" << endl;
			cout << endl;
		}

		//创建文件系统的根目录
		void CreateRootDirectory()
		{
			unsigned int temp_v = 0;
			unsigned int *temp_p = nullptr;
			DireFile dir_file;

			//占用Inode节点表的0号Inode节点
			inode_table.content[0].Occupy();
			//设定为目录文件
			inode_table.content[0].SetModeDirectory(true);
			//分配“物理盘块”
			inode_table.content[0].RedistributeBlock(DireFile::FIXED_SIZE_BLOCK);
			//取出0#逻辑盘块对应的“物理盘块”
			temp_v = inode_table.content[0].Bmap(0, temp_p);
			//目录文件读写准备
			dir_file.Prepare(&super_block, 0);
			//初始化操作——所有m_ino设置为0xFFFFFFFF（否则后续操作会出错）
			for (unsigned int i = 0; i <= DireFile::DIRECTORY_ENTRY_NUM - 1; i++)
			{
				dir_file.content[i].m_ino = 0xFFFFFFFF;
			}
			//新建目录项：“.”
			dir_file.content[0].m_ino = 0;
			strcpy(dir_file.content[0].m_name, ".");
			//新建目录项：“..”
			dir_file.content[1].m_ino = 0;
			strcpy(dir_file.content[1].m_name, "..");
		}

		//将磁盘读入到内存映像
		void SyncIn()
		{
			/* 显示与计时 */

			cout << "开始将磁盘文件读入到内存映像！" << endl;
			cout << "请稍候..." << endl;
			LARGE_INTEGER tick, fc_begin, fc_end;
			QueryPerformanceFrequency(&tick);		//获得时钟频率
			QueryPerformanceCounter(&fc_begin);		//获得初始硬件定时器计数

			/* 读入系统（BIOS与操作系统、文件系统超级块、InodeTable、位示图）盘块 */

			//读指针复位
			fs_disk.seekg(0);
			//0#~199#（100KiB）：BIOS与操作系统（直接跳过）
			fs_disk.seekg(Image_SuperBlock::SB_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
			//200#（0.5KiB）：文件系统超级块（直接跳过）
			fs_disk.seekg(Image_SuperBlock::IT_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
			//201#~1535#（667.5KiB）：Inode节点表
			for (int i = 0; i <= Image_InodeTable::MAX_INODE_NUM - 1; i++)
			{
				fs_disk.read((char*)&(inode_table.content[i]), Inode::INODE_SIZE);
			}
			//1536#~2047#（256KiB）：盘块位示图
			fs_disk.read(bit_map.content, Image_BitMap::ALL_BLOCK_DESC);

			/* 读入用户盘块 */

			//2048#~2097151#（1023MiB）：用户数据
			fs_disk.read(user_data.content, Image_SuperBlock::USER_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP);

			/* 显示与计时 */

			QueryPerformanceCounter(&fc_end);		//获得终止硬件定时器计数
			cout << setiosflags(ios::fixed) << setprecision(3);
			cout << "读入操作用时：" << double(fc_end.QuadPart - fc_begin.QuadPart) / tick.QuadPart << "秒" << endl;
			cout << "读入操作完成！" << endl;
			cout << endl;
		}

		//将内存映像写出到磁盘
		void SyncOut()
		{
			/* 显示与计时 */

			cout << "开始将内存映像写出到磁盘文件！" << endl;
			cout << "请稍候..." << endl;
			LARGE_INTEGER tick, fc_begin, fc_end;
			QueryPerformanceFrequency(&tick);		//获得时钟频率
			QueryPerformanceCounter(&fc_begin);		//获得初始硬件定时器计数

			/* 写出系统（BIOS与操作系统、文件系统超级块、InodeTable、位示图）盘块 */

			//写指针复位
			fs_disk.seekp(0);
			//0#~199#（100KiB）：BIOS与操作系统（直接跳过）
			fs_disk.seekp(Image_SuperBlock::SB_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
			//200#（0.5KiB）：文件系统超级块（直接跳过）
			fs_disk.seekp(Image_SuperBlock::IT_BLOCK_START * Image_SuperBlock::BLOCK_CAP);
			//201#~1535#（667.5KiB）：Inode节点表
			for (int i = 0; i <= Image_InodeTable::MAX_INODE_NUM - 1; i++)
			{
				fs_disk.write((char*)&(inode_table.content[i]), Inode::INODE_SIZE);
			}
			//1536#~2047#（256KiB）：盘块位示图
			fs_disk.write(bit_map.content, Image_BitMap::ALL_BLOCK_DESC);

			/* 写出用户盘块 */

			//2048#~2097151#（1023MiB）：用户数据
			fs_disk.write(user_data.content, Image_SuperBlock::USER_BLOCK_NUM * Image_SuperBlock::BLOCK_CAP);

			/* 显示与计时 */

			QueryPerformanceCounter(&fc_end);		//获得终止硬件定时器计数
			cout << setiosflags(ios::fixed) << setprecision(3);
			cout << "写出操作用时：" << double(fc_end.QuadPart - fc_begin.QuadPart) / tick.QuadPart << "秒" << endl;
			cout << "写出操作完成！" << endl;
			cout << endl;
		}
};
