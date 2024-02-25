#include"controller.h"

CommandReader::CommandReader(TCHAR* FileName) {
	this->DirTree = new DirectoryTree(_T("C:\\Windows"));
	_tfopen_s(&ReadFp,FileName, _T("r"));
	_tfopen_s(&WriteFp, L"result.txt", L"w,ccs=UNICODE");
}

int CommandReader::ExecuteCommand() {
	int mode = 0;
	fgetws(ReadBuf, MAX_PATHLEN * 3, ReadFp);
	this->CommandTrim();
	if (!lowerCaseCmp(this->Args[0], _T("stat Dirs"))) {
		mode = MODE_MYSTAT;
	}
	else if (!lowerCaseCmp(this->Args[0], _T("selected dirs"))) {
		mode = MODE_MYDIR;
	}
	else if (!lowerCaseCmp(this->Args[0], _T("selected files"))) {
		mode = MODE_MYFILE;
	}
	else {
		//命令错误
		return -1;
	}
	while (1) {
		fgetws(ReadBuf, MAX_PATHLEN * 3, ReadFp);
		int count = this->CommandTrim();
		if (!lowerCaseCmp(_T("end of dirs"),Args[0]) || !lowerCaseCmp(_T("end of files"),Args[0])) {
			//到达文件尾
			break;
		}
		switch (mode) {
			case MODE_MYSTAT:
				//文件夹统计
				this->Args[0][_tcslen(Args[0]) - 1] = _T('\0');//去掉末尾的\号
				this->GetFolderStat(Args[0]);
				break;
			case MODE_MYDIR:
				//目录修改（只有删除）操作
				Args[0][_tcslen(Args[0]) - 1] = _T('\0');//去掉末尾的\号
				if (!this->DirTree->AlterNode(Args[0],Args[1],0,0 )) {
					_stprintf_s(this->WriteBuf, _T("已删除目录:%ls\n"), Args[0]);
				}
				else {
					_stprintf_s(this->WriteBuf, _T("未找到目录:%ls\n"), Args[0]);
				}
				break;
			case MODE_MYFILE:
				//文件修改（删除，修改大小，新增）操作
				if (!this->DirTree->AlterNode(Args[0], Args[1], _ttoi(Args[2]), _ttoi(Args[3]))) {
					_stprintf_s(this->WriteBuf, _T("已对文件%ls进行[%ls]操作\n"), Args[0],Args[1]);
				}
				else {
					_stprintf_s(this->WriteBuf, _T("找不到文件:%ls\n"), Args[0]);
				}
				break;
			default:
				return -1;
		}
		auto err = this->WriteResult();

	}

	return 0;
}

/// <summary>
/// 以逗号Trim参数，返回Trim后的参数数量
/// </summary>
/// <returns></returns>
int CommandReader::CommandTrim() {
	TCHAR* head = ReadBuf;
	TCHAR* tail = head;
	int argIndex = 0;
	while (*tail != _T('\n') && !feof(ReadFp)) {
		if (*tail == _T(',')) {
			//到分界点
			*tail = _T('\0');
			_tcscpy_s(this->Args[argIndex++], head);
			head = ++tail;
		}
		tail++;
	}
	*tail = _T('\0');
	_tcscpy_s(this->Args[argIndex++], head);
	return argIndex;
}

/// <summary>
/// 获取对应的PATH信息，将其格式化输出到this->WriteBuf中
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
int CommandReader::GetFolderStat(TCHAR* Path) {
	Node *oldest = NULL, *newest = NULL;
	INT64 filesize=0;
	int count = 0;
	
	Node *target = this->DirTree->GetNodeByPath(Path);
	if (!target) {
		//找不到对应的目录
		_stprintf_s(this->WriteBuf, _T("No such directory:%s\n"), Path);
		return -1;
	}
	this->DirTree->GetDirectoryInfo(target, &oldest, &newest, &count, &filesize);
	swprintf_s(this->WriteBuf, 
		_T(
			"===================================\n"
			"目录:%ls\n"
			"时间最早的文件：%ls\n"
			"时间最晚的文件：%ls\n"
			"目录下的文件总数：%d\n"
			"目录下的文件大小总和：%lld (bytes)\n"
		),
		Path,oldest->PathName,newest->PathName,count,filesize);
	return 0;
}

/// <summary>
/// 将this->WriteBuf的内容写到WriteFp中，成功返回0
/// </summary>
/// <returns></returns>
int CommandReader::WriteResult()
{
	setlocale(LC_ALL, "");
	return EOF == fwrite(this->WriteBuf, sizeof(TCHAR), _tcslen(this->WriteBuf), this->WriteFp);
}