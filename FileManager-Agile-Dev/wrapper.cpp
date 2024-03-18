#include"wrapper.h"

FileManagerWrapper::FileManagerWrapper() {
	//this->Reader = new ControllerClass(_T("mydir.txt"));
	this->Reader = new ControllerClass();
}

/// <summary>
/// 打开指定的命令文件
/// </summary>
/// <param name="FileAbsolutePath"></param>
/// <returns></returns>
int FileManagerWrapper::OpenCommandFile(BYTE* FileAbsolutePath) {
	//转换BYTE*为TCHAR*
	TCHAR* tFileAbsolutePath = new TCHAR[MAX_PATHLEN];
	_tcscpy_s(tFileAbsolutePath,MAX_PATHLEN ,(TCHAR*)FileAbsolutePath);
	this->Reader->SetCommandScript(tFileAbsolutePath);
	return 0;
}

/// <summary>
/// 执行读取到的命令，成功返回0
/// </summary>
/// <returns></returns>
int FileManagerWrapper::ExecuteCommand()
{
	this->Reader->ExecuteCommand();
	return 0;
}

/// <summary>
/// 扫描指定目录，成功返回0
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
int FileManagerWrapper::ScanDir(BYTE* Path) {
	//转换BYTE*为TCHAR*
	TCHAR* tPath = new TCHAR[MAX_PATHLEN];
	_tcscpy_s(tPath, MAX_PATHLEN, (TCHAR*)Path);
	return this->Reader->ScanDir(tPath);
}

int FileManagerWrapper::Debug() {
	this->Reader->Debug();
	return 0;
}

BYTE* FileManagerWrapper::GetRoot()
{
	this->CurrentPointer = this->Reader->DirTree->Root;
	TCHAR formattedTime[100];
	unixTimeToFormattedString(this->CurrentPointer->ModifiedTime, formattedTime);
	_stprintf_s(this->Reader->WriteBuf, MAX_PATHLEN * 10,
		_T("类型：%ls\n名称：%ls\n大小：%lld\n修改时间：%ls\n时间戳：%llu"),
		this->CurrentPointer->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ? _T("目录") : _T("文件"),
		this->CurrentPointer->PathName, this->CurrentPointer->FileSize, formattedTime,this->CurrentPointer->ModifiedTime);
	//转换this->Reader->Writebuf里的内容为BYTE数组
	WideCharToMultiByte(CP_ACP, 0, this->Reader->WriteBuf, -1, (LPSTR)this->Reader->WriteBuf, MAX_PATHLEN * 10, NULL, NULL);
	//Dest = (BYTE*)this->Reader->WriteBuf;
	return (BYTE*)this->Reader->WriteBuf;

}


BYTE* FileManagerWrapper::ProvideSibling()
{
	if (this->CurrentPointer->Sibling) {
		this->CurrentPointer = this->CurrentPointer->Sibling;
		TCHAR formattedTime[100];
		unixTimeToFormattedString(this->CurrentPointer->ModifiedTime, formattedTime);
		_stprintf_s(this->Reader->WriteBuf, MAX_PATHLEN * 10,
			_T("类型：%ls\n名称：%ls\n大小：%lld\n修改时间：%ls\n时间戳：%llu"),
			this->CurrentPointer->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ? _T("目录") : _T("文件"),
			this->CurrentPointer->PathName, this->CurrentPointer->FileSize, formattedTime, this->CurrentPointer->ModifiedTime);
		//转换this->Reader->Writebuf里的内容为BYTE数组
		WideCharToMultiByte(CP_ACP, 0, this->Reader->WriteBuf, -1, (LPSTR)this->Reader->WriteBuf, MAX_PATHLEN * 10, NULL, NULL);
		return (BYTE*)this->Reader->WriteBuf;
	}
	else {
		return NULL;
	}
}

int FileManagerWrapper::BeginFolderStatCompare()
{
	return this->Reader->BeginFolderStatCompare();
}

int FileManagerWrapper::BeginFullDiffCompare()
{
	return this->Reader->BeginFullDiffCompare();
}

/// <summary>
/// 设置要检查全量变化的节点，并设置第一个备份
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
int FileManagerWrapper::SetFullDiffNode(BYTE* Path)
{
	//转换BYTE*为TCHAR*
	TCHAR* tPath = new TCHAR[MAX_PATHLEN];
	_tcscpy_s(tPath, MAX_PATHLEN, (TCHAR*)Path);
	Node* target = this->Reader->DirTree->GetNodeByPath(tPath);
	if (!target) {
		return -1;
	}
	TCHAR fileName[MAX_PATHLEN];
	_stprintf_s(fileName, MAX_PATHLEN, _T("node%d_ver0.txt"), target->NodeId);
	this->Reader->DirTree->DumpDirInfo(target, fileName);
	if (this->Reader->FullDiffNodes.size()) {
		//应题目要求，只维护一个节点
		this->Reader->FullDiffNodes.clear();
	}
	this->Reader->FullDiffNodes.push_back(target->NodeId);
	return 0;
}


/// <summary>
/// 成功修改当前节点为子节点，否则不修改
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
BYTE* FileManagerWrapper::GetChild(BYTE* Path)
{
	//转换BYTE*为TCHAR*
	TCHAR* tPath = new TCHAR[MAX_PATHLEN];
	_tcscpy_s(tPath, MAX_PATHLEN, (TCHAR*)Path);
	Node* target = this->Reader->DirTree->GetNodeByPath(tPath);
	if (target&&target->Child) {
		this->CurrentPointer = target->Child;
		TCHAR formattedTime[100];
		unixTimeToFormattedString(this->CurrentPointer->ModifiedTime, formattedTime);
		_stprintf_s(this->Reader->WriteBuf, MAX_PATHLEN * 10,
			_T("类型：%ls\n名称：%ls\n大小：%lld\n修改时间：%ls\n时间戳：%llu"),
			this->CurrentPointer->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ? _T("目录") : _T("文件"),
			this->CurrentPointer->PathName, this->CurrentPointer->FileSize, formattedTime, this->CurrentPointer->ModifiedTime);
		//转换this->Reader->Writebuf里的内容为BYTE数组
		WideCharToMultiByte(CP_ACP, 0, this->Reader->WriteBuf, -1, (LPSTR)this->Reader->WriteBuf, MAX_PATHLEN * 10, NULL, NULL);
		return (BYTE*)this->Reader->WriteBuf;
	}
	return NULL;
}

BYTE* FileManagerWrapper::GetParent()
{
	if (this->CurrentPointer->Parent) {
		this->CurrentPointer = this->CurrentPointer->Parent;
		if (this->CurrentPointer->Parent)
		{
			this->CurrentPointer = this->CurrentPointer->Parent->Child;//复位到本层目录第一个节点，如果是根节点的话就不用复位
		}
		TCHAR formattedTime[100];
		unixTimeToFormattedString(this->CurrentPointer->ModifiedTime, formattedTime);
		_stprintf_s(this->Reader->WriteBuf, MAX_PATHLEN * 10,
			_T("类型：%ls\n名称：%ls\n大小：%lld\n修改时间：%ls\n时间戳：%llu"),
			this->CurrentPointer->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ? _T("目录") : _T("文件"),
			this->CurrentPointer->PathName, this->CurrentPointer->FileSize, formattedTime, this->CurrentPointer->ModifiedTime);
		//转换this->Reader->Writebuf里的内容为BYTE数组
		WideCharToMultiByte(CP_ACP, 0, this->Reader->WriteBuf, -1, (LPSTR)this->Reader->WriteBuf, MAX_PATHLEN * 10, NULL, NULL);
		return (BYTE*)this->Reader->WriteBuf;
	}
	else {
		return NULL;
	}
}
