#include"util.h"
#include<locale>
#include<stdio.h>



/// <summary>
/// 简单的忽略大小写的字符串比较
/// </summary>
/// <param name="str1"></param>
/// <param name="str2"></param>
/// <returns></returns>
int lowerCaseCmp(TCHAR* str1, TCHAR* str2) {
	setlocale(LC_ALL, "");
	TCHAR p1[MAX_PATHLEN], p2[MAX_PATHLEN];
	_tcscpy_s(p1, str1);
	_tcscpy_s(p2, str2);
	_tcslwr_s(p1);
	_tcslwr_s(p2);
	return _tcscmp(p1,p2);
}

TCHAR* lowerCaseContain(TCHAR* str1, TCHAR* str2) {
	setlocale(LC_ALL, "");
	TCHAR p1[MAX_PATHLEN], p2[MAX_PATHLEN];
	_tcscpy_s(p1, str1);
	_tcscpy_s(p2, str2);
	_tcslwr_s(p1);
	_tcslwr_s(p2);
	return _tcsstr(p1, p2);
}

/// <summary>
/// 将统计信息输出到指定的文件
/// </summary>
/// <param name="txtFileName"></param>
/// <param name="buf"></param>
/// <returns></returns>
int dumpFolderStat(TCHAR* txtFileName, TCHAR* buf) {
	FILE* dumperFp = NULL;
	_tfopen_s(&dumperFp, txtFileName, _T("a,ccs=UTF-8"));
	fwrite(buf, sizeof(TCHAR), _tcslen(buf), dumperFp);
	fclose(dumperFp);
	return 0;
}

int unixTimeToFormattedString(UINT64 UnixTime, TCHAR* buf)
{
	time_t t = (time_t)static_cast<int64_t>(UnixTime);
	struct tm tmpTm;
	gmtime_s(&tmpTm, &t);
	_tcsftime(buf, 100, _T("%Y-%m-%d %H:%M:%S"), &tmpTm);
	return 0;
}
