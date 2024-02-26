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
	setlocale(LC_ALL, "");
	FILE *fpOrigin, *fpNew, *fpResult;
	TCHAR bufOrigin[MAX_PATHLEN], bufNew[MAX_PATHLEN],bufResult[MAX_PATHLEN];
	TCHAR bufOriginAttr[MAX_PATHLEN], bufNewAttr[MAX_PATHLEN];
	_tfopen_s(&fpOrigin,PathOrigin, _T("r,ccs=UNICODE"));
	_tfopen_s(&fpNew,PathNew, _T("r,ccs=UNICODE"));
	_tfopen_s(&fpResult,Pathresult, _T("a,ccs=UNICODE"));
	if (!fpOrigin || !fpNew || !fpResult) {
		//打开文件失败
		return -1;
	}
	//读取文件，查找被修改和被删除的数据
	// 对fpOrigin的每行记录，遍历fpNew，找到相同的记录
	//首先复位fpOrigin
	while (!feof(fpOrigin)) {
		//读取原始数据,奇数行只包含文件类型和文件名
		_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
		_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);//读取详细信息
		//读取新数据
		fseek(fpNew, sizeof(TCHAR), SEEK_SET);
		while (!feof(fpNew)) {
			_fgetts(bufNew, MAX_PATHLEN, fpNew);
			_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//读取详细信息
			auto res = _tcscmp(bufOrigin, bufNew);
			if (!_tcscmp(bufOrigin, bufNew)) {
				//找到相同的记录,比较详细信息
				if (_tcscmp(bufOriginAttr, bufNewAttr)) {
					//文件属性不同
					_ftprintf(fpResult, _T("被修改:%ls 属性改动如下：\n"), bufOrigin);
					_ftprintf(fpResult, _T("---原始属性:%ls\n"), bufOriginAttr);
					_ftprintf(fpResult, _T("---新属性:%ls\n"), bufNewAttr);
				}
				break;
			}

		}
		if (feof(fpNew)) {
			//没找到相同的记录
			_ftprintf(fpResult, _T("被删除:%ls\n"), bufOrigin);
		}
	}
	//接下来查找新增的数据
	rewind(fpNew);
	rewind(fpOrigin);
	while (!feof(fpNew)) {
		//读取原始数据,奇数行只包含文件类型和文件名
		_fgetts(bufNew, MAX_PATHLEN, fpNew);
		_fgetts(bufNewAttr, MAX_PATHLEN, fpNew);//读取详细信息
		//读取新数据
		rewind(fpOrigin);
		while (!feof(fpOrigin)) {
			_fgetts(bufOrigin, MAX_PATHLEN, fpOrigin);
			_fgetts(bufOriginAttr, MAX_PATHLEN, fpOrigin);
			if (!_tcscmp(bufOrigin, bufNew)) {
				//找到相同的记录,跳出
				break;
			}
		}

		if (feof(fpOrigin)) {
			//没找到相同的记录
			_ftprintf(fpResult, _T("新增:%ls，属性如下：\n"), bufOrigin);
			_ftprintf(fpResult, _T("---属性:%ls\n"), bufNewAttr);

		}
	}
	fclose(fpOrigin);
	fclose(fpNew);
	fclose(fpResult);
	return 0;
}