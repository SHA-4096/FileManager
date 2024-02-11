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

Node::Node(TCHAR* PathName,DWORD FileAttribute,FILETIME CreatedTime,int Depth) {
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
	setlocale(LC_ALL, "");
	WIN32_FIND_DATA data;
	TCHAR currentPath[MAX_PATHLEN];//��������ǰĿ¼������
	TCHAR subDirPath[MAX_PATHLEN];//��������ȫ·��
	HANDLE hFind = FindFirstFile(RootPath,&data);
	std::queue<Node*> dirQueue;
	this->Root = new Node(RootPath, data.dwFileAttributes,data.ftCreationTime , 0);
	dirQueue.push(Root);
	int curDep = 0;
	Node* curNode = Root;//���ļ���Ҫɨ���Ŀ¼�ڵ�
	Node* appNode = nullptr;//Ҫ��ӵĽڵ�
	while (!dirQueue.empty()) {
		curNode = dirQueue.front();
		curDep = curNode->Depth;
		dirQueue.pop();
		wsprintf(currentPath, L"%ls%ls", curNode->PathName, L"\\*");
		hFind = FindFirstFile(currentPath, &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(_tcscmp(data.cFileName, L".") && _tcscmp(data.cFileName, L".."))) {
					//����.��..
					continue;
				}
				//�½��ڵ�
				wsprintf(subDirPath, L"%ls\\%ls", curNode->PathName, data.cFileName);
				appNode = new Node(subDirPath, data.dwFileAttributes, data.ftCreationTime, curDep + 1);
				//���ͳ��
				this->MaxDepth = (appNode->Depth > this->MaxDepth) ? appNode->Depth : this->MaxDepth;
				//����·��
				_tcscpy_s(this->LongestFullPath, (_tcslen(appNode->PathName) > _tcslen(this->LongestFullPath)) ?
					appNode->PathName : this->LongestFullPath);

				if (!(curNode->Child)) {
					//��Ӹ�ɨ����Ϊnow��child					
					auto err = this->AddChild(curNode,appNode);
					if (err) {
						printf_s("AddChild Error\n");
						exit(-1);
					}
				}else {
					//��Ӹ�ɨ����ΪcurNode->child��sibling
					this->AddSibling(curNode->Child,appNode);
				}
				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					//�����Ŀ¼���������
					dirQueue.push(appNode);
					//Ŀ¼��ͳ��
					this->DirCount++;
				}
				else {
					this->FileCount++;
				}

			} while (FindNextFile(hFind, &data));
			
		}else {
			wprintf(L"FindFirstFile failed at %ls\n", currentPath);
		}
	}

}


/// <summary>
/// ���Sibling, �ɹ�����0
/// </summary>
/// <param name="target"></param>
/// <returns></returns>
int DirectoryTree::AddSibling(Node* base,Node* target) {
	Node* p = base;
	while(p->Sibling){
		p = p->Sibling;
	}
	p->Sibling = target;
	target->RealDep = p->RealDep + 1;
	target->Parent = base->Parent;
	//����������
	this->MaxRealDepth = max(target->RealDep, MaxRealDepth);
	return 0;
}

/// <summary>
/// ���Child���ɹ�����0
/// </summary>
/// <param name="target"></param>
/// <returns></returns>
int DirectoryTree::AddChild(Node* base,Node* target) {
	Node* p = base;
	if (p->Child) {
		//һ���ڵ㲻���ж��Child
		return -1;
	}
	base->Child = target;
	target->Parent = base;
	target->RealDep = base->RealDep + 1;
	//����������
	this->MaxRealDepth = max(target->RealDep, MaxRealDepth);
	return 0;
}