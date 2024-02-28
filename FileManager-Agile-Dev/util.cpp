#include"util.h"
#include<locale>
#include<stdio.h>



/// <summary>
/// 简单的忽略大小写的字符串比较
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
/// 比较两个扫描结果的差异，结果输出到PathResult路径的文件中，成功返回0
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
		//打开文件失败
		return -1;
	}
	//读取文件，查找被修改和被删除的数据
	//读取原始数据,奇数行只包含文件类型和文件名
	_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
	_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//读取详细信息
	// 对fpOrigin的每行记录，遍历fpNew，找到相同的记录
	while (!feof(fpOrigin)) {
		//读取新数据
		fsetpos(fpNew, &posNew);
		bool found = false;
		while (!feof(fpNew)) {
			_fgetts(bufNew, MAX_PATHLEN, fpNew);	
			_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//读取详细信息
			//auto res = _tcscmp(bufOrigin, bufNew);
			int cmp = _tcscmp(bufOrigin, bufNew);
			if (!cmp) {
				found = true;
				break;
			}
		}
		if (!found) {
			//没找到相同的记录
				_ftprintf(fpResult, _T("被删除:%ls\n"), bufOrigin);
		}
		//尽可能多移动指针
		do {
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//读取详细信息
			_fgetts(bufNew, MAX_PATHLEN, fpNew);
			_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//读取详细信息
		} while (!feof(fpOrigin) && !feof(fpNew) && !_tcscmp(bufOrigin, bufNew) && !_tcscmp(bufOrigin, bufNew));

	}
	//接下来查找新增的数据
	fsetpos(fpOrigin, &posOrigin);
	fsetpos(fpNew, &posNew);
	//读取原始数据,奇数行只包含文件类型和文件名
	_fgetts(bufNew, MAX_PATHLEN, fpNew);
	_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//读取详细信息
	// 对fpNew的每行记录，遍历fpOrigin，找到相同的记录
	while (!feof(fpNew)) {
		//读取新数据
		fsetpos(fpOrigin, &posOrigin);
		bool found = false;
		while (!feof(fpOrigin)) {
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//读取详细信息
			auto res = _tcscmp(bufNew, bufOrigin);
			int cmp = _tcscmp(bufNew, bufOrigin);
			if (!cmp) {
				found = true;
				break;
			}

		}
		if (!found) {
			//没找到相同的记录
			_ftprintf(fpResult, _T("新增:%ls\n"), bufNew);
		}
		//尽可能多移动指针
		do {
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//读取详细信息
			_fgetts(bufNew, MAX_PATHLEN, fpNew);
			_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//读取详细信息
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
		//打开文件失败
		return -1;
	}
	//逐行读取目录的统计信息并比较，找到不同的目录并输出更新的目录信息
	while (!feof(fpOrigin) && !feof(fpNew)) {
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		totalCnt++;
		if (_tcscmp(bufOrigin, bufNew)) {
			//找到不同的目录
			_stprintf_s(bufResult, _T("目录更新#%d\n"), diffCnt);
			_stprintf_s(bufResult, _T("%ls更新前：%ls\n"),bufResult, bufOrigin);
			_stprintf_s(bufResult, _T("%ls更新后：%ls\n"), bufResult,bufNew);
			fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
			diffCnt++;
		}
	}
	_stprintf_s(bufResult, _T("比较了%d个目录,共有%d个目录更新\n"), totalCnt, diffCnt);
	fwrite(bufResult, sizeof(TCHAR), _tcslen(bufResult), fpResult);
	fclose(fpOrigin);
	fclose(fpNew);
	fclose(fpResult);
}