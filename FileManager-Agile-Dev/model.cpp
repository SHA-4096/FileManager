#include"model.h"



SqlScript::SqlScript(){
	fp = nullptr;
	fopen_s(&fp,"script.sql","wb");
}

SqlScript::~SqlScript() {
	fclose(fp);
}

/// <summary>
/// ��ʼ���ű��ļ����ɹ�����0
/// </summary>
/// <param name="exePath"></param>
/// <param name="uName"></param>
/// <param name="pwd"></param>
/// <param name="host"></param>
/// <param name="port"></param>
/// <returns></returns>
int SqlScript::InitScript(/*char* exePath, char* uName, char* pwd, char* host, char* port*/	) {
	return 0;
}

/// <summary>
/// ���ļ���д��buf���ɹ�����0
/// </summary>
/// <param name="buf"></param>
/// <returns></returns>
int SqlScript::AppendScript(TCHAR* buf) {
	fwrite(buf, sizeof(TCHAR), _tcslen(buf), fp);
	return 0;
}

Node::Node(TCHAR* PathName,int FileAttribute,int CreatedTime,int Depth) {
	_tcscpy_s(this->PathName,MAX_PATHLEN ,PathName);
	this->FileAttribute = FileAttribute;
	this->CreatedTime = CreatedTime;
	this->Depth = Depth;
	this->Child = nullptr;
	this->Sibling = nullptr;
}

/// <summary>
/// ����Ŀ¼��
/// </summary>
/// <param name="RootName"></param>
DirectoryTree::DirectoryTree(TCHAR* RootPath) {
	this->Root = new Node(RootPath, 0, 0, 0);
}


/// <summary>
/// ���Sibling, �ɹ�����0
/// </summary>
/// <param name="target"></param>
/// <returns></returns>
int DirectoryTree::AddSibling(Node* target) {
	Node* p = this->now;
	while(now->Sibling !=nullptr){
		now = now->Sibling;
	}
	now->Sibling = target;
	target->Parent = now;
	return 0;
}

/// <summary>
/// ���Child���ɹ�����0
/// </summary>
/// <param name="target"></param>
/// <returns></returns>
int DirectoryTree::AddChild(Node* target) {
	Node* p = this->now;
	while (now->Child != nullptr) {
		now = now->Child;
	}
	now->Child = target;
	target->Parent = now;
	return 0;
}