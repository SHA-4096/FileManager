#pragma once
#include<locale.h>
#include<queue>
#include<stdio.h>
#include<tchar.h>
#include<map>
#include<windows.h>
#include<vector>
#include"util.h"
#include<time.h>
#define MAX_PATHLEN 1000
#define MAX_NODE_COUNT 500000
#define CHILD_ID(x) x->Child ? x->Child->NodeId:0
#define PARENT_ID(x) x->Parent ? x->Parent->NodeId:0
#define SIBLING_ID(x) x->Sibling ? x->Sibling->NodeId:0
#define SELF_ID(x) x ? x->NodeId:0

#define Unit_KB 0
#define Unit_MB 1
#define Unit_GB 2
#define Unit_TB 3

#define MAX_SQL_LINES 25000
#define WINDOWS_UNIX_TIME_DIFF 11644473600

/*
const TCHAR* UnitStr[] = { _T("KB"),_T("MB"),_T("GB"),_T("TB") };
const INT64 UnitSize[] = { 1024,1024*1024,1024*1024*1024,1024*1024*1024*1024 };

struct FileSizeStruct {
	INT64 Size;
	short Unit;
};
*/

class Node {
public:
	Node(WIN32_FIND_DATA* Data,int Depth,int NodeId,TCHAR* PathName);
	Node(INT64 Size, UINT64 ModifiedTime,DWORD FileAttribute,int Depth, int NodeId, TCHAR* PathName);
	TCHAR PathName[MAX_PATHLEN];
	DWORD FileAttribute;
	UINT64 ModifiedTime;
	int Depth;//文件树深度
	int RealDep;//数据结构真实深度
	int NodeId;
	INT64 FileSize;
	Node* Child;
	Node* Sibling;
	Node* Parent;//上层目录节点
	Node* RealParent;//数据结构中的父节点
};

/// <summary>
/// 表项如下(默认主键等省略)
/// +--------+---------+-------+-------+-------+---------+-------------+------+
/// |node_id |full_path|child  |sibling|parent |file_attr|modified_time|size  |
/// +--------+---------+-------+-------+-------+---------+-------------+------+
/// </summary>
class SqlScript {
public:
	SqlScript(TCHAR* TableName);
	~SqlScript();
	int OpenScriptFp(/*char* exePath, char* uName, char* pwd, char* host, char* port*/);//初始化脚本文件
	int AddNode(int NodeId,TCHAR* Path, DWORD Attr, UINT64 Time, INT64 Size, int Child, int Sibling, int Parent);//在文件中追加一行)
	int UpdateRelation(Node* p);//更新关系
	int CloseScriptFp();//释放文件
private:
	bool WritingActive = false;
	FILE* fp;
	std::hash<TCHAR*> hash;
	int AppendScript(TCHAR* buf);//在文件中追加脚本
	TCHAR TableName[MAX_PATHLEN];
	TCHAR FileName[MAX_PATHLEN];
	int CurrentWroteCount = 0;
	int CurrentFileCount = 0;
};

class DirectoryTree {
public:
	int MaxDepth = 0;
	int MaxRealDepth = 0;
	int DirCount = 0;
	int FileCount = 0;
	int IdAccumulator = 0;//分配NodeId
	int FolderStatVersionControl = 0;//用来记录文件夹统计数据的版本
	int FullDiffVersionControl = 0;//用来记录全量比对数据的版本
	int TotalAlterExecuted = 0;//记录总共执行的Alter操作次数
	TCHAR LongestFullPath[MAX_PATHLEN];
	Node* Root;
	std::vector<int> UpdatedNodes;
	DirectoryTree(TCHAR* RootPath);
	int GetDirectoryInfo(Node* p, Node** FileOldest, Node** FileNewest, int* FileAmount, INT64* TotalFileSize);
	int AlterNode(TCHAR* Path, TCHAR* Mode, INT64 LastModifiedTime, INT64 Size);
	Node* GetNodeByPath(TCHAR* NodePath);
	Node* GetNodeById(int NodeId);
	int DumpDirInfo(Node* p,TCHAR* txtFileName);
	SqlScript* Script;
private:
	int AddSibling(Node* base,Node* target);
	int AddChild(Node* base,Node* target);
	int DeleteNode(Node* p);
	std::map<int,Node*> NodeMap;
};

