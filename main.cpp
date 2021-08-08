#include <stdio.h>
#include <Windows.h>
#include <string>
#include <time.h>
#include <io.h> 
#include <conio.h> 
#include "httputil.h"
using namespace std;

BOOL IsRunAsAdministrator()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:

	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

/**
 * @brief		程序获取管理员权限
 * @param[in]	Param: 程序的运行参数
 * @param[in]	Showcmd: 程序的显示方式
 *
 * @return		0 - 获取管理员权限失败
 *				1 - 获取管理员权限成功
 *				2 - 已有管理员权限
*/
short GetWindowsAdmin(LPCTSTR Param = L"", int Showcmd = SW_SHOWDEFAULT)
{
	if (IsRunAsAdministrator()) return 2;
	TCHAR Path[MAX_PATH];
	ZeroMemory(Path, MAX_PATH);
	::GetModuleFileName(NULL, Path, MAX_PATH);
	HINSTANCE res;
	res = ShellExecute(NULL, L"runas", Path, Param, NULL, Showcmd);
	if ((int)res > 32) return 1;
	else return 0;
}

/**
 * @brief		根据域名获取 ip 地址
 * @param[in]	_strDomain: 原域名
 * @param[out]	_strIP: 传出获取到的 ip 地址字符串
 * @return		返回是否成功获取 ip
*/
bool GetIPFromDomain(string _strDomain, string* _strIp)
{
	// 当前 ip 查询 api 官网：https://ip-api.com/
	char* resData = sendRequest(("http://ip-api.com/xml/" + _strDomain).c_str());
	string str = resData;
	delete[] resData;

	// xml 解析

	// 获取状态
	string strLabel = "<status>";
	int indexStatus = str.find(strLabel);
	if (indexStatus < 0) return false;
	string strStatus;
	for (int i = indexStatus + strLabel.size(); str[i] != '<'; i++)
		strStatus += str[i];
	if (strStatus != "success")	// 获取失败
		return false;

	// 获取 ip 地址
	strLabel = "<query>";
	int indexIp = str.find(strLabel, indexStatus);
	if (indexIp < 0) return false;
	string strIp;
	for (int i = indexIp + strLabel.size(); str[i] != '<'; i++)
		strIp += str[i];
	*_strIp = strIp;

	return true;
}

/**
 * @brief 显示获取 ip 错误，然后退出程序
*/
void ShowGetIpError()
{
	printf("\n获取 ip 失败，请检查网络然后重试。如果还遇到此问题，有可能是域名解析 api 已经过时或者程序已经过时，"
		"\n您可以选择访问 huidong.xyz 寻找新版或联系 huidong_mail@163.com 尝试解决此问题。\n"
		"\n按任意键即可退出程序");
	_getwch();
	exit(-1);
}

int main()
{
	printf("\n");
	printf(	"  ________________________________                                         \n"
			" /   ____                         \\                                       \n"
			" |  /       .     |         |     |      Wanna Github ~~  【Github 登陆器】 \n"
			" |  |  ___  | |_  |__  |  | |__   |      version 0.1(alpha)   2021-8-8     \n"
			" |  |____|  | |__ |  | |__| |__|  |      huidong<huidong_mail@163.com>     \n"
			" \\________________________________/      web: huidong.xyz                  \n");
	printf("\n\n");
	printf("------------ 注意事项 -----------\n\n");
	printf("第一次使用此工具时请将 hosts 文件中的和 github 有关的内容删除，否则可能无法更新 dns\n");
	printf("\n\n");
	printf("------------ 操作记录 ------------\n");

	int r = GetWindowsAdmin();
	if (r == 0)
	{
		MessageBox(GetConsoleWindow(), L"获取管理员权限失败，程序写入 hosts 文件必须此权限。", L"wanna github", MB_OK);
	}

	// 已有管理员权限
	else if (r == 2)
	{
		printf("开始查询 Github 各域名 ip，请稍后……\r");

		// 记录域名 ip 查询所花时间
		int t = clock();

		// 解析域名列表
		const int nDomainNum = 3;
		string strDomain[nDomainNum] = { "github.com" ,"github.global.ssl.fastly.net" ,"codeload.Github.com" };
		string strIp[nDomainNum];

		for (int i = 0; i < nDomainNum; i++)
			if (!GetIPFromDomain(strDomain[i], &strIp[i]))
				ShowGetIpError();

		printf("查询域名 ip 完成，耗时 %.2f 秒		\r", (double)(clock() - t) / CLOCKS_PER_SEC);

		Sleep(300);
		printf("开始写入 hosts 文件					\r");

		FILE* fp = NULL;
		int nFileCode = fopen_s(&fp, "C:\\Windows\\System32\\drivers\\etc\\hosts", "rt+");
		if (nFileCode != 0)
		{
			printf("无法打开 hosts 文件，fopen_s 错误码：%d\n", nFileCode);
			printf("按任意键退出程序\n");
			_getwch();
			return -1;
		}

		char* pchHosts = new char[_filelength(_fileno(fp)) + 1];
		memset(pchHosts, 0, _filelength(_fileno(fp)) + 1);
		fread_s(pchHosts, _filelength(_fileno(fp)), _filelength(_fileno(fp)), 1, fp);
		string strHosts = pchHosts;
		delete[] pchHosts;

		// 写在 hosts 文件中的 Github 内容块的起始和结束的标记
		string strRecordBegin = "# Github (hd)";
		string strRecordEnd = "# Github end";

		// 检查原先是否已有记录，如果有，擦除原先的记录
		int nBeginIndex = strHosts.find(strRecordBegin);
		if (nBeginIndex != -1)
		{
			int nEndIndex = strHosts.find(strRecordEnd);
			if (nEndIndex != -1)
			{
				// 因为结束标记也要擦除，所以需要加上结束标记的长度
				nEndIndex += strRecordEnd.size();
				string strNewHosts;
				for (int i = 0; i < (int)strHosts.size(); i++)
				{
					if (!(i >= nBeginIndex && i <= nEndIndex))
					{
						strNewHosts += strHosts[i];
					}
				}

				// 以 "w+" 方式重新打开文件以清除数据
				fclose(fp);
				nFileCode = fopen_s(&fp, "C:\\Windows\\System32\\drivers\\etc\\hosts", "w+");
				if (nFileCode != 0)
				{
					printf("无法打开 hosts 文件，fopen_s 错误码：%d\n", nFileCode);
					printf("按任意键退出程序\n");
					_getwch();
					return -1;
				}

				fputs(strNewHosts.c_str(), fp);
			}
		}

		// 写入新记录
		fputs((strRecordBegin + "\n").c_str(), fp);
		for (int i = 0; i < nDomainNum; i++)
		{
			fprintf_s(fp, "%s %s\n", strIp[i].c_str(), strDomain[i].c_str());
		}
		fputs(strRecordEnd.c_str(), fp);

		fclose(fp);
		fp = NULL;

		printf("写入 hosts 文件完成");

		// 刷新 dns 缓存
		system("ipconfig /flushdns");

		printf("\n\n");
		printf(">>>>>>>>>>>> 按 [Enter] 一键进入 Github，开始冲刺 <<<<<<<<<<<<\n");

		while (true)
		{
			char x= _getch();
			if (x == '\r' || x == '\n')
			{
				system("start http://github.com");
				break;
			}

			Sleep(100);
		}
	}

	return 0;
}
