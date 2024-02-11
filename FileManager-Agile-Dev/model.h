#pragma once
#include<stdio.h>
#include<tchar.h>
#include<windows.h>
#include<locale.h>
#include<queue>
#define MAX_PATHLEN 1000

class SqlScript {
public:
	SqlScript();
	~SqlScript();
	int InitScript(/*char* exePath, char* uName, char* pwd, char* host, char* port*/);//初始化脚本文件
	int AppendScript(TCHAR* buf);//在文件中追加脚本
private:
	FILE* fp;
};

class Node {
public:
	Node(TCHAR*, DWORD, FILETIME, int);
	TCHAR PathName[MAX_PATHLEN];
	DWORD FileAttribute;
	FILETIME CreatedTime;
	int Depth;//文件树深度
	int RealDep;//数据结构真实深度
	Node* Child;
	Node* Sibling;
	Node* Parent;
};

class DirectoryTree {
public:
	int MaxDepth = 0;
	int MaxRealDepth = 0;
	int DirCount;
	int FileCount;
	TCHAR LongestFullPath[MAX_PATHLEN];
	Node* Root;
	DirectoryTree(TCHAR* RootName);
private:
	int AddSibling(Node* base,Node* target);
	int AddChild(Node* base,Node* target);
};