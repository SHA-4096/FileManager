#pragma once
#include<locale.h>
#include<queue>
#include<stdio.h>
#include<tchar.h>
#include<windows.h>
#include<uuids.h>
#define MAX_PATHLEN 1000

#define CHILD_ID(x) x->Child ? x->Child->NodeId:0
#define PARENT_ID(x) x->Parent ? x->Parent->NodeId:0
#define SIBLING_ID(x) x->Sibling ? x->Sibling->NodeId:0
#define SELF_ID(x) x ? x->NodeId:0




class Node {
public:
	Node(TCHAR*, DWORD, FILETIME, int Depth,int NodeId);
	TCHAR PathName[MAX_PATHLEN];
	DWORD FileAttribute;
	FILETIME CreatedTime;
	int Depth;//文件树深度
	int RealDep;//数据结构真实深度
	int NodeId;
	Node* Child;
	Node* Sibling;
	Node* Parent;
};

/// <summary>
/// 表项如下(默认主键等省略)
/// +--------+--------+-------+-------+-------+--------+----------+
/// |NodeId	 |FullPath|Child  |Sibling|Parent |FileAttr|CreateTime|
/// +--------+--------+-------+-------+-------+--------+----------+
/// </summary>
class SqlScript {
public:
	SqlScript();
	~SqlScript();
	size_t CalcHash(TCHAR* path);//用path在内存中的地址作为参数计算hash值
	int InitScript(/*char* exePath, char* uName, char* pwd, char* host, char* port*/);//初始化脚本文件
	int AddNode(TCHAR* path, DWORD attr, FILETIME time, int child, int sibling, int parent);//在文件中追加一行)
	int UpdateRelation(Node* p);//更新关系
private:
	FILE* fp;
	std::hash<TCHAR*> hash;
	int AppendScript(TCHAR* buf);//在文件中追加脚本
};

class DirectoryTree {
public:
	int MaxDepth = 0;
	int MaxRealDepth = 0;
	int DirCount;
	int FileCount;
	int IdAccumulator = 0;//分配NodeId
	TCHAR LongestFullPath[MAX_PATHLEN];
	Node* Root;
	DirectoryTree(TCHAR* RootPath);
	int GetDirectoryInfo(Node* p);
	Node* GetNodeByPath(TCHAR* NodePath);
private:
	int AddSibling(Node* base,Node* target);
	int AddChild(Node* base,Node* target);
	SqlScript* Script;
};

