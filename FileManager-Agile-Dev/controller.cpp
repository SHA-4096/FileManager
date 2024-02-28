#include"controller.h"
int ControllerClass::SetCommandScript(TCHAR* FileName) {
	if (ReadFp) {
		fclose(ReadFp);	
	}
	return _tfopen_s(&ReadFp, FileName, _T("r,ccs=UTF-8"));
	
}

int ControllerClass::ScanDir(TCHAR* Path) {
	this->DirTree = new DirectoryTree(Path);
	//DEBUG
	this->DirTree->DumpDirInfo(this->DirTree->GetNodeById(1), _T("node1_ver0.txt"));
	this->FullDiffNodes.push_back(1);
	//END DEBUG
	return 0;
}

int ControllerClass::ExecuteCommand() {
	int mode = 0;
	fgetws(ReadBuf, MAX_PATHLEN * 3, ReadFp);
	this->CommandTrim();//读取命令中的各项参数
	if (!lowerCaseCmp(this->Args[0], _T("stat dirs"))) {
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
		int tmpId = 0;
		switch (mode) {
			case MODE_MYSTAT:
				//文件夹统计
				this->Args[0][_tcslen(Args[0]) - 1] = _T('\0');//去掉末尾的\号
				tmpId = this->GetFolderStat(Args[0]);
				if (tmpId != -1) {
					this->FolderStatNodes.push_back(tmpId);
//					this->DirTree->UpdatedNodes.push_back(tmpId);
				}
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
	//进行信息统计
	//遍历UpdatedNodes
	/*if (mode != MODE_MYSTAT) {
		for (int index = 0; index < this->DirTree->UpdatedNodes.size(); index++) {
			int nodeId = this->DirTree->UpdatedNodes[index];
			TCHAR txtNewFileName[MAX_PATHLEN];
			_stprintf_s(txtNewFileName, _T("node%d_ver%d.txt"), nodeId, this->DirTree->VersionControl + 1);
			TCHAR txtOldFileName[MAX_PATHLEN];
			_stprintf_s(txtOldFileName, _T("node%d_ver%d.txt"), nodeId, this->DirTree->VersionControl);
			this->DirTree->DumpDirInfo(this->DirTree->GetNodeById(nodeId), txtNewFileName);
			cmpDiff(txtOldFileName, txtNewFileName, L"cmp_result.txt");
		}
	}*/
	if (mode != MODE_MYSTAT) {
		//对于FolderStatNodes,只需比较目录差异就行
		TCHAR txtFileNameOld[MAX_PATHLEN], txtFileNameNew[MAX_PATHLEN];
		_stprintf_s(txtFileNameOld, _T("folders_ver%d.txt"), 0);
		_stprintf_s(txtFileNameNew, _T("folders_ver%d.txt"), this->DirTree->VersionControl);
		for (int index = 0; index < this->FolderStatNodes.size(); index++) {
			this->DirTree->VersionControl++;//先设置新的版本号
			int NodeId = this->FolderStatNodes[index];
			Node* target = this->DirTree->GetNodeById(NodeId);
			if (!target) {
				continue;
			}
			else {
				this->GetFolderStat(target->PathName);
			}
			
			this->DirTree->VersionControl--;//恢复旧的版本号
		}
		this->CmpFolderDiff(txtFileNameOld, txtFileNameNew);

		//对于FullDiffNodes,需要比较其所有子节点的差异
		TCHAR txtFileNameResult[MAX_PATHLEN];
		for (int index = 0; index < this->FullDiffNodes.size(); index++) {
			this->DirTree->VersionControl++;//设置新的版本号
			int NodeId = this->FullDiffNodes[index];			
			Node* target = this->DirTree->GetNodeById(NodeId);
			if (!target) {
				continue;
			}
			else {
				_stprintf_s(txtFileNameNew, _T("node%d_ver%d.txt"), target->NodeId, this->DirTree->VersionControl);
				this->DirTree->DumpDirInfo(target, txtFileNameNew);
			}
			_stprintf_s(txtFileNameOld, _T("node%d_ver%d.txt"), target->NodeId, 0);
			_stprintf_s(txtFileNameResult, _T("noed%d_full_compare_result.txt"), target->NodeId);
			//this->CmpFullDiff(txtFileNameOld, txtFileNameNew, txtFileNameResult);
			cmpDiff(txtFileNameOld, txtFileNameNew, txtFileNameResult);
			this->DirTree->VersionControl--;//恢复旧的版本号
		}
		//此次操作结束，更新版本号
		this->DirTree->VersionControl++;
	}
	return 0;
}

/// <summary>
/// 以逗号Trim参数，返回Trim后的参数数量
/// </summary>
int ControllerClass::CommandTrim() {
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
/// 获取对应的PATH信息，将其格式化输出到this->WriteBuf中，返回NodeId或-1
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
int ControllerClass::GetFolderStat(TCHAR* Path) {
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
	//this->DirTree->UpdatedNodes.push_back(target->NodeId);
	swprintf_s(this->WriteBuf, 
		_T(
			"目录:%ls\n"
			"时间最早的文件：%ls  "
			"时间最晚的文件：%ls  "
			"目录下的文件总数：%d  "
			"目录下的文件大小总和：%lld (bytes)  \n"
		),
		target->PathName,oldest->PathName,newest->PathName,count,filesize);
	TCHAR txtFileName[MAX_PATHLEN];
	_stprintf_s(txtFileName, _T("folders_ver%d.txt"),this->DirTree->VersionControl);
	dumpFolderStat(txtFileName, this->WriteBuf);
	return target->NodeId;
}



/// <summary>
/// 将this->WriteBuf的内容写到WriteFp中，成功返回0
/// </summary>
/// <returns></returns>
int ControllerClass::WriteResult()
{
	_tfopen_s(&WriteFp, _T("result.txt"), _T("a,ccs=UTF-8"));
	//setlocale(LC_ALL, "");
	auto res = (EOF == fwrite(this->WriteBuf, sizeof(TCHAR), _tcslen(this->WriteBuf), this->WriteFp));
	fclose(WriteFp);
	return res;
}

int ControllerClass::Debug() {
	cmpDiff(_T("node397.txt"), _T("node397new.txt"), _T("AAA_RES.txt"));
	return 0;
}

/// <summary>
/// 对文件夹的子文件属性进行统计，输出到标准结果文件中
/// </summary>
/// <param name="PathOrigin"></param>
/// <param name="PathNew"></param>
/// <returns></returns>
int ControllerClass::CmpFolderDiff(TCHAR* PathOrigin, TCHAR* PathNew) {
	int diffCnt = 0, totalCnt = 0;
	FILE* fpOrigin, * fpNew;
	TCHAR bufOrigin[MAX_PATHLEN], bufNew[MAX_PATHLEN];
	TCHAR bufOriginAttr[MAX_PATHLEN], bufNewAttr[MAX_PATHLEN];
	_tfopen_s(&fpOrigin, PathOrigin, _T("r,ccs=UTF-8"));
	_tfopen_s(&fpNew, PathNew, _T("r,ccs=UTF-8"));
	if (!fpOrigin || !fpNew) {
		fpOrigin ? fclose(fpOrigin) : 0;
		fpNew ? fclose(fpNew) : 0;
		//打开文件失败
		return -1;
	}
	//逐行读取目录的统计信息并比较，找到不同的目录并输出更新的目录信息
	while (1) {
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);
		if (feof(fpOrigin)) {
			//原文件所有记录都比较完了，直接退出
			break;
		}
		totalCnt++;
		while (_tcscmp(bufOrigin, bufNew)&&!feof(fpOrigin)) {
			//目录有差异说明该目录被删除
			_stprintf_s(this->WriteBuf, _T("【目录更新#%d】\n"), diffCnt++);
			_stprintf_s(this->WriteBuf, _T("%ls==被删除：%ls"), this->WriteBuf, bufOrigin);
			this->WriteResult();
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
			if (feof(fpOrigin)) {
				//原文件所有记录都比较完了，直接退出
				break;
			}
			totalCnt++;
		}
		if (feof(fpOrigin) && feof(fpNew)&& _tcscmp(bufOrigin, bufNew)) {
			//两个文件同时到达文件尾，且目录不同，则说明当前bufOrigin内的目录是被删除的，直接跳出循环
			break;
		}
		if (_tcscmp(bufOriginAttr, bufNewAttr)) {
			//目录相同，属性不同
			_stprintf_s(this->WriteBuf, _T("【目录更新#%d】\n"), diffCnt++);
			_stprintf_s(this->WriteBuf, _T("%ls==目录名：%ls"), this->WriteBuf, bufOrigin);
			_stprintf_s(this->WriteBuf, _T("%ls==更新前属性：%ls"), this->WriteBuf, bufOriginAttr);
			_stprintf_s(this->WriteBuf, _T("%ls==更新后属性：%ls"), this->WriteBuf, bufNewAttr);
			this->WriteResult();
		}
	}
	_stprintf_s(this->WriteBuf, _T("比较了%d个目录,共有%d个目录更新\n"), totalCnt, diffCnt);
	this->WriteResult();
	fclose(fpOrigin);
	fclose(fpNew);
}

/// <summary>
/// 对目录及子目录的一系列信息进行统计，能够统计出文件的增删改,输出到新的结果文件中
/// </summary>
/// <param name="PathOrigin"></param>
/// <param name="PathNew"></param>
/// <returns></returns>
int ControllerClass::CmpFullDiff(TCHAR* PathOrigin, TCHAR* PathNew, TCHAR* OutputPath) {
	int diffCnt = 0, totalCnt = 0;
	FILE* fpOrigin, * fpNew,*fpResult;
	TCHAR bufOrigin[MAX_PATHLEN], bufNew[MAX_PATHLEN],bufResult[MAX_PATHLEN];
	TCHAR bufOriginAttr[MAX_PATHLEN], bufNewAttr[MAX_PATHLEN];
	_tfopen_s(&fpOrigin, PathOrigin, _T("r,ccs=UTF-8"));
	_tfopen_s(&fpNew, PathNew, _T("r,ccs=UTF-8"));
	_tfopen_s(&fpResult, OutputPath, _T("a,ccs=UTF-8"));
	if (!fpOrigin || !fpNew) {
		fpOrigin ? fclose(fpOrigin) : 0;
		fpNew ? fclose(fpNew) : 0;
		fpResult ? fclose(fpResult) : 0;
		//打开文件失败
		return -1;
	}
	//逐行读取目录的统计信息并比较，找到不同的目录并输出更新的目录信息
	while (1) {
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);
		if (feof(fpOrigin)) {
			//原文件所有记录都比较完了，直接退出
			break;
		}
		totalCnt++;
		while (_tcscmp(bufOrigin, bufNew) && !feof(fpOrigin)) {
			//目录有差异说明该目录被删除
			_stprintf_s(bufResult, _T("【节点更改#%d】\n"), diffCnt++);
			_stprintf_s(bufResult, _T("%ls==被删除：%ls"), bufResult, bufOrigin);
			fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
			if (feof(fpOrigin)) {
				//原文件所有记录都比较完了，直接退出
				break;
			}
			totalCnt++;
		}
		if (_tcscmp(bufOriginAttr, bufNewAttr)) {
			//目录相同，属性不同
			_stprintf_s(bufResult, _T("【目录更新#%d】\n"), diffCnt++);
			_stprintf_s(bufResult, _T("%ls==目录名：%ls"), bufResult, bufOrigin);
			_stprintf_s(bufResult, _T("%ls==更新前属性：%ls"), bufResult, bufOriginAttr);
			_stprintf_s(bufResult, _T("%ls==更新后属性：%ls"), bufResult, bufNewAttr);
			fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
		}
	}
	//反过来查找新增的节点
	FILE* tmp = fpOrigin;
	fpOrigin = fpNew;
	fpNew = tmp;
	while (1) {
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);
		if (feof(fpOrigin)) {
			//原文件所有记录都比较完了，直接退出
			break;
		}
		totalCnt++;
		while (_tcscmp(bufOrigin, bufNew) && !feof(fpOrigin)) {
			//目录有差异说明该目录被删除
			_stprintf_s(bufResult, _T("【节点更改#%d】\n"), diffCnt++);
			_stprintf_s(bufResult, _T("%ls==新增节点：%ls"), bufResult, bufOrigin);
			fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
			if (feof(fpOrigin)) {
				//原文件所有记录都比较完了，直接退出
				break;
			}
			totalCnt++;
		}
	}
	_stprintf_s(bufResult, _T("比较了%d个节点,共有%d个节点更新\n"), totalCnt, diffCnt);
	this->WriteResult();
	fclose(fpOrigin);
	fclose(fpNew);
	fclose(fpResult);
	return 0;
}