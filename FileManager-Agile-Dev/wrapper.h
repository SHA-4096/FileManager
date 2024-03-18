#pragma once
#include"controller.h"
#include"util.h"

public ref class FileManagerWrapper
{
public:
	FileManagerWrapper();
	int OpenCommandFile(BYTE* FileAbsolutePath);
	int ScanDir(BYTE* Path);
	int ExecuteCommand();
	BYTE* GetRoot();
	BYTE* GetChild(BYTE* Path);
	BYTE* GetParent();
	BYTE* ProvideSibling();
	int BeginFolderStatCompare();
	int BeginFullDiffCompare();
	int SetFullDiffNode(BYTE* Path);
	//int ReadResult();//�ü��±��򿪽���ļ�
	int Debug();
	
private:
	ControllerClass* Reader;//Wrapper for ControllerClass
	Node* CurrentPointer;
};
