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