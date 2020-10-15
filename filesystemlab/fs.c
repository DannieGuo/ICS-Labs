/*
Filesystem Lab disigned and implemented by Liang Junkai,RUC
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fuse.h>
#include <errno.h>
#include "disk.h"

#define S_IFDIR 0040000
#define S_IFREG 0100000
#define DIRMODE (S_IFDIR|0755) //DIRMODE对应目录的mode 
#define REGMODE (S_IFREG|0644) //REGMODE对应普通文件的mode

//Format the virtual block device in the following function
#define FILE_NUM 32768  
#define MAX_FILE_NAME 24
#define SUPER_BLOCK 0
#define INODE_BITMAP 1
#define DATA_BITMAP1 2
#define DATA_BITMAP2 3
#define INODE_START 4
#define DATA_START 845
#define INODE_PER_BLOCK ((int)(BLOCK_SIZE/sizeof(Inode)))
#define DATA_BLOCK_NUM (BLOCK_NUM - 1) - DATA_START + 1
#define DIRENT_NUM_PRE_BLOCK ((int)((BLOCK_SIZE - sizeof(int))/sizeof(Dirent)))
#define ROOT_INODE_NUM 0

typedef struct Superblock{
	unsigned long f_bsize; // = BLOCK_SIZE 块大小 
	fsblkcnt_t f_blocks; // = BLOCK_NUM 块数量 
	fsblkcnt_t f_bfree; //空闲块数量 
	fsblkcnt_t f_bavail; //可用块数量 
	fsfilcnt_t f_files; //文件结点数 
	fsfilcnt_t f_ffree; //空闲结点数 
	fsfilcnt_t f_favail; //可用结点数 
	unsigned long f_namemax; // = 24 文件名长度上限 
}Superblock;


typedef struct Inode{
	mode_t mode; //文件 or 目录 
	off_t size;	//文件字节数
	time_t atime; //被访问的时间
	time_t ctime; //状态改变的时间 
	time_t mtime; //被修改的时间
	int block_num; //一共有几个块 
	int direct_pointer[15]; //记录data block的块号
	int indirect_pointer[2]; //indircet block的块号 
}Inode;


typedef struct Bitmap{
	int if_use[1024]; //int 32位,存32个位置的block是否被用
}Bitmap;
 

typedef struct Dirent{
	char file_name[MAX_FILE_NAME + 1];
	int inode_num;
}Dirent; 

typedef struct Dic_Datablock{
	int use_num; //该块里已用的数目 
	Dirent dirent[DIRENT_NUM_PRE_BLOCK];
}Dic_Datablock;

//写super block 
void Write_Superblock(Superblock superblock)
{
    char* temp = (char*)(&superblock);
    char temp_super[BLOCK_SIZE];
    memcpy(temp_super, temp, sizeof(Superblock));
    disk_write(SUPER_BLOCK, temp_super);
}

//获得super block 
Superblock Get_Superblock()
{
    Superblock superblock;
    char temp[BLOCK_SIZE];
    disk_read(SUPER_BLOCK, temp);
    superblock = *(Superblock*)temp;
    return superblock;
}

//找可用的inode或data block 
int Find_Free(int block_num)//block num: bitmap的块号 
{
	char temp[BLOCK_SIZE];
	disk_read(block_num, temp);
	Bitmap *bitmap = (Bitmap*)temp;
	
	int id_num = -1;
	int Mask = 0x1;	
	//0是空的, 1是被占用 
	for(int i = 0; i < 1024; i++)
	{
		int bit32 = bitmap->if_use[i];
		for(int shift = 0; shift < 32; shift++)
		{
			if(((bit32 >> shift) & Mask) == 0)//找到一个空的 
			{
				id_num = i * 32 + shift;
				return id_num;
			}
		}
	}
	//在data bitmap1里没找到 
	if(block_num == DATA_BITMAP1)
	{
		id_num = Find_Free(DATA_BITMAP2);
		id_num = id_num + 32768;
		if(id_num >= DATA_BLOCK_NUM) //data bitmap2后面有部分是多出来的 
			return -1;
		return id_num;
	}
	return id_num;  
}

//写入的时候改bitmap 
void Write_Bitmap(int block_num, int write_num)
{
	int Mask = 0x1;
	char temp[BLOCK_SIZE];
	if(write_num > 32768)
	{
		write_num = write_num - 32768;
		block_num = DATA_BITMAP2;
	}
	disk_read(block_num, temp);
	Bitmap *bitmap = (Bitmap*)temp;
	
	int i = (int)(write_num / 32);//每32个bit在一个int里 
	int shift = write_num - i * 32;
	bitmap->if_use[i] = (bitmap->if_use[i] | (Mask << shift));
	
	char temp_bitmap[BLOCK_SIZE];
	memcpy(temp_bitmap, (char*)bitmap, sizeof(Bitmap));
	disk_write(block_num, temp_bitmap);
    Superblock sbk = Get_Superblock();
    if(block_num > INODE_BITMAP)//data block
    {
        sbk.f_bfree--;
        sbk.f_bavail--;
    }
    else//inode 
    {
        sbk.f_ffree--;
        sbk.f_favail--;
    }
    Write_Superblock(sbk);
} 

//删除的时候改bitmap 
void Rm_Bitmap(int block_num, int write_num)
{
	int Mask = 0x1;
	char temp[BLOCK_SIZE];
	if(write_num > 32768)
	{
		write_num = write_num - 32768;
		block_num = DATA_BITMAP2;
	}
	write_num = write_num - DATA_START;
	disk_read(block_num, temp);
	Bitmap *bitmap = (Bitmap*)temp;
	
	int i = (int)(write_num / 32);//每32个bit在一个int里 
	int shift = write_num - i * 32;
	
	bitmap->if_use[i] = (bitmap->if_use[i] & (~(Mask << shift)));
	
	char temp_bitmap[BLOCK_SIZE];
	memcpy(temp_bitmap, (char*)bitmap, sizeof(Bitmap));
	disk_write(block_num, temp_bitmap);
	
    Superblock sbk = Get_Superblock();
    if(block_num > INODE_BITMAP)//data block
    {
        sbk.f_bfree++;
        sbk.f_bavail++;
    }
    else//inode
    {
        sbk.f_ffree++;
        sbk.f_favail++;
    }
    Write_Superblock(sbk);
}

//写inode 
void Write_Inode(int inode_num, Inode inode)
{//inode_num是记第几个inode,得算出块的位置 
	//第几个block 
	int inode_block_num = INODE_START + (int)(inode_num / INODE_PER_BLOCK);
	//block里的第几个inode 
	int off = inode_num - ((int)(inode_num/INODE_PER_BLOCK)) * INODE_PER_BLOCK;
	
	char temp[BLOCK_SIZE];
	disk_read(inode_block_num, temp);
	Inode *inodes = (Inode*)temp;
	inodes[off] = inode;
		
	char inode_temp[BLOCK_SIZE];
	memcpy(inode_temp, (char*)inodes, BLOCK_SIZE);
	disk_write(inode_block_num, inode_temp);
} 

//用inode_num读取inode 
Inode Get_Inode(int inode_num)
{
	int inode_block_num = INODE_START + (int)(inode_num / INODE_PER_BLOCK);
	int off = inode_num - ((int)(inode_num/INODE_PER_BLOCK)) * INODE_PER_BLOCK;
	char temp[BLOCK_SIZE];
	disk_read(inode_block_num, temp);
	Inode* inodes = (Inode*)temp;
	Inode inode = inodes[off];
	return inode;
} 

//初始化indirect block 
void Init_Indirect_Block(int indirect_block)
{
	char temp[BLOCK_SIZE];
	int num = 0;
	memcpy(temp, (char*)(&num), sizeof(int));
	disk_write(indirect_block, temp);
}

//往一个目录里插文件/子目录
void Insert_File_Into_Dic(int dic_inode_num, Dirent file_dirent)
{
    Inode dic_inode = Get_Inode(dic_inode_num);
	int temp_data_num = -1;
	int indirect_block = -1;
	char temp[BLOCK_SIZE];
	if(dic_inode.block_num == 0)
	{//得开一个新的块来放//就先开一个块放着,然后重入该函数 
		int free_data_block = Find_Free(DATA_BITMAP1);
		Write_Bitmap(DATA_BITMAP1, free_data_block);
		dic_inode.block_num = 1;
		free_data_block += DATA_START;
		dic_inode.direct_pointer[0] = free_data_block;
		dic_inode.size += BLOCK_SIZE;
		Write_Inode(dic_inode_num, dic_inode);
		//这个新块也得写一下
		char temp0[BLOCK_SIZE];
		Dic_Datablock new_dic_data;
		new_dic_data.use_num = 0;
		memcpy(temp0, (char*)&new_dic_data, sizeof(Dic_Datablock));
		disk_write(free_data_block, temp0);
		Insert_File_Into_Dic(dic_inode_num, file_dirent);
		return;
	}
	else
	if(dic_inode.block_num <= 15) //用的块的数目小于等于15,就还是direct pointer 
		temp_data_num = dic_inode.direct_pointer[dic_inode.block_num - 1];
	else
	if(dic_inode.block_num <= 15 + 1 + 1023)//第一个indirect pointer ,里面只有1023个 
	{
		//找最后一个指针指向的块
		indirect_block = dic_inode.indirect_pointer[0];
		char temp1[BLOCK_SIZE];
		disk_read(indirect_block, temp1);
		//indirect里的第一个用来计数 
		int num = ((int*)temp1)[0];//不会是0,因为有开indirectblock就得开一个新块 
		temp_data_num = ((int*)temp1)[num];
	}
	else//第二个indirect pointer 
	{
		indirect_block = dic_inode.indirect_pointer[1];
		char temp1[BLOCK_SIZE];
		disk_read(indirect_block, temp1);
		//indirect里的第一个用来计数 
		int num = ((int*)temp1)[0];//不会是0,因为有开indirectblock就得开一个新块 
		temp_data_num = ((int*)temp1)[num];
	}

	//temp_data_num是找到的最后存的块 
	disk_read(temp_data_num, temp);
	Dic_Datablock dic_data = *(Dic_Datablock*)temp;
	if(dic_data.use_num < DIRENT_NUM_PRE_BLOCK)//这一块还可以用 
	{
		dic_data.dirent[dic_data.use_num] = file_dirent;
		dic_data.use_num++;
		char temp1[BLOCK_SIZE];
		memcpy(temp1, (char*)&dic_data, sizeof(Dic_Datablock));
        disk_write(temp_data_num, temp1);	
		//然后改inode
		//dic_inode.atime = time(NULL);
	 	dic_inode.mtime = time(NULL);
		dic_inode.ctime = time(NULL);
		Write_Inode(dic_inode_num, dic_inode);
		return;
	}
	else
	{//这块已经满了,得重开一个datablock来放 
		//只重开一个块,重开完还是重新进入这个函数来做 
		//新数据块,已经写回去了 
		int free_data_block = Find_Free(DATA_BITMAP1);
		Write_Bitmap(DATA_BITMAP1, free_data_block);
		free_data_block += DATA_START;
		char temp0[BLOCK_SIZE];
		Dic_Datablock new_dic_data;
		new_dic_data.use_num = 0;
		memcpy(temp0, (char*)&new_dic_data, sizeof(Dic_Datablock));
		disk_write(free_data_block, temp0);
		dic_inode.block_num ++;
		dic_inode.size += BLOCK_SIZE;
		//改inode 
		if(dic_inode.block_num <= 15)//direct pointer还没满 
		{
			dic_inode.direct_pointer[dic_inode.block_num - 1] = free_data_block;
			Write_Inode(dic_inode_num, dic_inode);
			Insert_File_Into_Dic(dic_inode_num, file_dirent);
			return;
		}
		else
		if(dic_inode.block_num == 16)//刚好超了,要用indirect1
		{//就把indirect的块分了,然后重入即可 
			dic_inode.block_num ++;//一个indirect块
			dic_inode.size += BLOCK_SIZE;
			//分indirect块
			int indirect_block = Find_Free(DATA_BITMAP1);
			Write_Bitmap(DATA_BITMAP1, indirect_block);
			indirect_block += DATA_START;
			Init_Indirect_Block(indirect_block);
			dic_inode.indirect_pointer[0] = indirect_block;
			Write_Inode(dic_inode_num, dic_inode);
			
			char tempin[BLOCK_SIZE];
			((int*)tempin)[0] = 1;
			((int*)tempin)[1] = free_data_block;
			disk_write(indirect_block, tempin);
			Insert_File_Into_Dic(dic_inode_num, file_dirent);
			return;
		}
		else
		if(dic_inode.block_num == 15 + 1 + 1023 + 1)//indirect1刚好超了 
		{
			dic_inode.block_num ++;//一个indirect块
			dic_inode.size += BLOCK_SIZE;
			int indirect_block = Find_Free(DATA_BITMAP1);
			Write_Bitmap(DATA_BITMAP1, indirect_block);
			indirect_block += DATA_START;
			Init_Indirect_Block(indirect_block);
			dic_inode.indirect_pointer[1] = indirect_block;
			Write_Inode(dic_inode_num, dic_inode);
			char tempin[BLOCK_SIZE];
			((int*)tempin)[0] = 1;
			((int*)tempin)[1] = free_data_block;
			disk_write(indirect_block, tempin);
			Insert_File_Into_Dic(dic_inode_num, file_dirent);
			return;
		}
		else//indirect
		{
			Write_Inode(dic_inode_num, dic_inode);
			char temp1[BLOCK_SIZE];
			disk_read(indirect_block, temp1);
			((int*)temp1)[0] += 1;
			int num = ((int*)temp1)[0]; 
			((int*)temp1)[num] = free_data_block;
			disk_write(indirect_block, temp1);
			Insert_File_Into_Dic(dic_inode_num, file_dirent);
			return;
		}
	}
} 

//从目录的data block找文件/子目录的inode_num 
int Find_In_Dic_Data(int data_block, char* name)
{
	char temp[BLOCK_SIZE];
	disk_read(data_block, temp);
	Dic_Datablock* data = (Dic_Datablock*)temp;
	for(int i = 0; i < data->use_num; i++)
	{
		if(strcmp(data->dirent[i].file_name, name) == 0)
		{
			return data->dirent[i].inode_num;
		}
	}
	return -1;
}

//找到文件/子目录是在存目录的哪个data block里  
int Find_In_Dic(int dic_inode_num, char* name)
{
	int find_inode_num = -1;
        Inode dic_inode = Get_Inode(dic_inode_num);
	//不管多少个block,前面的block都是要找的,就先找了再判断后面还有没有要找的
	int direct_len;
	if(dic_inode.block_num < 15)
		direct_len = dic_inode.block_num;
	else
		direct_len = 15;
	for(int i = 0; i < direct_len; i++)
	{
		//datablock的块号, 要到这个块里找name 
		find_inode_num = Find_In_Dic_Data(dic_inode.direct_pointer[i], name);
		if(find_inode_num != -1)
			return dic_inode.direct_pointer[i];
			//return find_inode_num;
	} 
	if(dic_inode.block_num > 15)
	{//找第一个indirect block 
		//dic_inode.indirect_pointer[0] indirect块里的datablock号 
		char temp[BLOCK_SIZE];
		disk_read(dic_inode.indirect_pointer[0], temp);
		int len = ((int*)temp)[0];
		for(int i = 1; i <= len; i++)
		{
			find_inode_num = Find_In_Dic_Data(((int*)temp)[i], name);
			if(find_inode_num != -1)
				return ((int*)temp)[i];
				//return find_inode_num;
		}
		if(dic_inode.block_num > 15 + 1 + 1023)
		{//找第二个indirect block 
			char temp[BLOCK_SIZE];
			disk_read(dic_inode.indirect_pointer[1], temp);
			int len = ((int*)temp)[0];
			for(int i = 1; i <= len; i++)
			{
				find_inode_num = Find_In_Dic_Data(((int*)temp)[i], name);
				if(find_inode_num != -1)
					return ((int*)temp)[i];
					//return find_inode_num;
			}
		}
	}
	return -1;
	//return find_inode_num;
}

//fs_readdir中用到的, 读出一个data block里的所有文件 
void Print_Dic_Data(int data_block_num, void *buffer, fuse_fill_dir_t filler)
{
	char temp[BLOCK_SIZE];
    disk_read(data_block_num, temp);
    Dic_Datablock data;
    data = *(Dic_Datablock*)temp;
    int len = data.use_num;
    for(int i = 0; i < len; i++)
        filler(buffer, data.dirent[i].file_name, NULL, 0);    
}

//得到父目录的路径 
char* Get_Father_Path(char* path)
{
    int len = strlen(path);
    char* father_path = (char*)malloc(len);
    int pos = len - 1;
    while(path[pos] != '/')
        pos--;
    for(int i = 0; i < pos; i++)
        father_path[i] = path[i];
    father_path[pos] = '\0';//空字符,字符串结束 
    return father_path;
}

//得到文件名 
char* Get_File_Name(char* path)
{
    int len = strlen(path);
    char* name = (char*)malloc(len);
    int pos = len - 1;
    while(path[pos] != '/')
        pos--;
    for(int i = pos + 1; i < len; i++)
        name[i - pos - 1] = path[i];
    name[len - pos - 1] = '\0';//空字符,字符串结束
    return name;
}

//路径得到对应的inode num 
int Path_To_Inode_Num(char* path)
{
    int inode_num = -1;
    int father_inode_num = ROOT_INODE_NUM;
    int st = 1;
    int len = strlen(path);
    char name[MAX_FILE_NAME + 1];
    while(st < len)
    {
        int off = 0;
        while(path[st] != '/' && st < len)
            name[off++] = path[st++];
        name[off++] = '\0';
        st++;
        int data_block = Find_In_Dic(father_inode_num, name);
        if(data_block == -1)
        	return -1;
        father_inode_num = Find_In_Dic_Data(data_block, name);
    }
    return father_inode_num;
}

int mkfs()
{	
	Superblock superblock;
	superblock.f_bsize = BLOCK_SIZE;
	superblock.f_blocks = BLOCK_NUM;
	superblock.f_bfree = BLOCK_NUM - 4;
	superblock.f_bavail = BLOCK_NUM - 4; //去掉已经占的那些 
	superblock.f_files = FILE_NUM;
	superblock.f_ffree = FILE_NUM;
	superblock.f_favail = FILE_NUM;
	superblock.f_namemax = MAX_FILE_NAME;
	Write_Superblock(superblock);
	
	//bitmap初始化 
	Bitmap bitmap0;
	for(int i = 0; i < 1024; i++)
		bitmap0.if_use[i] = 0;
	char temp_bitmap[BLOCK_SIZE];
	memcpy(temp_bitmap, (char*)(&bitmap0), sizeof(Bitmap));
	
	disk_write(INODE_BITMAP, temp_bitmap);
	disk_write(DATA_BITMAP1, temp_bitmap);
		 
	//根目录信息
	int inode_num = Find_Free(INODE_BITMAP);//1.找个空的inode 
	Write_Bitmap(INODE_BITMAP, inode_num);//2.写bitmap
	
	Inode root;
	root.mode = DIRMODE;
	root.atime = time(NULL);
	root.ctime = time(NULL);
	root.mtime = time(NULL); 
	root.block_num = 0;	//目前还没有用数据块
	root.size = 0;
	
	Write_Inode(inode_num, root);//3.写inode
	return 0;
}

int fs_getattr (const char *path, struct stat *attr)
{	
	printf("Getattr is called: %s\n", path);
	
    int inode_num = Path_To_Inode_Num(path);
	if(inode_num == -1)
		return -ENOENT;
		
    Inode inode = Get_Inode(inode_num);
	attr->st_mode = inode.mode;
	attr->st_nlink = 1;
	attr->st_uid = getuid();
	attr->st_gid = getgid();
	attr->st_size = inode.size;
	attr->st_atime = inode.atime;
	attr->st_mtime = inode.mtime;
	attr->st_ctime = inode.ctime;
	return 0;
}

int fs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	printf("Readdir is called:%s\n", path);
    int inode_num = Path_To_Inode_Num(path);
	if(inode_num == -1)
		return -ENOENT;
		
	//把now_inode_num目录里的所有文件名打印出来
    Inode dic_inode = Get_Inode(inode_num);
	int direct_len = dic_inode.block_num <= 15? dic_inode.block_num : 15;
	for(int i = 0; i < direct_len; i++)
	{//把目录的这个数据块里的文件名都打印出来 
		Print_Dic_Data(dic_inode.direct_pointer[i], buffer, filler);
	}
	if(dic_inode.block_num > 15)
	{
		char temp[BLOCK_SIZE];
		disk_read(dic_inode.indirect_pointer[0], temp);
		int len = ((int*)temp)[0];
		for(int i = 1; i <= len; i++)
			Print_Dic_Data(((int*)temp)[i], buffer, filler);
		if(dic_inode.block_num > 15 + 1 + 1023)
		{
			char temp[BLOCK_SIZE];
			disk_read(dic_inode.indirect_pointer[1], temp);
			int len = ((int*)temp)[0];
			for(int i = 1; i <= len; i++)
				Print_Dic_Data(((int*)temp)[i], buffer, filler);
		}
	}
	dic_inode.atime = time(NULL);
	Write_Inode(inode_num, dic_inode);
	return 0;
}

int fs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("Read is called:%s\n", path);
    int inode_num = Path_To_Inode_Num(path);
	Inode inode = Get_Inode(inode_num);
	inode.atime = time(NULL);
	Write_Inode(inode_num, inode);
	
	char file_content[inode.size];
	char temp[BLOCK_SIZE];
	int num = inode.block_num <= 15 ? inode.block_num : 15;
	off_t off = 0; //在文件里读到的地方 
	int done = 0;
	//direct block
	for(int i = 0; i < num; i++)
	{
		disk_read(inode.direct_pointer[i], temp);
		size_t now_size = inode.size - off < BLOCK_SIZE? inode.size - off : BLOCK_SIZE;
		memcpy(file_content + off, temp, now_size);
		off += now_size;
		if(off >= offset + size)//从direct里读出的大小已经够了 
		{
			done = 1;
			break;
		}
	}
	if(done == 0)
	{//如果还有indirect的可以读 
		 if(inode.block_num > 15)
		 {
		 	int indirect_block_num = inode.indirect_pointer[0];
		 	char intemp[BLOCK_SIZE];
		 	disk_read(indirect_block_num, intemp);
		 	int num = ((int*)temp)[0];
		 	for(int i = 1; i <= num; i++)
		 	{
		 		disk_read(((int*)temp)[i], temp);
		 		size_t now_size = inode.size - off < BLOCK_SIZE? inode.size - off : BLOCK_SIZE;
				memcpy(file_content + off, temp, now_size);
				off += now_size;
				if(off >= offset + size)//大小已经够了 
				{
					done = 1;
					break;
				}
			}
		 }
		 if(done == 0)
		 {
		 	if(inode.block_num > 15 + 1 + 1023)
		 	{
		 		int indirect_block_num = inode.indirect_pointer[1];
			 	char intemp[BLOCK_SIZE];
			 	disk_read(indirect_block_num, intemp);
			 	int num = ((int*)temp)[0];
			 	for(int i = 1; i <= num; i++)
			 	{
			 		disk_read(((int*)temp)[i], temp);
			 		size_t now_size = inode.size - off < BLOCK_SIZE? inode.size - off : BLOCK_SIZE;
					memcpy(file_content + off, temp, now_size);
					off += now_size;
					if(off >= offset + size)//大小已经够了 
					{
						done = 1;
						break;
					}
				}	
			}
		 }
	}
	//实际可以读到的大小 
	size_t copy_size = size < inode.size - offset? size:inode.size - offset;
	//复制实际读到的内容 
	memcpy(buffer, file_content + offset, copy_size);
	return copy_size;
}

int fs_mknod (const char *path, mode_t mode, dev_t dev)
{
	printf("Mknod is called:%s\n", path);
    int father_inode_num = Path_To_Inode_Num(Get_Father_Path(path));
	if(father_inode_num == -1)
		return -ENOSPC;
		
	int file_inode_num = Find_Free(INODE_BITMAP);
	if(file_inode_num == -1)
		return -ENOSPC;
	Write_Bitmap(INODE_BITMAP, file_inode_num);
	
	Inode file;
	file.atime = time(NULL);
	file.ctime = time(NULL);
	file.mtime = time(NULL);
	file.block_num = 0;
	file.mode = REGMODE;	
	file.size = 0;
	Write_Inode(file_inode_num, file);
		
	Dirent file_dirent;
	file_dirent.inode_num = file_inode_num;
	strcpy(file_dirent.file_name, Get_File_Name(path));
	
	//在insert里有改父目录时间 
	Insert_File_Into_Dic(father_inode_num, file_dirent);
	return 0;
}

int fs_mkdir (const char *path, mode_t mode)
{
	printf("mkdir is called\n");
	int inode_num = Find_Free(INODE_BITMAP);
	
	if(inode_num == -1)
		return -ENOSPC;
	Write_Bitmap(INODE_BITMAP, inode_num);
	
	Inode new_dic;
	new_dic.mode = DIRMODE;
	new_dic.block_num = 0;
	new_dic.size = 0;
	new_dic.atime = time(NULL);
	new_dic.ctime = time(NULL);
	new_dic.mtime = time(NULL);
	Write_Inode(inode_num, new_dic);
	
	Dirent dirent;
	dirent.inode_num = inode_num;
	strcpy(dirent.file_name, Get_File_Name(path));
	
    int father_inode_num = Path_To_Inode_Num(Get_Father_Path(path));
	if(father_inode_num == -1)
		return -ENOSPC;
		
	Insert_File_Into_Dic(father_inode_num, dirent);
	return 0;
}

int fs_rmdir (const char *path)
{
	printf("Rmdir is called:%s\n",path);
	
    int father_inode_num = Path_To_Inode_Num(Get_Father_Path(path));
	Inode father_inode = Get_Inode(father_inode_num);
	father_inode.mtime = time(NULL);
	father_inode.ctime = time(NULL);
	Write_Inode(father_inode_num, father_inode);
	
	char *name = Get_File_Name(path);
	int father_data_block = Find_In_Dic(father_inode_num, name);
	char temp[BLOCK_SIZE];
	disk_read(father_data_block, temp);
	Dic_Datablock* data = (Dic_Datablock*)temp;
	int find = 0;
	for(int i = 0; i < data->use_num; i++)
	{
		if(strcmp(data->dirent[i].file_name, name) == 0)
			find = 1;
		if(find == 1)
		{
			if(i < data->use_num - 1)
				data->dirent[i] = data->dirent[i + 1];
		}
	}
	data->use_num--;
	memcpy(temp, (char*)data, sizeof(Dic_Datablock));
	disk_write(father_data_block, temp);
	
	int inode_num = Find_In_Dic_Data(father_data_block, name);
	Inode inode = Get_Inode(inode_num);
	int num = inode.block_num <= 15? inode.block_num : 15;
	for(int i = 0; i < num; i++)
		Rm_Bitmap(DATA_BITMAP1, inode.direct_pointer[i]);
	if(inode.block_num > 15)
	{
		int indirect = inode.indirect_pointer[0];
		char temp1[BLOCK_SIZE];
		disk_read(indirect, temp1);
		int len = ((int*)temp1)[0]; 
		for(int i = 1; i <= len; i++) 
			Rm_Bitmap(DATA_BITMAP1, ((int*)temp1)[i]);
			
		if(inode.block_num > 15 + 1 + 1023)
		{
			indirect = inode.indirect_pointer[1];
			disk_read(indirect, temp1);
			len = ((int*)temp1)[0];
			for(int i = 1; i <= len; i++) 
				Rm_Bitmap(DATA_BITMAP1, ((int*)temp1)[i]);
		}
	}
	Rm_Bitmap(INODE_BITMAP, inode_num);
	return 0;
}

int fs_unlink (const char *path)
{
	printf("Unlink is callded:%s\n",path);
    int father_inode_num = Path_To_Inode_Num(Get_Father_Path(path));
	Inode father_inode = Get_Inode(father_inode_num);
	father_inode.mtime = time(NULL);
	father_inode.ctime = time(NULL);
	Write_Inode(father_inode_num, father_inode);
	
	char *name = Get_File_Name(path); 
	int father_data_block = Find_In_Dic(father_inode_num, name);
	char temp[BLOCK_SIZE];
	disk_read(father_data_block, temp);
	Dic_Datablock* data = (Dic_Datablock*)temp;
	int find = 0;
	for(int i = 0; i < data->use_num; i++)
	{
		if(strcmp(data->dirent[i].file_name, name) == 0)
			find = 1;
		if(find == 1)
		{
			if(i < data->use_num - 1)
				data->dirent[i] = data->dirent[i + 1];
		}
	}
	data->use_num--;
	memcpy(temp, (char*)data, sizeof(Dic_Datablock));
	disk_write(father_data_block, temp);
	 
	int inode_num = Find_In_Dic_Data(father_data_block, name);
	Inode inode = Get_Inode(inode_num);
	int num = inode.block_num <= 15? inode.block_num : 15;
	for(int i = 0; i < num; i++)
		Rm_Bitmap(DATA_BITMAP1, inode.direct_pointer[i]);
	if(inode.block_num > 15)
	{
		int indirect = inode.indirect_pointer[0];
		char temp1[BLOCK_SIZE];
		disk_read(indirect, temp1);
		int len = ((int*)temp1)[0];
		for(int i = 1; i <= len; i++) 
			Rm_Bitmap(DATA_BITMAP1, ((int*)temp1)[i]);
			
		if(inode.block_num > 15 + 1 + 1023)
		{
			indirect = inode.indirect_pointer[1];
			disk_read(indirect, temp1);
			len = ((int*)temp1)[0];
			for(int i = 1; i <= len; i++) 
				Rm_Bitmap(DATA_BITMAP1, ((int*)temp1)[i]);
		}
	}
	Rm_Bitmap(INODE_BITMAP, inode_num);
	return 0;
}

int fs_rename (const char *oldpath, const char *newname)
{
	printf("Rename is called:%s\n", oldpath);
     
    int old_father_num = Path_To_Inode_Num(Get_Father_Path(oldpath));
	if(old_father_num == -1)
		return -ENOSPC;
    int new_father_num = Path_To_Inode_Num(Get_Father_Path(newname));
	if(new_father_num == -1)
		return -ENOSPC;
	char* old_name = Get_File_Name(oldpath);
	char* new_name = Get_File_Name(newname);
	Dirent file;
	//把第一个目录中的dirent去掉
	int father_data_block = Find_In_Dic(old_father_num, old_name);
	if(father_data_block == -1)
		return -ENOSPC;
	char temp[BLOCK_SIZE];
	disk_read(father_data_block, temp);
	Dic_Datablock* data = (Dic_Datablock*)temp;
	int find = 0;
	for(int i = 0; i < data->use_num; i++)
	{
		if(strcmp(data->dirent[i].file_name, old_name) == 0)
		{
			find = 1;
			file = data->dirent[i];
		}
		if(find == 1)
		{
			if(i < data->use_num - 1)
				data->dirent[i] = data->dirent[i + 1];
		}
	}
	data->use_num--;
	memcpy(temp, (char*)data, sizeof(Dic_Datablock));
	disk_write(father_data_block, temp);
	
	//把dirent加到第二个目录中
	strcpy(file.file_name, new_name);
	Insert_File_Into_Dic(new_father_num, file);
	return 0;
}

int File_i_Data_Block(Inode inode, int i)
{
	int num = inode.block_num;
	if(num <= 15)
		return inode.direct_pointer[num - 1];
	else
	if(num <= 15 + 1 + 1023)
	{
		i = i - 15;
		int indirect_block = inode.indirect_pointer[0];
		char temp[BLOCK_SIZE];
		disk_read(indirect_block, temp);
		return ((int*)temp)[i];
	}
	else
	{
		i = i - 15 - 1023;
		int indirect_block = inode.indirect_pointer[1];
		char temp[BLOCK_SIZE];
		disk_read(indirect_block, temp);
		return ((int*)temp)[i];
	}
}

int fs_write (const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("Write is called:%s\n", path);
	off_t new_size = offset + size;
	if(fs_truncate(path, new_size) == -ENOSPC)
		return 0;
	
    Inode inode = Get_Inode(Path_To_Inode_Num(path));
	char temp[BLOCK_SIZE];
	
	//要写入的内容是要存在这个文件的第几个block里 
	int start_block = (int)(offset/BLOCK_SIZE);
	int end_block = (int)((offset + size - 1)/BLOCK_SIZE);
	
	//从block的哪个位置开始 
	int start_off = offset % BLOCK_SIZE;
	int end_off = (offset + size - 1) % BLOCK_SIZE;
	
	//写到哪里了 
	int buffer_off = 0;
	
	for(int i = start_block; i <= end_block; i++)
	{
		//要存入的那个datablock的块号 
		int data_block = File_i_Data_Block(inode, i);
		if(i == start_block)
		{
			disk_read(data_block, temp);
			memcpy(temp + start_off, buffer, BLOCK_SIZE - start_off);
			buffer_off += BLOCK_SIZE - start_off;
			if(i == end_block)
				memcpy(temp + start_off, buffer, end_off - start_off + 1);	
		}
		else
		if(i == end_block)
			memcpy(temp, buffer + buffer_off, end_off + 1);
		else
		{
			memcpy(temp, buffer + buffer_off, BLOCK_SIZE);
			buffer_off += BLOCK_SIZE;
		}
		disk_write(data_block, temp);
	}
	return size;
}

int fs_truncate (const char *path, off_t size)
{
	printf("Truncate is called:%s\n", path);
    int father_num = Path_To_Inode_Num(Get_Father_Path(path));
	char* name = Get_File_Name(path);
	int inode_num = Find_In_Dic_Data(Find_In_Dic(father_num, name), name);
	Inode inode = Get_Inode(inode_num);
	int old_block_num = inode.block_num;
	
	int new_block_num = (int)(size / BLOCK_SIZE);
	if(size - new_block_num * BLOCK_SIZE > 0 )
		new_block_num++;
	
	if(inode.size < size)
	{
		int add_block_num = new_block_num - old_block_num;
		while(inode.block_num < 15 && add_block_num > 0)
		{//新加的块还能放在direct里 
			add_block_num--;
			int new_data_block = Find_Free(DATA_BITMAP1);
			if(new_data_block == -1)
				return -ENOSPC;
			Write_Bitmap(DATA_BITMAP1, new_data_block);
			new_data_block += DATA_START;
			inode.direct_pointer[inode.block_num] = new_data_block;
			inode.block_num++;
		}
		//新的块还没分配完 
		while(add_block_num > 0)//如果第二个块分完还不够的话就返回 
		{
			//得新分一个indirect block 
			if(inode.block_num == 15 || inode.block_num == 15 + 1 + 1023)
			{
				int indirect_block = Find_Free(DATA_BITMAP1);
				if(indirect_block == -1)
					return -ENOSPC;
				Write_Bitmap(DATA_BITMAP1, indirect_block);
				indirect_block += DATA_START;
				if(inode.block_num == 15)
					inode.indirect_pointer[0] = indirect_block;
				else
					inode.indirect_pointer[1] = indirect_block;
				inode.block_num++;
				inode.size += BLOCK_SIZE;
				Init_Indirect_Block(indirect_block);
			}
			//分剩下的新块
			//第一个块里的
			if(inode.block_num < 15 + 1 + 1023)//第一个块可以分
			{
				int indirect_block = inode.indirect_pointer[0];
				char temp[BLOCK_SIZE];
				disk_read(indirect_block, temp);
				while(((int*)temp)[0] < 1023 && add_block_num > 0)
				{
					add_block_num--;
					int new_data_block = Find_Free(DATA_BITMAP1);
					if(new_data_block == -1)
						return -ENOSPC;
					Write_Bitmap(DATA_BITMAP1, new_data_block);
					new_data_block += DATA_START;
					((int*)temp)[((int*)temp)[0] + 1] = new_data_block;
					((int*)temp)[0]++;
				}
				//写回去 
				disk_write(indirect_block, temp); 
			} 
			//如果还没分完
			//找第二个块
			//够不够分 
			else//第二次进循环才//还不够分就返回 
			{
				int indirect_block = inode.indirect_pointer[1];
				char temp[BLOCK_SIZE];
				disk_read(indirect_block, temp);
				while(((int*)temp)[0] < 1023 && add_block_num > 0)
				{
					add_block_num--;
					int new_data_block = Find_Free(DATA_BITMAP1);
					if(new_data_block == -1)
						return -ENOSPC;
					Write_Bitmap(DATA_BITMAP1, new_data_block);
					new_data_block += DATA_START;
					((int*)temp)[((int*)temp)[0] + 1] = new_data_block;
					((int*)temp)[0]++;
				}
				//写回去 
				disk_write(indirect_block, temp); 
				if(add_block_num > 0)
					return -ENOSPC;
			}
		}
		
		if(add_block_num > 0)
			return -ENOSPC;	
	}
	else 
	{
		for(int i = old_block_num; i > new_block_num; i--)
		{
			if(inode.block_num <= 15)
			{
				Rm_Bitmap(DATA_BITMAP1, inode.direct_pointer[inode.block_num - 1]);
				inode.block_num--;
			}
			else
			if(inode.block_num <= 15 + 1 + 1023)
			{
				char temp[BLOCK_SIZE];
				disk_read(inode.indirect_pointer[0], temp);
				((int*)temp)[0]--;
				inode.block_num--;
				//如果刚好整个indirectblock都空了 
				if(((int*)temp)[0] == 0)
				{
					Rm_Bitmap(DATA_BITMAP1, inode.indirect_pointer[0]);
					inode.block_num--;
				}
				else
				{
					Rm_Bitmap(DATA_BITMAP1, ((int*)temp)[((int*)temp)[0] + 1]);
				}
			}
			else
			{
				char temp[BLOCK_SIZE];
				disk_read(inode.indirect_pointer[1], temp);
				((int*)temp)[0]--;
				inode.block_num--;
				//如果刚好整个indirectblock都空了 
				if(((int*)temp)[0] == 0)
				{
					Rm_Bitmap(DATA_BITMAP1, inode.indirect_pointer[1]);
					inode.block_num--;
				}
				else
				{
					Rm_Bitmap(DATA_BITMAP1, ((int*)temp)[((int*)temp)[0] + 1]);
				}
			}
		}
	}
	inode.size = size;
	inode.ctime = time(NULL);
	Write_Inode(inode_num, inode);
	return 0;
}

int fs_utime (const char *path, struct utimbuf *buffer)
{
	printf("Utime is called:%s\n",path);
    int inode_num = Path_To_Inode_Num(path);
    Inode inode = Get_Inode(inode_num);
    inode.atime = buffer->actime;
    inode.mtime = buffer->modtime;
    Write_Inode(inode_num, inode);
	return 0;
}

int fs_statfs (const char *path, struct statvfs *stat)
{
    printf("Statfs is called:%s\n", path);
    Superblock spb = Get_Superblock();
    stat->f_bsize = spb.f_bsize;
    stat->f_blocks = spb.f_blocks;
    stat->f_bfree = spb.f_bfree;
    stat->f_bavail = spb.f_bavail;
    stat->f_files = spb.f_files;
    stat->f_ffree = spb.f_ffree;
    stat->f_favail = spb.f_favail;
    stat->f_namemax = spb.f_namemax;
	return 0;
}

int fs_open (const char *path, struct fuse_file_info *fi)
{
	printf("Open is called:%s\n", path);
	return 0;
}

//Functions you don't actually need to modify
int fs_release (const char *path, struct fuse_file_info *fi)
{
	printf("Release is called:%s\n",path);
	return 0;
}

int fs_opendir (const char *path, struct fuse_file_info *fi)
{
	printf("Opendir is called:%s\n",path);
	return 0;
}

int fs_releasedir (const char * path, struct fuse_file_info *fi)
{
	printf("Releasedir is called:%s\n",path);
	return 0;
}

static struct fuse_operations fs_operations = {
	.getattr    = fs_getattr,
	.readdir    = fs_readdir,
	.read       = fs_read,
	.mkdir      = fs_mkdir,
	.rmdir      = fs_rmdir,
	.unlink     = fs_unlink,
	.rename     = fs_rename,
	.truncate   = fs_truncate,
	.utime      = fs_utime,
	.mknod      = fs_mknod,
	.write      = fs_write,
	.statfs     = fs_statfs,
	.open       = fs_open,
	.release    = fs_release,
	.opendir    = fs_opendir,
	.releasedir = fs_releasedir
};

int main(int argc, char *argv[])
{
	if(disk_init())
		{
		printf("Can't open virtual disk!\n");
		return -1;
		}
	if(mkfs())
		{
		printf("Mkfs failed!\n");
		return -2;
		}
    return fuse_main(argc, argv, &fs_operations, NULL);
}
