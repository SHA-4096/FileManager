#pragma once
#include<tchar.h>
#include"model.h"
#include"util.h"
#define MAX_PATHLEN 1000
#define MODE_MYDIR 0
#define MODE_MYFILE 1
#define MODE_MYSTAT 2
#define MAX_ARG_COUNT 4
class ControllerClass{
public:
	DirectoryTree* DirTree;
	int SetCommandScript(TCHAR* Path);
	int ScanDir(TCHAR* Path);
	int ExecuteCommand();
	TCHAR WriteBuf[MAX_PATHLEN * 5];//输出数据的buffer
private:
	TCHAR ReadBuf[MAX_PATHLEN * 3];//从文件中读取数据的buffer
	TCHAR Args[4][MAX_PATHLEN];//用来存储命令参数
	int GetFolderStat(TCHAR* Path);
	int CommandTrim();
	int WriteResult();
	FILE* ReadFp;
	FILE* WriteFp;
};
