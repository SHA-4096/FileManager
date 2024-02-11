#pragma once
#include<stdio.h>
#include<tchar.h>
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
	Node(TCHAR*, int, int, int);
	TCHAR PathName[MAX_PATHLEN];
	int FileAttribute;
	int CreatedTime;
	int Depth;
	Node* Child;
	Node* Sibling;
	Node* Parent;
};

class DirectoryTree {
public:
	int MaxDepth;
	int NodeCount;
	Node* Root;
	Node* now;
	DirectoryTree(TCHAR* RootName);
	int AddSibling(Node* target);
	int AddChild(Node* target);
};