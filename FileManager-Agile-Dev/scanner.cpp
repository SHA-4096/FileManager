#include "scanner.h"
#include<iostream>
using namespace std;
int Scanner::PrintAllFilesInDirectory(TCHAR* path) {
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(path, &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			setlocale(LC_ALL, "");
			wprintf(L"%ls\n", data.cFileName);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	return 0;
}
