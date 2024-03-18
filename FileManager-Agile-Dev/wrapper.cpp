#include"wrapper.h"

FileManagerWrapper::FileManagerWrapper() {
	//this->Reader = new ControllerClass(_T("mydir.txt"));
	this->Reader = new ControllerClass();
}

/// <summary>
/// ��ָ���������ļ�
/// </summary>
/// <param name="FileAbsolutePath"></param>
/// <returns></returns>
int FileManagerWrapper::OpenCommandFile(BYTE* FileAbsolutePath) {
	//ת��BYTE*ΪTCHAR*
	TCHAR* tFileAbsolutePath = new TCHAR[MAX_PATHLEN];
	_tcscpy_s(tFileAbsolutePath,MAX_PATHLEN ,(TCHAR*)FileAbsolutePath);
	this->Reader->SetCommandScript(tFileAbsolutePath);
	return 0;
}

/// <summary>
/// ִ�ж�ȡ��������ɹ�����0
/// </summary>
/// <returns></returns>
int FileManagerWrapper::ExecuteCommand()
{
	this->Reader->ExecuteCommand();
	return 0;
}

/// <summary>
/// ɨ��ָ��Ŀ¼���ɹ�����0
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
int FileManagerWrapper::ScanDir(BYTE* Path) {
	//ת��BYTE*ΪTCHAR*
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
		_T("���ͣ�%ls\n���ƣ�%ls\n��С��%lld\n�޸�ʱ�䣺%ls\nʱ�����%llu"),
		this->CurrentPointer->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ? _T("Ŀ¼") : _T("�ļ�"),
		this->CurrentPointer->PathName, this->CurrentPointer->FileSize, formattedTime,this->CurrentPointer->ModifiedTime);
	//ת��this->Reader->Writebuf�������ΪBYTE����
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
			_T("���ͣ�%ls\n���ƣ�%ls\n��С��%lld\n�޸�ʱ�䣺%ls\nʱ�����%llu"),
			this->CurrentPointer->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ? _T("Ŀ¼") : _T("�ļ�"),
			this->CurrentPointer->PathName, this->CurrentPointer->FileSize, formattedTime, this->CurrentPointer->ModifiedTime);
		//ת��this->Reader->Writebuf�������ΪBYTE����
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
/// ����Ҫ���ȫ���仯�Ľڵ㣬�����õ�һ������
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
int FileManagerWrapper::SetFullDiffNode(BYTE* Path)
{
	//ת��BYTE*ΪTCHAR*
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
		//Ӧ��ĿҪ��ֻά��һ���ڵ�
		this->Reader->FullDiffNodes.clear();
	}
	this->Reader->FullDiffNodes.push_back(target->NodeId);
	return 0;
}


/// <summary>
/// �ɹ��޸ĵ�ǰ�ڵ�Ϊ�ӽڵ㣬�����޸�
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
BYTE* FileManagerWrapper::GetChild(BYTE* Path)
{
	//ת��BYTE*ΪTCHAR*
	TCHAR* tPath = new TCHAR[MAX_PATHLEN];
	_tcscpy_s(tPath, MAX_PATHLEN, (TCHAR*)Path);
	Node* target = this->Reader->DirTree->GetNodeByPath(tPath);
	if (target&&target->Child) {
		this->CurrentPointer = target->Child;
		TCHAR formattedTime[100];
		unixTimeToFormattedString(this->CurrentPointer->ModifiedTime, formattedTime);
		_stprintf_s(this->Reader->WriteBuf, MAX_PATHLEN * 10,
			_T("���ͣ�%ls\n���ƣ�%ls\n��С��%lld\n�޸�ʱ�䣺%ls\nʱ�����%llu"),
			this->CurrentPointer->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ? _T("Ŀ¼") : _T("�ļ�"),
			this->CurrentPointer->PathName, this->CurrentPointer->FileSize, formattedTime, this->CurrentPointer->ModifiedTime);
		//ת��this->Reader->Writebuf�������ΪBYTE����
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
			this->CurrentPointer = this->CurrentPointer->Parent->Child;//��λ������Ŀ¼��һ���ڵ㣬����Ǹ��ڵ�Ļ��Ͳ��ø�λ
		}
		TCHAR formattedTime[100];
		unixTimeToFormattedString(this->CurrentPointer->ModifiedTime, formattedTime);
		_stprintf_s(this->Reader->WriteBuf, MAX_PATHLEN * 10,
			_T("���ͣ�%ls\n���ƣ�%ls\n��С��%lld\n�޸�ʱ�䣺%ls\nʱ�����%llu"),
			this->CurrentPointer->FileAttribute & FILE_ATTRIBUTE_DIRECTORY ? _T("Ŀ¼") : _T("�ļ�"),
			this->CurrentPointer->PathName, this->CurrentPointer->FileSize, formattedTime, this->CurrentPointer->ModifiedTime);
		//ת��this->Reader->Writebuf�������ΪBYTE����
		WideCharToMultiByte(CP_ACP, 0, this->Reader->WriteBuf, -1, (LPSTR)this->Reader->WriteBuf, MAX_PATHLEN * 10, NULL, NULL);
		return (BYTE*)this->Reader->WriteBuf;
	}
	else {
		return NULL;
	}
}
