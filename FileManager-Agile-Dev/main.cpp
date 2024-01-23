#include"scanner.h"

int main() {
	Scanner scanner;

	TCHAR* path = new TCHAR[50];
	wsprintf(path, L"C:\\Windows");
	scanner.DirectoryTraverse(path);
	return 0;
}