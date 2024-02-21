#include"model.h"

#pragma region SqlScript


SqlScript::SqlScript() {
	fp = nullptr;
	fopen_s(&fp, "script.sql", "wb");
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
int SqlScript::InitScript(/*char* exePath, char* uName, char* pwd, char* host, char* port*/) {
	return 0;
}

/// <summary>
/// �����ڵ㣬ʱ���ʽ��֮�����ַ����洢
/// </summary>
/// <param name="path"></param>
/// <param name="attr"></param>
/// <param name="time"></param>
/// <param name="child"></param>
/// <param name="sibling"></param>
/// <param name="parent"></param>
/// <returns></returns>
int SqlScript::AddNode(TCHAR* path, DWORD attr, INT64 time, int child, int sibling, int parent)
{
	auto SysTime = new SYSTEMTIME();
	//FileTimeToSystemTime(&time, SysTime);
	TCHAR buf[MAX_PATHLEN*2];
	//auto formattedTime = new TCHAR[20];
	//_stprintf_s(formattedTime, 20, _T("%d-%d-%d"), SysTime->wYear, SysTime->wMonth, SysTime->wDay);
	_stprintf_s(buf, MAX_PATHLEN*2, _T("INSERT INTO `file` (`path`, `attr`, `time`, `child`, `sibling`, `parent`) VALUES ('%ls', %d, %lld, %d, %d, %d);\n"), 
		path, attr, time, child, sibling, parent);
	this->AppendScript(buf);
	return 0;
}

/// <summary>
/// ����p�ڵ��Parent,Sibling�Լ�Child
/// </summary>
/// <param name="NodeId"></param>
/// <param name="Child"></param>
/// <param name="Sibling"></param>
/// <param name="Parent"></param>
/// <returns></returns>
int SqlScript::UpdateRelation(Node* p) {
	TCHAR buf[MAX_PATHLEN * 2];
	_stprintf_s(buf, MAX_PATHLEN * 2, _T("UPDATE `file` SET `child` = %d, `sibling` = %d, `parent` = %d WHERE `node_id` = %d;\n"),
		CHILD_ID(p), SIBLING_ID(p), PARENT_ID(p), SELF_ID(p));
	AppendScript(buf);
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
#pragma endregion

#pragma region Node

Node::Node(WIN32_FIND_DATA* Data, int Depth, int NodeId, TCHAR* PathName) {
	this->FileAttribute = Data->dwFileAttributes;
	this->ModifiedTime = Data->ftLastWriteTime.dwLowDateTime + Data->ftLastWriteTime.dwHighDateTime * (MAXDWORD + 1);
	this->FileSize = (Data->nFileSizeHigh * (MAXDWORD + 1)) + Data->nFileSizeLow;
	//this->FileSize /= (1024 * 1024);//��MB��
	_tcscpy_s(this->PathName, MAX_PATHLEN, PathName);
	this->NodeId = NodeId;
	this->Depth = Depth;
	this->Child = NULL;
	this->Sibling = NULL;
}
#pragma endregion

#pragma region DirectoryTree
/// <summary>
/// ����Ŀ¼��
/// </summary>
/// <param name="RootName"></param>
DirectoryTree::DirectoryTree(TCHAR* RootPath) {
	this->Script = new SqlScript();
	setlocale(LC_ALL, "");
	WIN32_FIND_DATA data;
	TCHAR currentPath[MAX_PATHLEN];//��������ǰĿ¼������
	TCHAR subDirPath[MAX_PATHLEN];//��������ȫ·��
	HANDLE hFind = FindFirstFile(RootPath, &data);
	std::queue<Node*> dirQueue;
	this->Root = new Node(&data,0, this->IdAccumulator++, RootPath);
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
				appNode = new Node(&data, curDep + 1, this->IdAccumulator++, subDirPath);
				//���ͳ��
				this->MaxDepth = (appNode->Depth > this->MaxDepth) ? appNode->Depth : this->MaxDepth;
				//����·��
				_tcscpy_s(this->LongestFullPath, (_tcslen(appNode->PathName) > _tcslen(this->LongestFullPath)) ?
					appNode->PathName : this->LongestFullPath);

				if (!(curNode->Child)) {
					//��Ӹ�ɨ����Ϊnow��child					
					auto err = this->AddChild(curNode, appNode);
					if (err) {
						printf_s("AddChild Error\n");
						exit(-1);
					}
				}
				else {
					//��Ӹ�ɨ����ΪcurNode->child��sibling
					this->AddSibling(curNode->Child, appNode);
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

		}
		else {
			wprintf(L"FindFirstFile failed at %ls\n", currentPath);
		}
	}

}

/// <summary>
/// 
/// </summary>
/// <param name="p">[in]</param>
/// <param name="FileOldest">[out]</param>
/// <param name="FileNewest">[out]</param>
/// <param name="FileAmount">[out]</param>
/// <param name="TotalFileSize">[out]</param>
/// <returns></returns>
int DirectoryTree::GetDirectoryInfo(Node* p,Node** FileOldest,Node** FileNewest,int* FileAmount,INT64* TotalFileSize) {
	if (p->FileAttribute & FILE_ATTRIBUTE_DIRECTORY) {
		//��Ŀ¼
		Node* now = p->Child;
		Node* newest = NULL;
		Node* oldest = NULL;
		int fileCount = 0;
		INT64 totalFileSize = 0;
		while (now) {
			if (now->FileAttribute & FILE_ATTRIBUTE_DIRECTORY) {
				//��Ŀ¼�Ļ�������
				continue;
			}
			//�ҵ������������ļ��ڵ�
			if (!oldest || !newest) {
				//���Ŀ¼�´����ļ����������ʼ��Newest��Oldest
				oldest = now;
				newest = now;
			}
			oldest = (oldest->ModifiedTime < now->ModifiedTime) ? oldest:now;
			newest = (newest->ModifiedTime > now->ModifiedTime) ? newest : now;
			//ͳ���ļ��������ļ��ܴ�С
			fileCount++;
			totalFileSize += now->FileSize;
			now = now->Sibling;
		}
		//���ؽ��
		*FileAmount = fileCount;
		*FileNewest = newest;
		*FileOldest = oldest;
		*TotalFileSize = totalFileSize;
		return 0;
	}
	else {
		//����Ŀ¼
		return -1;
	}
}

/// <summary>
/// ���ļ��ڵ������ɾ�Ĳ������ɹ�����0
/// </summary>
/// <param name="p"></param>
/// <param name="Mode"></param>
/// <param name="LastModifiedTime"></param>
/// <param name="Size"></param>
/// <returns></returns>
int DirectoryTree::AlterFileNode(TCHAR* Path, TCHAR* Mode, INT64 LastModifiedTime, INT64 Size)
{
	TCHAR* Modes[3] = { L"A",L"M",L"D"};//[A]dd,[D]elete,[M]odify
	if (!_tcscmp(Mode, Modes[0])) {
		//�����ļ��ڵ�
		return 0;

	}else if (!_tcscmp(Mode, Modes[1])) {
		//�ȼ���ļ��ڵ��Ƿ����
		Node* target = this->GetNodeByPath(Path);
		if (!target) {
			//�ļ��ڵ㲻����
			return -1;
		}
		//�޸��ļ��ڵ�
		target->FileSize = Size;
		target->ModifiedTime = LastModifiedTime;
		return 0;
	}
	else if (!_tcscmp(Mode, Modes[2])){
		//ɾ���ļ��ڵ�
		Node* target = this->GetNodeByPath(Path);
		if (!target) {
			//�ļ��ڵ㲻����
			return -1;
		}
		this->DeleteFileNode(target);
		return 0;
	}
	else {
		return -1;
	}
}

/// <summary>
/// ͨ��path��ȡָ���Ľڵ㣬ʧ�ܷ���NULL
/// </summary>
/// <param name="NodePath"></param>
/// <returns></returns>
Node* DirectoryTree::GetNodeByPath(TCHAR* NodePath) {
	Node* ret = NULL;
	Node* now = this->Root;

	while (now) {
		while (wcsstr(NodePath, now->PathName)) {
			//��������
			Node* p = now->Child;
			bool nowUpdateFlag = false;
			while(p){
				//���now��ӦĿ¼��һ��Ŀ¼
				if(wcsstr(NodePath, p->PathName)) {
					now = p;
					nowUpdateFlag = true;
					break;
				}
				p = p->Sibling;
			}
			if (!nowUpdateFlag) {
				//nowû�б����£�˵��������now����Ҫ�ҵĽڵ㣬���ļ�����û�ж�ӦĿ¼
				break;
			}
		}
		if (!_tcscmp(NodePath, now->PathName)) {
			//�����ɹ�
			ret = now;
			break;
		}
		//����ʧ�ܣ�����nowͬ��������Ŀ¼
		now = now->Sibling;
	}
	return ret;
}

/// <summary>
/// ���Sibling, �ɹ�����0
/// </summary>
/// <param name="target"></param>
/// <returns></returns>
int DirectoryTree::AddSibling(Node* base, Node* target) {
	Node* p = base;
	while (p->Sibling) {
		p = p->Sibling;
	}
	p->Sibling = target;
	target->RealDep = p->RealDep + 1;
	target->Parent = base->Parent;
	target->RealParent = p;
	//����������
	this->MaxRealDepth = max(target->RealDep, MaxRealDepth);
	//Sql��ز���
	this->Script->AddNode(target->PathName, target->FileAttribute, target->ModifiedTime, 0, 0, target->Parent->NodeId);
	this->Script->UpdateRelation(p);
	return 0;
}

/// <summary>
/// ���Child���ɹ�����0
/// </summary>
/// <param name="target"></param>
/// <returns></returns>
int DirectoryTree::AddChild(Node* base, Node* target) {
	Node* p = base;
	if (p->Child) {
		//һ���ڵ㲻���ж��Child
		return -1;
	}
	base->Child = target;
	target->Parent = base;
	target->RealDep = base->RealDep + 1;
	target->RealParent = base;
	//����������
	this->MaxRealDepth = max(target->RealDep, MaxRealDepth);
	//Sql��ز���
	this->Script->AddNode(target->PathName, target->FileAttribute, target->ModifiedTime, 0, 0, target->Parent->NodeId);
	this->Script->UpdateRelation(p);
	return 0;
}
/// <summary>
/// ɾ��ָ���Ľڵ㣬����ɹ�����0
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
int DirectoryTree::DeleteFileNode(Node* p)
{
	if(p->FileAttribute & FILE_ATTRIBUTE_DIRECTORY){
		//��Ŀ¼
		return -1;
	}
	else {
		//���ļ�
		//ɾ���ļ��ڵ�
		//ע���ļ��ڵ�ض�û��Child�ڵ㣬ֻ��Ҫά��Sibling�ڵ�ͺ���
		//������Parent������֪Parent����
		if (!(p->RealParent)) {
			//���ڵ㣬ֱ��ɾ��
			delete p;
		}
		else {
			if (p->Parent == p->RealParent) {
				//RealParent���ϲ�Ŀ¼
				p->RealParent->Child = p->Sibling;
				p->Sibling->RealParent = p->RealParent;
			}
			else {
				p->RealParent->Sibling = p->Sibling;
				p->Sibling->RealParent = p->RealParent;
			}
			delete p;
		}
	}
	return 0;
}
#pragma endregion

