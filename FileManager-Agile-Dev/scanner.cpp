#include "scanner.h"
#include<iostream>

using namespace std;
int Scanner::DirectoryTraverse(TCHAR* path) {
	queue<TCHAR*> directories;
	WIN32_FIND_DATA data;
	TCHAR* subDirPath;//��������Ŀ¼������
	TCHAR currentPath[MAX_PATHLEN];//��������ǰĿ¼������
	directories.push(path);
	//��BFS����Ŀ¼��
	while (!directories.empty()) {//���зǿ�ʱ
		wsprintf(currentPath, L"%ls%ls", directories.front(), L"\\*");
		//printf("%d\n", (int)directories.size());
		HANDLE hFind = FindFirstFile(currentPath, &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				setlocale(LC_ALL, "");
				//wprintf(L"%ls\t %ls\n", data.cFileName, data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? L"Directory" : L"File");
				if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && lstrcmpW(data.cFileName, L".") && lstrcmpW(data.cFileName, L"..")) {
					subDirPath = new TCHAR[MAX_PATHLEN];//Ҫ���ڴ��з���ռ䣬Ȼ��dequeֻ��ָ��
					if (subDirPath == NULL) {
						//�����ڴ����
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
