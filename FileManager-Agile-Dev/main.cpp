#include"scanner.h"

int main() {
	Scanner scanner;
	TCHAR* path = (TCHAR*)TEXT("D:\\DOCS\\*");
	scanner.PrintAllFilesInDirectory(path);
	return 0;
}