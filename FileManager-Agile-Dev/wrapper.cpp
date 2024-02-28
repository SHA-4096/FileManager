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