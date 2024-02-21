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

#define Unit_KB 0
#define Unit_MB 1
#define Unit_GB 2
#define Unit_TB 3

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
	TCHAR PathName[MAX_PATHLEN];
	DWORD FileAttribute;
	FILETIME CreatedTime;
	int Depth;//�ļ������
	int RealDep;//���ݽṹ��ʵ���
	int NodeId;
	INT64 FileSize;
	Node* Child;
	Node* Sibling;
	Node* Parent;
};

/// <summary>
/// ��������(Ĭ��������ʡ��)
/// +--------+--------+-------+-------+-------+--------+----------+
/// |NodeId	 |FullPath|Child  |Sibling|Parent |FileAttr|CreateTime|
/// +--------+--------+-------+-------+-------+--------+----------+
/// </summary>
class SqlScript {
public:
	SqlScript();
	~SqlScript();
	size_t CalcHash(TCHAR* path);//��path���ڴ��еĵ�ַ��Ϊ��������hashֵ
	int InitScript(/*char* exePath, char* uName, char* pwd, char* host, char* port*/);//��ʼ���ű��ļ�
	int AddNode(TCHAR* path, DWORD attr, FILETIME time, int child, int sibling, int parent);//���ļ���׷��һ��)
	int UpdateRelation(Node* p);//���¹�ϵ
private:
	FILE* fp;
	std::hash<TCHAR*> hash;
	int AppendScript(TCHAR* buf);//���ļ���׷�ӽű�
};

class DirectoryTree {
public:
	int MaxDepth = 0;
	int MaxRealDepth = 0;
	int DirCount = 0;
	int FileCount = 0;
	int IdAccumulator = 0;//����NodeId
	TCHAR LongestFullPath[MAX_PATHLEN];
	Node* Root;
	DirectoryTree(TCHAR* RootPath);
	int GetDirectoryInfo(Node* p, Node* FileOldest, Node* FileNewest, int* FileAmount, INT64* TotalFileSize);
	Node* GetNodeByPath(TCHAR* NodePath);
private:
	int AddSibling(Node* base,Node* target);
	int AddChild(Node* base,Node* target);
	SqlScript* Script;
};

