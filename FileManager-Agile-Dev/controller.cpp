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
	this->CommandTrim();//��ȡ�����еĸ������
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
		//�������
		return -1;
	}
	while (1) {
		fgetws(ReadBuf, MAX_PATHLEN * 3, ReadFp);
		int count = this->CommandTrim();
		if (!lowerCaseCmp(_T("end of dirs"),Args[0]) || !lowerCaseCmp(_T("end of files"),Args[0])) {
			//�����ļ�β
			break;
		}
		int tmpId = 0;
		switch (mode) {
			case MODE_MYSTAT:
				//�ļ���ͳ��
				this->Args[0][_tcslen(Args[0]) - 1] = _T('\0');//ȥ��ĩβ��\��
				tmpId = this->GetFolderStat(Args[0]);
				if (tmpId != -1) {
					this->FolderStatNodes.push_back(tmpId);
//					this->DirTree->UpdatedNodes.push_back(tmpId);
				}
				break;
			case MODE_MYDIR:
				//Ŀ¼�޸ģ�ֻ��ɾ��������
				Args[0][_tcslen(Args[0]) - 1] = _T('\0');//ȥ��ĩβ��\��
				if (!this->DirTree->AlterNode(Args[0],Args[1],0,0 )) {
					_stprintf_s(this->WriteBuf, _T("��ɾ��Ŀ¼:%ls\n"), Args[0]);
				}
				else {
					_stprintf_s(this->WriteBuf, _T("δ�ҵ�Ŀ¼:%ls\n"), Args[0]);
				}
				break;
			case MODE_MYFILE:
				//�ļ��޸ģ�ɾ�����޸Ĵ�С������������
				if (!this->DirTree->AlterNode(Args[0], Args[1], _ttoi(Args[2]), _ttoi(Args[3]))) {
					_stprintf_s(this->WriteBuf, _T("�Ѷ��ļ�%ls����[%ls]����\n"), Args[0],Args[1]);
				}
				else {
					_stprintf_s(this->WriteBuf, _T("�Ҳ����ļ�:%ls\n"), Args[0]);
				}
				break;
			default:
				return -1;
		}
		auto err = this->WriteResult();
	}
	//������Ϣͳ��
	//����UpdatedNodes
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
		//����FolderStatNodes,ֻ��Ƚ�Ŀ¼�������
		TCHAR txtFileNameOld[MAX_PATHLEN], txtFileNameNew[MAX_PATHLEN];
		_stprintf_s(txtFileNameOld, _T("folders_ver%d.txt"), 0);
		_stprintf_s(txtFileNameNew, _T("folders_ver%d.txt"), this->DirTree->VersionControl);
		for (int index = 0; index < this->FolderStatNodes.size(); index++) {
			this->DirTree->VersionControl++;//�������µİ汾��
			int NodeId = this->FolderStatNodes[index];
			Node* target = this->DirTree->GetNodeById(NodeId);
			if (!target) {
				continue;
			}
			else {
				this->GetFolderStat(target->PathName);
			}
			
			this->DirTree->VersionControl--;//�ָ��ɵİ汾��
		}
		this->CmpFolderDiff(txtFileNameOld, txtFileNameNew);

		//����FullDiffNodes,��Ҫ�Ƚ��������ӽڵ�Ĳ���
		TCHAR txtFileNameResult[MAX_PATHLEN];
		for (int index = 0; index < this->FullDiffNodes.size(); index++) {
			this->DirTree->VersionControl++;//�����µİ汾��
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
			this->DirTree->VersionControl--;//�ָ��ɵİ汾��
		}
		//�˴β������������°汾��
		this->DirTree->VersionControl++;
	}
	return 0;
}

/// <summary>
/// �Զ���Trim����������Trim��Ĳ�������
/// </summary>
int ControllerClass::CommandTrim() {
	TCHAR* head = ReadBuf;
	TCHAR* tail = head;
	int argIndex = 0;
	while (*tail != _T('\n') && !feof(ReadFp)) {
		if (*tail == _T(',')) {
			//���ֽ��
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
/// ��ȡ��Ӧ��PATH��Ϣ�������ʽ�������this->WriteBuf�У�����NodeId��-1
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
int ControllerClass::GetFolderStat(TCHAR* Path) {
	Node *oldest = NULL, *newest = NULL;
	INT64 filesize=0;
	int count = 0;
	
	Node *target = this->DirTree->GetNodeByPath(Path);
	if (!target) {
		//�Ҳ�����Ӧ��Ŀ¼
		_stprintf_s(this->WriteBuf, _T("No such directory:%s\n"), Path);
		return -1;
	}
	this->DirTree->GetDirectoryInfo(target, &oldest, &newest, &count, &filesize);
	//this->DirTree->UpdatedNodes.push_back(target->NodeId);
	swprintf_s(this->WriteBuf, 
		_T(
			"Ŀ¼:%ls\n"
			"ʱ��������ļ���%ls  "
			"ʱ��������ļ���%ls  "
			"Ŀ¼�µ��ļ�������%d  "
			"Ŀ¼�µ��ļ���С�ܺͣ�%lld (bytes)  \n"
		),
		target->PathName,oldest->PathName,newest->PathName,count,filesize);
	TCHAR txtFileName[MAX_PATHLEN];
	_stprintf_s(txtFileName, _T("folders_ver%d.txt"),this->DirTree->VersionControl);
	dumpFolderStat(txtFileName, this->WriteBuf);
	return target->NodeId;
}



/// <summary>
/// ��this->WriteBuf������д��WriteFp�У��ɹ�����0
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
/// ���ļ��е����ļ����Խ���ͳ�ƣ��������׼����ļ���
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
		//���ļ�ʧ��
		return -1;
	}
	//���ж�ȡĿ¼��ͳ����Ϣ���Ƚϣ��ҵ���ͬ��Ŀ¼��������µ�Ŀ¼��Ϣ
	while (1) {
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);
		if (feof(fpOrigin)) {
			//ԭ�ļ����м�¼���Ƚ����ˣ�ֱ���˳�
			break;
		}
		totalCnt++;
		while (_tcscmp(bufOrigin, bufNew)&&!feof(fpOrigin)) {
			//Ŀ¼�в���˵����Ŀ¼��ɾ��
			_stprintf_s(this->WriteBuf, _T("��Ŀ¼����#%d��\n"), diffCnt++);
			_stprintf_s(this->WriteBuf, _T("%ls==��ɾ����%ls"), this->WriteBuf, bufOrigin);
			this->WriteResult();
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
			if (feof(fpOrigin)) {
				//ԭ�ļ����м�¼���Ƚ����ˣ�ֱ���˳�
				break;
			}
			totalCnt++;
		}
		if (feof(fpOrigin) && feof(fpNew)&& _tcscmp(bufOrigin, bufNew)) {
			//�����ļ�ͬʱ�����ļ�β����Ŀ¼��ͬ����˵����ǰbufOrigin�ڵ�Ŀ¼�Ǳ�ɾ���ģ�ֱ������ѭ��
			break;
		}
		if (_tcscmp(bufOriginAttr, bufNewAttr)) {
			//Ŀ¼��ͬ�����Բ�ͬ
			_stprintf_s(this->WriteBuf, _T("��Ŀ¼����#%d��\n"), diffCnt++);
			_stprintf_s(this->WriteBuf, _T("%ls==Ŀ¼����%ls"), this->WriteBuf, bufOrigin);
			_stprintf_s(this->WriteBuf, _T("%ls==����ǰ���ԣ�%ls"), this->WriteBuf, bufOriginAttr);
			_stprintf_s(this->WriteBuf, _T("%ls==���º����ԣ�%ls"), this->WriteBuf, bufNewAttr);
			this->WriteResult();
		}
	}
	_stprintf_s(this->WriteBuf, _T("�Ƚ���%d��Ŀ¼,����%d��Ŀ¼����\n"), totalCnt, diffCnt);
	this->WriteResult();
	fclose(fpOrigin);
	fclose(fpNew);
}

/// <summary>
/// ��Ŀ¼����Ŀ¼��һϵ����Ϣ����ͳ�ƣ��ܹ�ͳ�Ƴ��ļ�����ɾ��,������µĽ���ļ���
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
		//���ļ�ʧ��
		return -1;
	}
	//���ж�ȡĿ¼��ͳ����Ϣ���Ƚϣ��ҵ���ͬ��Ŀ¼��������µ�Ŀ¼��Ϣ
	while (1) {
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);
		if (feof(fpOrigin)) {
			//ԭ�ļ����м�¼���Ƚ����ˣ�ֱ���˳�
			break;
		}
		totalCnt++;
		while (_tcscmp(bufOrigin, bufNew) && !feof(fpOrigin)) {
			//Ŀ¼�в���˵����Ŀ¼��ɾ��
			_stprintf_s(bufResult, _T("���ڵ����#%d��\n"), diffCnt++);
			_stprintf_s(bufResult, _T("%ls==��ɾ����%ls"), bufResult, bufOrigin);
			fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
			if (feof(fpOrigin)) {
				//ԭ�ļ����м�¼���Ƚ����ˣ�ֱ���˳�
				break;
			}
			totalCnt++;
		}
		if (_tcscmp(bufOriginAttr, bufNewAttr)) {
			//Ŀ¼��ͬ�����Բ�ͬ
			_stprintf_s(bufResult, _T("��Ŀ¼����#%d��\n"), diffCnt++);
			_stprintf_s(bufResult, _T("%ls==Ŀ¼����%ls"), bufResult, bufOrigin);
			_stprintf_s(bufResult, _T("%ls==����ǰ���ԣ�%ls"), bufResult, bufOriginAttr);
			_stprintf_s(bufResult, _T("%ls==���º����ԣ�%ls"), bufResult, bufNewAttr);
			fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
		}
	}
	//���������������Ľڵ�
	FILE* tmp = fpOrigin;
	fpOrigin = fpNew;
	fpNew = tmp;
	while (1) {
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);
		if (feof(fpOrigin)) {
			//ԭ�ļ����м�¼���Ƚ����ˣ�ֱ���˳�
			break;
		}
		totalCnt++;
		while (_tcscmp(bufOrigin, bufNew) && !feof(fpOrigin)) {
			//Ŀ¼�в���˵����Ŀ¼��ɾ��
			_stprintf_s(bufResult, _T("���ڵ����#%d��\n"), diffCnt++);
			_stprintf_s(bufResult, _T("%ls==�����ڵ㣺%ls"), bufResult, bufOrigin);
			fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
			if (feof(fpOrigin)) {
				//ԭ�ļ����м�¼���Ƚ����ˣ�ֱ���˳�
				break;
			}
			totalCnt++;
		}
	}
	_stprintf_s(bufResult, _T("�Ƚ���%d���ڵ�,����%d���ڵ����\n"), totalCnt, diffCnt);
	this->WriteResult();
	fclose(fpOrigin);
	fclose(fpNew);
	fclose(fpResult);
	return 0;
}