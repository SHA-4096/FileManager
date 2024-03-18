#include"model.h"

#pragma region SqlScript

/// <summary>
/// ��ʼ�����ݿ������䣬�����ͬ����Ļ����Ƚ���ɾ��
/// </summary>
/// <param name="TableName"></param>
SqlScript::SqlScript(TCHAR* TableName) {
	//fp = nullptr;
	//fopen_s(&fp, "script.sql", "wb");
	TCHAR buf[MAX_PATHLEN*10];
	swprintf_s(buf, MAX_PATHLEN * 10,_T("SET sql_mode=\'NO_BACKSLASH_ESCAPES\';\n"));//ȡ��ת��
	swprintf_s(buf, MAX_PATHLEN*10,_T("%lsDROP TABLE IF EXISTS `%ls`;\n"), buf,TableName);
	swprintf_s(buf, MAX_PATHLEN * 10, _T("%ls"
		"CREATE TABLE %ls ("
		"node_id INT,"
		"full_path TEXT,"
		"child INT,"
		"sibling INT,"
		"parent INT,"
		"file_attr INT,"
		"modified_time BIGINT,"
		"size BIGINT"
		");\n"),
		buf,TableName
	);
	this->OpenScriptFp();
	this->AppendScript(buf);
	_tcscpy_s(this->TableName, TableName);
}

SqlScript::~SqlScript() {
	
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
int SqlScript::OpenScriptFp() {
	this->CurrentWroteCount++;
	_stprintf_s(this->FileName, MAX_PATHLEN, _T("script_%d.sql"), this->CurrentFileCount++);
	fp ? fclose(fp) : 0;
	_tfopen_s(&fp, this->FileName, _T("a,ccs=UTF-8"));
	this->WritingActive = true;
	return 0;
}

/// <summary>
/// �����ڵ㣬ʱ����INT64�洢
/// </summary>
/// <param name="path"></param>
/// <param name="attr"></param>
/// <param name="time"></param>
/// <param name="child"></param>
/// <param name="sibling"></param>
/// <param name="parent"></param>
/// <returns></returns>
int SqlScript::AddNode(int NodeId,TCHAR* Path, DWORD Attr, UINT64 Time, INT64 Size,int Child, int Sibling, int Parent)
{
	auto SysTime = new SYSTEMTIME();
	//FileTimeToSystemTime(&time, SysTime);
	TCHAR buf[MAX_PATHLEN*2];
	//auto formattedTime = new TCHAR[20];
	//_stprintf_s(formattedTime, 20, _T("%d-%d-%d"), SysTime->wYear, SysTime->wMonth, SysTime->wDay);
	_stprintf_s(buf, MAX_PATHLEN*2, _T("INSERT INTO %ls (`node_id`,`full_path`, `file_attr`, `modified_time`, `child`, `sibling`, `parent` ,`size` ) VALUES (%d ,'%ls', %d, %llu, %d, %d, %d, %lld);\n"), 
		this->TableName,NodeId,Path, Attr, Time, Child, Sibling, Parent,Size);
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
	_stprintf_s(buf, MAX_PATHLEN * 2, _T("UPDATE %ls SET `child` = %d, `sibling` = %d, `parent` = %d WHERE `node_id` = %d;\n"),
		this->TableName,CHILD_ID(p), SIBLING_ID(p), PARENT_ID(p), SELF_ID(p));
	AppendScript(buf);
	return 0;
}

int SqlScript::CloseScriptFp()
{
	this->WritingActive = false;
	fclose(fp);
	return 0;
}

/// <summary>
/// ���ļ���д��buf���ɹ�����0;ʵ���ļ��ֿ��߼�
/// </summary>
/// <param name="buf"></param>
/// <returns></returns>
int SqlScript::AppendScript(TCHAR* buf) {
	if(!this->WritingActive) return -1;
	//ÿMAX_SQL_LINES��д��ͻ�һ���ļ�
	if (!this->CurrentWroteCount) {
		_stprintf_s(this->FileName, MAX_PATHLEN, _T("script_%d.sql"), this->CurrentFileCount++);
		fp ? fclose(fp) : 0;
		_tfopen_s(&fp, this->FileName, _T("a,ccs=UTF-8"));
	}
	fwrite(buf, sizeof(TCHAR), _tcslen(buf), fp);
	if (this->CurrentWroteCount++ > MAX_SQL_LINES) {
		this->CurrentWroteCount = 0;
	}
	return 0;
}
#pragma endregion

#pragma region Node

Node::Node(WIN32_FIND_DATA* Data, int Depth, int NodeId, TCHAR* PathName) {
	this->FileAttribute = Data->dwFileAttributes;
	//��ʱ��ת��Ϊ����
	ULARGE_INTEGER uli;//Ϊ�˱��������󣬲���ֱ�ӽ��м�������ModifiedTime
	uli.LowPart = Data->ftLastWriteTime.dwLowDateTime;
	uli.HighPart = Data->ftLastWriteTime.dwHighDateTime;
	this->ModifiedTime = uli.QuadPart;
	this->ModifiedTime /= 10000000;//ת��Ϊ��(FILETIME�ĵ�λ��100ns)
	//ת��ΪUNIXʱ��
	this->ModifiedTime -= WINDOWS_UNIX_TIME_DIFF;

	this->FileSize = (Data->nFileSizeHigh * (MAXDWORD + 1)) + Data->nFileSizeLow;
	//this->FileSize /= (1024 * 1024);//��MB��
	_tcscpy_s(this->PathName, MAX_PATHLEN, PathName);
	this->NodeId = NodeId;
	this->Depth = Depth;
	this->Child = NULL;
	this->Sibling = NULL;
}

Node::Node(INT64 Size, UINT64 ModifiedTime, DWORD FileAttribute,int Depth, int NodeId, TCHAR* PathName)
{
	this->FileAttribute = FileAttribute;
	this->ModifiedTime = ModifiedTime;
	this->FileSize = Size;
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
	this->Script = new SqlScript(_T("Nodes"));
	setlocale(LC_ALL, "");
	WIN32_FIND_DATA data;
	TCHAR currentPath[MAX_PATHLEN];//��������ǰĿ¼������
	TCHAR subDirPath[MAX_PATHLEN];//��������ȫ·��
	HANDLE hFind = FindFirstFile(RootPath, &data);
	std::queue<Node*> dirQueue;
	this->Root = new Node(&data,0, this->IdAccumulator++, RootPath);
	this->Script->AddNode(this->Root->NodeId,this->Root->PathName, this->Root->FileAttribute, this->Root->ModifiedTime, this->Root->FileSize, 0, 0, 0);
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
				if (!(lowerCaseCmp(data.cFileName, L".") && lowerCaseCmp(data.cFileName, L".."))) {
					//����.��..
					continue;
				}
				//�½��ڵ�
				wsprintf(subDirPath, L"%ls\\%ls", curNode->PathName, data.cFileName);
				//�½��ڵ�
				appNode = new Node(&data, curDep + 1, this->IdAccumulator++, subDirPath);
				//��¼NodeId��Node�����ӳ���ϵ
				NodeMap[appNode->NodeId] = appNode;
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
	//ɨ����ɣ��ͷ��ļ�ָ��
	this->Script->CloseScriptFp();

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
		//֮ǰû�м�¼�������¼��ǰĿ¼�µ��ļ���Ϣ
		FILE* tmp = NULL;
		TCHAR* txtOldFileName = new TCHAR[MAX_PATHLEN];
		int folderNodeId = p->NodeId;

		Node* now = p->Child;
		Node* newest = NULL;
		Node* oldest = NULL;
		int fileCount = 0;
		INT64 totalFileSize = 0;
		while (now) {
			if (now->FileAttribute & FILE_ATTRIBUTE_DIRECTORY) {
				//��Ŀ¼�Ļ�������
				now = now->Sibling;
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
/// ���ļ��ڵ������ɾ�Ĳ������ɹ�����0��ͬʱ��������ݿ������
/// </summary>
/// <param name="p"></param>
/// <param name="Mode"></param>
/// <param name="LastModifiedTime"></param>
/// <param name="Size"></param>
/// <returns></returns>
int DirectoryTree::AlterNode(TCHAR* Path, TCHAR* Mode, INT64 LastModifiedTime, INT64 Size)
{
	this->TotalAlterExecuted ++;
	TCHAR* Modes[3] = { L"A",L"M",L"D"};//[A]dd,[D]elete,[M]odify
	if (!lowerCaseCmp(Mode, Modes[0])) {
		//�����ļ��ڵ�
		//���Ƚ��ļ�������Trim��
		TCHAR folderPath[MAX_PATHLEN];
		_tcscpy_s(folderPath, Path);
		int tail = _tcslen(folderPath) - 1;
		while (folderPath[tail] != L'\\') {
			folderPath[tail] = L'\0';
			tail--;
		}
		folderPath[tail] = L'\0';
		//����ļ����Ƿ����
		Node* target = this->GetNodeByPath(folderPath);
		if (!target) {
			return -1;
		}
		//����ļ��ڵ��Ƿ����
		Node* fileTarget = this->GetNodeByPath(Path);
		if (fileTarget) {
			return -1;
		}
		//�����ļ��ڵ�
		Node* newNode = new Node(Size, LastModifiedTime, (DWORD(FILE_ATTRIBUTE_NORMAL)), target->Depth + 1, this->IdAccumulator++, Path);
		if (target->Child) {
			target = target->Child;
			AddSibling(target, newNode);
		}
		else {
			AddChild(target, newNode);
		}
		return 0;

	}else if (!lowerCaseCmp(Mode, Modes[1])) {
		//�޸��ļ��ڵ�
		//�ȼ���ļ��ڵ��Ƿ����
		Node* target = this->GetNodeByPath(Path);
		if (!target) {
			//�ļ��ڵ㲻����
			return -1;
		}
		//�޸�
		target->FileSize = Size;
		target->ModifiedTime = LastModifiedTime;
		return 0;
	}
	else if (!lowerCaseCmp(Mode, Modes[2])){
		//ɾ���ļ����ļ��нڵ�
		Node* target = this->GetNodeByPath(Path);
		if (!target) {
			//�ڵ㲻����
			return -1;
		}
		this->DeleteNode(target);
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
		while (lowerCaseContain(NodePath, now->PathName)) {
			//��������
			Node* p = now->Child;
			bool nowUpdateFlag = false;
			while(p){
				//���now��ӦĿ¼��һ��Ŀ¼
				if(lowerCaseContain(NodePath, p->PathName)) {
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
		if (!lowerCaseCmp(NodePath, now->PathName)) {
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
	this->Script->AddNode(target->NodeId,target->PathName, target->FileAttribute,target->ModifiedTime,target->FileSize, 0, 0, target->Parent->NodeId);
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
	this->Script->AddNode(target->NodeId,target->PathName, target->FileAttribute, target->ModifiedTime,target->FileSize, 0, 0, target->Parent->NodeId);
	this->Script->UpdateRelation(p);
	return 0;
}
/// <summary>
/// ɾ��ָ���Ľڵ㣬����ɹ�����0
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
int DirectoryTree::DeleteNode(Node* p)
{
	if (!p) {
		return -1;
	}
	Node* isoTreeRoot = p->Child;//ɾ���ڵ����ܻ���һ�ñ�������������
	if (!(p->RealParent)) {
		//���ڵ㣬ֱ��ɾ��
		this->NodeMap.erase(p->NodeId);
		delete p;
	}
	else {
		if (p->Parent == p->RealParent) {
			//RealParent���ϲ�Ŀ¼
			p->RealParent->Child = p->Sibling;
			if(p->Sibling) p->Sibling->RealParent = p->RealParent;
		}
		else {
			p->RealParent->Sibling = p->Sibling;
			if(p->Sibling) p->Sibling->RealParent = p->RealParent;
		}
		//ͳ������
		p->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ?
			this->DirCount-- : this->FileCount--;
		this->NodeMap.erase(p->NodeId);
		//�ͷ��ڴ�
		delete p;
	}
	if (isoTreeRoot) {
		//ɾ��������������Ч�Ľڵ���ɵ���,isoTreeRootΪ��
		std::queue<Node*> nodesToDelete;
		nodesToDelete.push(isoTreeRoot);
		while (nodesToDelete.size()) {
			//��α�����ɾ���ڵ�
			Node* curNode = nodesToDelete.front();
			nodesToDelete.pop();
			if (curNode->Child)  nodesToDelete.push(curNode->Child);
			if (curNode->Sibling) nodesToDelete.push(curNode->Sibling);
			//ͳ������
			curNode->FileAttribute& FILE_ATTRIBUTE_DIRECTORY ?
				this->DirCount-- : this->FileCount--;
			this->NodeMap.erase(curNode->NodeId);
			//�ͷ��ڴ�
			delete curNode;

		}
	}
	return 0;
}


#pragma endregion

/// <summary>
/// ���ڵ�p��Ŀ¼��Ϣ��ʽ�����Ϊtxt�ļ�
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
int DirectoryTree::DumpDirInfo(Node* p,TCHAR* txtFileName)
{
	TCHAR buf[MAX_PATHLEN * 10];
	//�����Ըýڵ�Ϊ����Ŀ¼��
	std::queue<Node*> nodesToDump;
	Node* tmpRoot = p;
	nodesToDump.push(p);
	FILE* txtFp;
	_tfopen_s(&txtFp, txtFileName, L"w,ccs=UTF-8");
	while (nodesToDump.size()) {
		Node* now = nodesToDump.front();
		nodesToDump.pop();
		TCHAR formattedTime[100];
		unixTimeToFormattedString(now->ModifiedTime, formattedTime);
		//���
		_stprintf_s(buf, MAX_PATHLEN * 10,
			_T("##����-%ls ·���� %ls \n��С:%lld �޸�ʱ��:%ls\n"),
			now->FileAttribute&FILE_ATTRIBUTE_DIRECTORY ? _T("Ŀ¼"):_T("�ļ�"),
			now->PathName,now->FileSize,formattedTime);
		fwrite(buf, sizeof(TCHAR), _tcslen(buf), txtFp);
		//Ϊ���ܹ������Ը��ӶȶԱ��ļ���Ҫ��ͳ���굱��Ŀ¼��ͳ����һ��
		if (now->Sibling && now != p) {//Ҫ���⽫ͬ������Ŀ¼ͳ�ƽ�ȥ
			nodesToDump.push(now->Sibling);
		}
		if (now->Child) {
			nodesToDump.push(now->Child);
		}
	}
	txtFp ? fclose(txtFp):0;
	return 0;
}

/// <summary>
/// ��NodeID��ȡ��Ӧ��Node����ʧ�ܷ���NULL
/// </summary>
/// <param name="NodeId"></param>
/// <returns></returns>
Node* DirectoryTree::GetNodeById(int NodeId)
{
	auto it = NodeMap.find(NodeId);
	if (it != NodeMap.end()) {
		return it->second;
	}
	return NULL;
}