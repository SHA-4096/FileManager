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
/// 初始化脚本文件，成功返回0
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
/// 新增节点，时间格式化之后以字符串存储
/// </summary>
/// <param name="path"></param>
/// <param name="attr"></param>
/// <param name="time"></param>
/// <param name="child"></param>
/// <param name="sibling"></param>
/// <param name="parent"></param>
/// <returns></returns>
int SqlScript::AddNode(TCHAR* path, DWORD attr, FILETIME time, int child, int sibling, int parent)
{
	auto SysTime = new SYSTEMTIME();
	FileTimeToSystemTime(&time, SysTime);
	TCHAR buf[MAX_PATHLEN*2];
	auto formattedTime = new TCHAR[20];
	_stprintf_s(formattedTime, 20, _T("%d-%d-%d"), SysTime->wYear, SysTime->wMonth, SysTime->wDay);
	_stprintf_s(buf, MAX_PATHLEN*2, _T("INSERT INTO `file` (`path`, `attr`, `time`, `child`, `sibling`, `parent`) VALUES ('%ls', %d, %ls, %d, %d, %d);\n"), 
		path, attr, formattedTime, child, sibling, parent);
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
	_stprintf_s(buf, MAX_PATHLEN * 2, _T("UPDATE `file` SET `child` = %d, `sibling` = %d, `parent` = %d WHERE `node_id` = %d;\n"),
		CHILD_ID(p), SIBLING_ID(p), PARENT_ID(p), SELF_ID(p));
	AppendScript(buf);
	return 0;
}

/// <summary>
/// 向文件中写入buf，成功返回0
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
	this->CreatedTime = Data->ftCreationTime;
	this->FileSize = (Data->nFileSizeHigh * (MAXDWORD + 1)) + Data->nFileSizeLow;
	//this->FileSize /= (1024 * 1024);//以MB计
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
	this->Script = new SqlScript();
	setlocale(LC_ALL, "");
	WIN32_FIND_DATA data;
	TCHAR currentPath[MAX_PATHLEN];//用来做当前目录的搜索
	TCHAR subDirPath[MAX_PATHLEN];//用来保存全路径
	HANDLE hFind = FindFirstFile(RootPath, &data);
	std::queue<Node*> dirQueue;
	this->Root = new Node(&data,0, this->IdAccumulator++, RootPath);
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
				if (!(_tcscmp(data.cFileName, L".") && _tcscmp(data.cFileName, L".."))) {
					//跳过.和..
					continue;
				}
				//新建节点
				wsprintf(subDirPath, L"%ls\\%ls", curNode->PathName, data.cFileName);
				appNode = new Node(&data, curDep + 1, this->IdAccumulator++, subDirPath);
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
int DirectoryTree::GetDirectoryInfo(Node* p,Node* FileOldest,Node* FileNewest,int* FileAmount,INT64* TotalFileSize) {
	if (p->FileAttribute & FILE_ATTRIBUTE_DIRECTORY) {
		//是目录
		Node* now = p->Child;
		Node* newest = NULL;
		Node* oldest = NULL;
		int fileCount = 0;
		INT64 totalFileSize = 0;
		while (now) {
			if (now->FileAttribute & FILE_ATTRIBUTE_DIRECTORY) {
				//是目录的话就跳过
				continue;
			}
			//找到最早和最晚的文件节点
			if (!oldest || !newest) {
				//如果目录下存在文件，则用其初始化Newest和Oldest
				oldest = now;
				newest = now;
			}
			oldest = (CompareFileTime(&(oldest->CreatedTime),&(now->CreatedTime))<0) ? oldest:now;
			newest = (CompareFileTime(&(newest->CreatedTime), &(now->CreatedTime))>0) ? newest : now;
			//统计文件数量和文件总大小
			fileCount++;
			totalFileSize += now->FileSize;
			now = now->Sibling;
		}
		//返回结果
		*FileAmount = fileCount;
		FileNewest = newest;
		FileOldest = oldest;
		*TotalFileSize = totalFileSize;
		return 0;
	}
	else {
		//不是目录
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
		while (wcsstr(NodePath, now->PathName)) {
			//向下搜索
			Node* p = now->Child;
			bool nowUpdateFlag = false;
			while(p){
				//检查now对应目录下一层目录
				if(wcsstr(NodePath, p->PathName)) {
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
		if (!_tcscmp(NodePath, now->PathName)) {
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
	//更新最大深度
	this->MaxRealDepth = max(target->RealDep, MaxRealDepth);
	//Sql相关操作
	this->Script->AddNode(target->PathName, target->FileAttribute, target->CreatedTime, 0, 0, target->Parent->NodeId);
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
	//更新最大深度
	this->MaxRealDepth = max(target->RealDep, MaxRealDepth);
	//Sql相关操作
	this->Script->AddNode(target->PathName, target->FileAttribute, target->CreatedTime, 0, 0, target->Parent->NodeId);
	this->Script->UpdateRelation(p);
	return 0;
}
#pragma endregion

