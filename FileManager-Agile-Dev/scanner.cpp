#include "scanner.h"
#include<iostream>

using namespace std;
int Scanner::DirectoryTraverse(TCHAR* path) {
	queue<TCHAR*> directories;
	WIN32_FIND_DATA data;
	TCHAR* subDirPath;//用来做子目录的搜索
	TCHAR currentPath[MAX_PATHLEN];//用来做当前目录的搜索
	directories.push(path);
	//用BFS遍历目录树
	while (!directories.empty()) {//队列非空时
		wsprintf(currentPath, L"%ls%ls", directories.front(), L"\\*");
		//printf("%d\n", (int)directories.size());
		HANDLE hFind = FindFirstFile(currentPath, &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				setlocale(LC_ALL, "");
				//wprintf(L"%ls\t %ls\n", data.cFileName, data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? L"Directory" : L"File");
				if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && lstrcmpW(data.cFileName, L".") && lstrcmpW(data.cFileName, L"..")) {
					subDirPath = new TCHAR[MAX_PATHLEN];//要在内存中分配空间，然后deque只存指针
					if (subDirPath == NULL) {
						//处理内存溢出
						cout << "Memory allocation failed!" << endl;
						return -1;
					}
					wsprintf(subDirPath, L"%ls\\%ls", directories.front(), data.cFileName);
					directories.push(subDirPath);
				}
			} while (FindNextFile(hFind, &data));
		}
		else {
			wprintf(L"FindFirstFile failed at %ls\n", currentPath);
		}
		auto ptrToFree = directories.front();
		directories.pop();
		//free(ptrToFree);//Not Legal!
		//wprintf(L"%ls\n", ptrToFree);
		delete[] ptrToFree;
		FindClose(hFind);
	}
	return 0;
}
