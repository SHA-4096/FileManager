#pragma once
#include"controller.h"

public ref class FileManagerWrapper
{
public:
	FileManagerWrapper();
	int OpenCommandFile(BYTE* FileAbsolutePath);
	int ScanDir(BYTE* Path);
	int ExecuteCommand();
	int Debug();
private:
	ControllerClass* Reader;//Wrapper for ControllerClass
};
