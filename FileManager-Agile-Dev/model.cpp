#include"model.h"

#pragma region SqlScript

/// <summary>
/// 初始化数据库表创建语句，如果有同名表的话则先将其删除
/// </summary>
/// <param name="TableName"></param>
SqlScript::SqlScript(TCHAR* TableName) {
	//fp = nullptr;
	//fopen_s(&fp, "script.sql", "wb");
	TCHAR buf[MAX_PATHLEN*10];
	swprintf_s(buf, MAX_PATHLEN * 10,_T("SET sql_mode=\'NO_BACKSLASH_ESCAPES\';\n"));//取消转义
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
/// 初始化脚本文件，成功返回0
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
/// 新增节点，时间用INT64存储
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
/// 更新p节点的Parent,Sibling以及Child
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
/// 向文件中写入buf，成功返回0;实现文件分块逻辑
/// </summary>
/// <param name="buf"></param>
/// <returns></returns>
int SqlScript::AppendScript(TCHAR* buf) {
	if(!this->WritingActive) return -1;
	//每MAX_SQL_LINES次写入就换一个文件
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
	//将时间转换为秒数
	ULARGE_INTEGER uli;//为了避免对齐错误，不能直接进行简单运算获得ModifiedTime
	uli.LowPart = Data->ftLastWriteTime.dwLowDateTime;
	uli.HighPart = Data->ftLastWriteTime.dwHighDateTime;
	this->ModifiedTime = uli.QuadPart;
	this->ModifiedTime /= 10000000;//转换为秒(FILETIME的单位是100ns)
	//转换为UNIX时间
	this->ModifiedTime -= WINDOWS_UNIX_TIME_DIFF;

	this->FileSize = (Data->nFileSizeHigh * (MAXDWORD + 1)) + Data->nFileSizeLow;
	//this->FileSize /= (1024 * 1024);//以MB计
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
/// 构建目录树
/// </summary>
/// <param name="RootName"></param>
DirectoryTree::DirectoryTree(TCHAR* RootPath) {
	this->Script = new SqlScript(_T("Nodes"));
	setlocale(LC_ALL, "");
	WIN32_FIND_DATA data;
	TCHAR currentPath[MAX_PATHLEN];//用来做当前目录的搜索
	TCHAR subDirPath[MAX_PATHLEN];//用来保存全路径
	HANDLE hFind = FindFirstFile(RootPath, &data);
	std::queue<Node*> dirQueue;
	this->Root = new Node(&data,0, this->IdAccumulator++, RootPath);
	this->Script->AddNode(this->Root->NodeId,this->Root->PathName, this->Root->FileAttribute, this->Root->ModifiedTime, this->Root->FileSize, 0, 0, 0);
	dirQueue.push(Root);
	int curDep = 0;
	Node* curNode = Root;//根文件或要扫描的目录节点
	Node* appNode = nullptr;//要添加的节点
	while (!dirQueue.empty()) {
		curNode = dirQueue.front();
		curDep = curNode->Depth;
		dirQueue.pop();
		wsprintf(currentPath, L"%ls%ls", curNode->PathName, L"\\*");
		hFind = FindFirstFile(currentPath, &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(lowerCaseCmp(data.cFileName, L".") && lowerCaseCmp(data.cFileName, L".."))) {
					//跳过.和..
					continue;
				}
				//新建节点
				wsprintf(subDirPath, L"%ls\\%ls", curNode->PathName, data.cFileName);
				//新建节点
				appNode = new Node(&data, curDep + 1, this->IdAccumulator++, subDirPath);
				//记录NodeId和Node对象的映射关系
				NodeMap[appNode->NodeId] = appNode;
				//深度统计
				this->MaxDepth = (appNode->Depth > this->MaxDepth) ? appNode->Depth : this->MaxDepth;
				//检查最长路径
				_tcscpy_s(this->LongestFullPath, (_tcslen(appNode->PathName) > _tcslen(this->LongestFullPath)) ?
					appNode->PathName : this->LongestFullPath);

				if (!(curNode->Child)) {
					//添加该扫描结果为now的child					
					auto err = this->AddChild(curNode, appNode);
					if (err) {
						printf_s("AddChild Error\n");
						exit(-1);
					}
				}
				else {
					//添加该扫描结果为curNode->child的sibling
					this->AddSibling(curNode->Child, appNode);
				}
				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					//如果是目录，加入队列
					dirQueue.push(appNode);
					//目录数统计
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
	//扫描完成，释放文件指针
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
		//是目录
		//之前没有记录过，则记录当前目录下的文件信息
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
				//是目录的话就跳过
				now = now->Sibling;
				continue;
			}
			//找到最早和最晚的文件节点
			if (!oldest || !newest) {
				//如果目录下存在文件，则用其初始化Newest和Oldest
				oldest = now;
				newest = now;
			}
			oldest = (oldest->ModifiedTime < now->ModifiedTime) ? oldest:now;
			newest = (newest->ModifiedTime > now->ModifiedTime) ? newest : now;
			//统计文件数量和文件总大小
			fileCount++;
			totalFileSize += now->FileSize;
			now = now->Sibling;
		}
		//返回结果
		*FileAmount = fileCount;
		*FileNewest = newest;
		*FileOldest = oldest;
		*TotalFileSize = totalFileSize;
		return 0;
	}
	else {
		//不是目录
		return -1;
	}
}

/// <summary>
/// 对文件节点进行增删改操作，成功返回0；同时会更新数据库的内容
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
		//新增文件节点
		//首先将文件名部分Trim掉
		TCHAR folderPath[MAX_PATHLEN];
		_tcscpy_s(folderPath, Path);
		int tail = _tcslen(folderPath) - 1;
		while (folderPath[tail] != L'\\') {
			folderPath[tail] = L'\0';
			tail--;
		}
		folderPath[tail] = L'\0';
		//检查文件夹是否存在
		Node* target = this->GetNodeByPath(folderPath);
		if (!target) {
			return -1;
		}
		//检查文件节点是否存在
		Node* fileTarget = this->GetNodeByPath(Path);
		if (fileTarget) {
			return -1;
		}
		//新增文件节点
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
		//修改文件节点
		//先检查文件节点是否存在
		Node* target = this->GetNodeByPath(Path);
		if (!target) {
			//文件节点不存在
			return -1;
		}
		//修改
		target->FileSize = Size;
		target->ModifiedTime = LastModifiedTime;
		return 0;
	}
	else if (!lowerCaseCmp(Mode, Modes[2])){
		//删除文件或文件夹节点
		Node* target = this->GetNodeByPath(Path);
		if (!target) {
			//节点不存在
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
/// 通过path获取指定的节点，失败返回NULL
/// </summary>
/// <param name="NodePath"></param>
/// <returns></returns>
Node* DirectoryTree::GetNodeByPath(TCHAR* NodePath) {
	Node* ret = NULL;
	Node* now = this->Root;

	while (now) {
		while (lowerCaseContain(NodePath, now->PathName)) {
			//向下搜索
			Node* p = now->Child;
			bool nowUpdateFlag = false;
			while(p){
				//检查now对应目录下一层目录
				if(lowerCaseContain(NodePath, p->PathName)) {
					now = p;
					nowUpdateFlag = true;
					break;
				}
				p = p->Sibling;
			}
			if (!nowUpdateFlag) {
				//now没有被更新，说明可能是now就是要找的节点，或文件树中没有对应目录
				break;
			}
		}
		if (!lowerCaseCmp(NodePath, now->PathName)) {
			//检索成功
			ret = now;
			break;
		}
		//检索失败，搜索now同级的其它目录
		now = now->Sibling;
	}
	return ret;
}

/// <summary>
/// 添加Sibling, 成功返回0
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
	//更新最大深度
	this->MaxRealDepth = max(target->RealDep, MaxRealDepth);
	//Sql相关操作
	this->Script->AddNode(target->NodeId,target->PathName, target->FileAttribute,target->ModifiedTime,target->FileSize, 0, 0, target->Parent->NodeId);
	this->Script->UpdateRelation(p);
	return 0;
}

/// <summary>
/// 添加Child，成功返回0
/// </summary>
/// <param name="target"></param>
/// <returns></returns>
int DirectoryTree::AddChild(Node* base, Node* target) {
	Node* p = base;
	if (p->Child) {
		//一个节点不能有多个Child
		return -1;
	}
	base->Child = target;
	target->Parent = base;
	target->RealDep = base->RealDep + 1;
	target->RealParent = base;
	//更新最大深度
	this->MaxRealDepth = max(target->RealDep, MaxRealDepth);
	//Sql相关操作
	this->Script->AddNode(target->NodeId,target->PathName, target->FileAttribute, target->ModifiedTime,target->FileSize, 0, 0, target->Parent->NodeId);
	this->Script->UpdateRelation(p);
	return 0;
}
/// <summary>
/// 删除指定的节点，如果成功返回0
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
int DirectoryTree::DeleteNode(Node* p)
{
	if (!p) {
		return -1;
	}
	Node* isoTreeRoot = p->Child;//删除节点后可能会有一棵被独立出来的树
	if (!(p->RealParent)) {
		//根节点，直接删了
		this->NodeMap.erase(p->NodeId);
		delete p;
	}
	else {
		if (p->Parent == p->RealParent) {
			//RealParent是上层目录
			p->RealParent->Child = p->Sibling;
			if(p->Sibling) p->Sibling->RealParent = p->RealParent;
		}
		else {
			p->RealParent->Sibling = p->Sibling;
			if(p->Sibling) p->Sibling->RealParent = p->RealParent;
		}
		//统计数量
		p->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ?
			this->DirCount-- : this->FileCount--;
		this->NodeMap.erase(p->NodeId);
		//释放内存
		delete p;
	}
	if (isoTreeRoot) {
		//删除独立出来的无效的节点组成的树,isoTreeRoot为根
		std::queue<Node*> nodesToDelete;
		nodesToDelete.push(isoTreeRoot);
		while (nodesToDelete.size()) {
			//层次遍历并删除节点
			Node* curNode = nodesToDelete.front();
			nodesToDelete.pop();
			if (curNode->Child)  nodesToDelete.push(curNode->Child);
			if (curNode->Sibling) nodesToDelete.push(curNode->Sibling);
			//统计数量
			curNode->FileAttribute& FILE_ATTRIBUTE_DIRECTORY ?
				this->DirCount-- : this->FileCount--;
			this->NodeMap.erase(curNode->NodeId);
			//释放内存
			delete curNode;

		}
	}
	return 0;
}


#pragma endregion

/// <summary>
/// 将节点p的目录信息格式化输出为txt文件
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
int DirectoryTree::DumpDirInfo(Node* p,TCHAR* txtFileName)
{
	TCHAR buf[MAX_PATHLEN * 10];
	//遍历以该节点为根的目录树
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
		//输出
		_stprintf_s(buf, MAX_PATHLEN * 10,
			_T("##类型-%ls 路径： %ls \n大小:%lld 修改时间:%ls\n"),
			now->FileAttribute&FILE_ATTRIBUTE_DIRECTORY ? _T("目录"):_T("文件"),
			now->PathName,now->FileSize,formattedTime);
		fwrite(buf, sizeof(TCHAR), _tcslen(buf), txtFp);
		//为了能够以线性复杂度对比文件，要先统计完当层目录再统计下一层
		if (now->Sibling && now != p) {//要避免将同级其余目录统计进去
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
/// 用NodeID获取对应的Node对象，失败返回NULL
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