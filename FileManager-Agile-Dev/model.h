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
	int InitScript(/*char* exePath, char* uName, char* pwd, char* host, char* port*/);//��ʼ���ű��ļ�
	int AppendScript(TCHAR* buf);//���ļ���׷�ӽű�
private:
	FILE* fp;
};

class Node {
public:
	Node(TCHAR*, DWORD, FILETIME, int);
	TCHAR PathName[MAX_PATHLEN];
	DWORD FileAttribute;
	FILETIME CreatedTime;
	int Depth;//�ļ������
	int RealDep;//���ݽṹ��ʵ���
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