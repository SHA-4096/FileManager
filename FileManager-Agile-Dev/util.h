#pragma once
#include<tchar.h>
#include<windows.h>
#define MAX_PATHLEN 1000

int lowerCaseCmp(TCHAR* str1, TCHAR* str2);
TCHAR* lowerCaseContain(TCHAR* str1, TCHAR* str2);
int cmpDiff(TCHAR* PathOrigin, TCHAR* PathNew, TCHAR* Pathresult);