#include"util.h"
#include<locale>
#include<stdio.h>



/// <summary>
/// �򵥵ĺ��Դ�Сд���ַ����Ƚ�
/// </summary>
/// <param name="str1"></param>
/// <param name="str2"></param>
/// <returns></returns>
int lowerCaseCmp(TCHAR* str1, TCHAR* str2) {
	setlocale(LC_ALL, "");
	TCHAR p1[MAX_PATHLEN], p2[MAX_PATHLEN];
	_tcscpy_s(p1, str1);
	_tcscpy_s(p2, str2);
	_tcslwr_s(p1);
	_tcslwr_s(p2);
	return _tcscmp(p1,p2);
}

TCHAR* lowerCaseContain(TCHAR* str1, TCHAR* str2) {
	setlocale(LC_ALL, "");
	TCHAR p1[MAX_PATHLEN], p2[MAX_PATHLEN];
	_tcscpy_s(p1, str1);
	_tcscpy_s(p2, str2);
	_tcslwr_s(p1);
	_tcslwr_s(p2);
	return _tcsstr(p1, p2);
}

/// <summary>
/// �Ƚ�����ɨ�����Ĳ��죬��������PathResult·�����ļ��У��ɹ�����0
/// </summary>
/// <param name="PathOrigin"></param>
/// <param name="PathNew"></param>
/// <param name="Pathresult"></param>
/// <returns></returns>
int cmpDiff(TCHAR* PathOrigin, TCHAR* PathNew, TCHAR* Pathresult) {
	//setlocale(LC_ALL, "");
	FILE *fpOrigin, *fpNew, *fpResult;
	fpos_t posOrigin, posNew;
	TCHAR bufOrigin[MAX_PATHLEN], bufNew[MAX_PATHLEN],bufResult[MAX_PATHLEN];
	TCHAR bufOriginAttr[MAX_PATHLEN], bufNewAttr[MAX_PATHLEN];
	_tfopen_s(&fpOrigin,PathOrigin, _T("r,ccs=UTF-8"));
	_tfopen_s(&fpNew,PathNew, _T("r,ccs=UTF-8"));
	_tfopen_s(&fpResult,Pathresult, _T("a,ccs=UTF-8"));
	fgetpos(fpOrigin, &posOrigin);
	fgetpos(fpNew, &posNew);

	if (!fpOrigin || !fpNew || !fpResult) {
		//���ļ�ʧ��
		return -1;
	}
	//��ȡ�ļ������ұ��޸ĺͱ�ɾ��������
	//��ȡԭʼ����,������ֻ�����ļ����ͺ��ļ���
	_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
	_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//��ȡ��ϸ��Ϣ
	// ��fpOrigin��ÿ�м�¼������fpNew���ҵ���ͬ�ļ�¼
	while (!feof(fpOrigin)) {
		//��ȡ������
		fsetpos(fpNew, &posNew);
		bool found = false;
		while (!feof(fpNew)) {
			_fgetts(bufNew, MAX_PATHLEN, fpNew);	
			_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//��ȡ��ϸ��Ϣ
			//auto res = _tcscmp(bufOrigin, bufNew);
			int cmp = _tcscmp(bufOrigin, bufNew);
			if (!cmp) {
				found = true;
				break;
			}
		}
		if (!found) {
			//û�ҵ���ͬ�ļ�¼
				_ftprintf(fpResult, _T("��ɾ��:%ls\n"), bufOrigin);
		}
		//�����ܶ��ƶ�ָ��
		do {
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//��ȡ��ϸ��Ϣ
			_fgetts(bufNew, MAX_PATHLEN, fpNew);
			_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//��ȡ��ϸ��Ϣ
		} while (!feof(fpOrigin) && !feof(fpNew) && !_tcscmp(bufOrigin, bufNew) && !_tcscmp(bufOrigin, bufNew));

	}
	//��������������������
	fsetpos(fpOrigin, &posOrigin);
	fsetpos(fpNew, &posNew);
	//��ȡԭʼ����,������ֻ�����ļ����ͺ��ļ���
	_fgetts(bufNew, MAX_PATHLEN, fpNew);
	_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//��ȡ��ϸ��Ϣ
	// ��fpNew��ÿ�м�¼������fpOrigin���ҵ���ͬ�ļ�¼
	while (!feof(fpNew)) {
		//��ȡ������
		fsetpos(fpOrigin, &posOrigin);
		bool found = false;
		while (!feof(fpOrigin)) {
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//��ȡ��ϸ��Ϣ
			auto res = _tcscmp(bufNew, bufOrigin);
			int cmp = _tcscmp(bufNew, bufOrigin);
			if (!cmp) {
				found = true;
				break;
			}

		}
		if (!found) {
			//û�ҵ���ͬ�ļ�¼
			_ftprintf(fpResult, _T("����:%ls\n"), bufNew);
		}
		//�����ܶ��ƶ�ָ��
		do {
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//��ȡ��ϸ��Ϣ
			_fgetts(bufNew, MAX_PATHLEN, fpNew);
			_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//��ȡ��ϸ��Ϣ
		} while (!feof(fpOrigin) && !feof(fpNew) && !_tcscmp(bufOrigin, bufNew) && !_tcscmp(bufOrigin, bufNew));
	}
	fclose(fpOrigin);
	fclose(fpNew);
	fclose(fpResult);
	return 0;
}

int dumpFolderStat(TCHAR* txtFileName, TCHAR* buf) {
	FILE* dumperFp = NULL;
	_tfopen_s(&dumperFp, txtFileName, _T("a,ccs=UTF-8"));
	fwrite(buf, sizeof(TCHAR), _tcslen(buf), dumperFp);
	fclose(dumperFp);
	return 0;
}

int cmpFolderDiff(TCHAR* PathOrigin, TCHAR* PathNew, TCHAR* Pathresult) {
	int diffCnt = 0,totalCnt=0;
	FILE* fpOrigin, * fpNew, * fpResult;
	TCHAR bufOrigin[MAX_PATHLEN], bufNew[MAX_PATHLEN], bufResult[MAX_PATHLEN];
	_tfopen_s(&fpOrigin, PathOrigin, _T("r,ccs=UTF-8"));
	_tfopen_s(&fpNew, PathNew, _T("r,ccs=UTF-8"));
	_tfopen_s(&fpResult, Pathresult, _T("a,ccs=UTF-8"));
	if (!fpOrigin || !fpNew || !fpResult) {
		//���ļ�ʧ��
		return -1;
	}
	//���ж�ȡĿ¼��ͳ����Ϣ���Ƚϣ��ҵ���ͬ��Ŀ¼��������µ�Ŀ¼��Ϣ
	while (!feof(fpOrigin) && !feof(fpNew)) {
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		totalCnt++;
		if (_tcscmp(bufOrigin, bufNew)) {
			//�ҵ���ͬ��Ŀ¼
			_stprintf_s(bufResult, _T("Ŀ¼����#%d\n"), diffCnt);
			_stprintf_s(bufResult, _T("%ls����ǰ��%ls\n"),bufResult, bufOrigin);
			_stprintf_s(bufResult, _T("%ls���º�%ls\n"), bufResult,bufNew);
			fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
			diffCnt++;
		}
	}
	_stprintf_s(bufResult, _T("�Ƚ���%d��Ŀ¼,����%d��Ŀ¼����\n"), totalCnt, diffCnt);
	fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
	fclose(fpOrigin);
	fclose(fpNew);
	fclose(fpResult);
}