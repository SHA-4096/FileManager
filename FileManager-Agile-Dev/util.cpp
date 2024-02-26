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
	setlocale(LC_ALL, "");
	FILE *fpOrigin, *fpNew, *fpResult;
	TCHAR bufOrigin[MAX_PATHLEN], bufNew[MAX_PATHLEN],bufResult[MAX_PATHLEN];
	TCHAR bufOriginAttr[MAX_PATHLEN], bufNewAttr[MAX_PATHLEN];
	_tfopen_s(&fpOrigin,PathOrigin, _T("r,ccs=UNICODE"));
	_tfopen_s(&fpNew,PathNew, _T("r,ccs=UNICODE"));
	_tfopen_s(&fpResult,Pathresult, _T("a,ccs=UNICODE"));
	if (!fpOrigin || !fpNew || !fpResult) {
		//���ļ�ʧ��
		return -1;
	}
	//��ȡ�ļ������ұ��޸ĺͱ�ɾ��������
	// ��fpOrigin��ÿ�м�¼������fpNew���ҵ���ͬ�ļ�¼
	//���ȸ�λfpOrigin
	while (!feof(fpOrigin)) {
		//��ȡԭʼ����,������ֻ�����ļ����ͺ��ļ���
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//��ȡ��ϸ��Ϣ
		//��ȡ������
		fseek(fpNew, sizeof(TCHAR), SEEK_SET);
		while (!feof(fpNew)) {
			_fgetts(bufNew, MAX_PATHLEN, fpNew);
			_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//��ȡ��ϸ��Ϣ
			auto res = _tcscmp(bufOrigin, bufNew);
			if (!_tcscmp(bufOrigin, bufNew)) {
				//�ҵ���ͬ�ļ�¼,�Ƚ���ϸ��Ϣ
				if (_tcscmp(bufOriginAttr, bufNewAttr)) {
					//�ļ����Բ�ͬ
					_ftprintf(fpResult, _T("���޸�:%ls ���ԸĶ����£�\n"), bufOrigin);
					_ftprintf(fpResult, _T("---ԭʼ����:%ls\n"), bufOriginAttr);
					_ftprintf(fpResult, _T("---������:%ls\n"), bufNewAttr);
				}
				break;
			}

		}
		if (feof(fpNew)) {
			//û�ҵ���ͬ�ļ�¼
			_ftprintf(fpResult, _T("��ɾ��:%ls\n"), bufOrigin);
		}
	}
	//��������������������
	rewind(fpNew);
	rewind(fpOrigin);
	while (!feof(fpNew)) {
		//��ȡԭʼ����,������ֻ�����ļ����ͺ��ļ���
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//��ȡ��ϸ��Ϣ
		//��ȡ������
		rewind(fpOrigin);
		while (!feof(fpOrigin)) {
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
			if (!_tcscmp(bufOrigin, bufNew)) {
				//�ҵ���ͬ�ļ�¼,����
				break;
			}
		}

		if (feof(fpOrigin)) {
			//û�ҵ���ͬ�ļ�¼
			_ftprintf(fpResult, _T("����:%ls���������£�\n"), bufOrigin);
			_ftprintf(fpResult, _T("---����:%ls\n"), bufNewAttr);

		}
	}
	fclose(fpOrigin);
	fclose(fpNew);
	fclose(fpResult);
	return 0;
}