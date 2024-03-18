#pragma once
#include<tchar.h>
#include<windows.h>
#define MAX_PATHLEN 1000

#define WINDOWS_UNIX_TIME_DIFF 1164447360

int lowerCaseCmp(TCHAR* str1, TCHAR* str2);
TCHAR* lowerCaseContain(TCHAR* str1, TCHAR* str2);
int dumpFolderStat(TCHAR* txtFileName, TCHAR* buf);
int unixTimeToFormattedString(UINT64 UnixTime, TCHAR* buf);
