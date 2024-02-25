#include"util.h"

/// <summary>
/// 简单的忽略大小写的字符串比较
/// </summary>
/// <param name="str1"></param>
/// <param name="str2"></param>
/// <returns></returns>
int lowerCaseCmp(TCHAR* str1, TCHAR* str2) {
	TCHAR p1[MAX_PATHLEN], p2[MAX_PATHLEN];
	_tcscpy_s(p1, str1);
	_tcscpy_s(p2, str2);
	_tcslwr_s(p1);
	_tcslwr_s(p2);
	return _tcscmp(p1,p2);
}

TCHAR* lowerCaseContain(TCHAR* str1, TCHAR* str2) {
	TCHAR p1[MAX_PATHLEN], p2[MAX_PATHLEN];
	_tcscpy_s(p1, str1);
	_tcscpy_s(p2, str2);
	_tcslwr_s(p1);
	_tcslwr_s(p2);
	return _tcsstr(p1, p2);
}