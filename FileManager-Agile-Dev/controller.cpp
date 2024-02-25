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
		switch (mode) {
			case MODE_MYSTAT:
				//�ļ���ͳ��
				this->Args[0][_tcslen(Args[0]) - 1] = _T('\0');//ȥ��ĩβ��\��
				this->GetFolderStat(Args[0]);
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

	return 0;
}

/// <summary>
/// �Զ���Trim����������Trim��Ĳ�������
/// </summary>
/// <returns></returns>
int CommandReader::CommandTrim() {
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
/// ��ȡ��Ӧ��PATH��Ϣ�������ʽ�������this->WriteBuf��
/// </summary>
/// <param name="Path"></param>
/// <returns></returns>
int CommandReader::GetFolderStat(TCHAR* Path) {
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
	swprintf_s(this->WriteBuf, 
		_T(
			"===================================\n"
			"Ŀ¼:%ls\n"
			"ʱ��������ļ���%ls\n"
			"ʱ��������ļ���%ls\n"
			"Ŀ¼�µ��ļ�������%d\n"
			"Ŀ¼�µ��ļ���С�ܺͣ�%lld (bytes)\n"
		),
		Path,oldest->PathName,newest->PathName,count,filesize);
	return 0;
}

/// <summary>
/// ��this->WriteBuf������д��WriteFp�У��ɹ�����0
/// </summary>
/// <returns></returns>
int CommandReader::WriteResult()
{
	setlocale(LC_ALL, "");
	return EOF == fwrite(this->WriteBuf, sizeof(TCHAR), _tcslen(this->WriteBuf), this->WriteFp);
}