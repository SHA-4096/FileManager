#pragma once
#include<tchar.h>
#include<vector>
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
	int Debug();
	int BeginFolderStatCompare();//�����ļ�����������ֱ�ӱ�Wrapper����
	int BeginFullDiffCompare();//�����ļ�����������ֱ�ӱ�Wrapper����

	std::vector<int> FolderStatNodes;//Ҫͳ���ļ��и�����Ϣ�仯�Ľڵ�
	std::vector<int> FullDiffNodes;//Ҫͳ����Ŀ¼����Ľڵ�
	TCHAR WriteBuf[MAX_PATHLEN * 5];//������ݵ�buffer
	
private:
	TCHAR ReadBuf[MAX_PATHLEN * 3];//���ļ��ж�ȡ���ݵ�buffer
	TCHAR Args[4][MAX_PATHLEN];//�����洢�������
	int CmpFolderDiff(TCHAR* PathOrigin, TCHAR* PathNew);
	int CmpFullDiff(TCHAR* PathOrigin, TCHAR* PathNew, TCHAR* OutputPath);//ûɶ�ã������ʱ��ǵ����
	int GetFolderStat(TCHAR* Path);
	int CommandTrim();
	int WriteResult();
	FILE* ReadFp;
	FILE* WriteFp;
};
