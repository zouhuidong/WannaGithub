#include <stdio.h>
#include <string>
#include <time.h>
#include <io.h> 
#include <conio.h>
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
using namespace std;

// wstring和string转换函数需要用的头
#include <comutil.h>  
#pragma comment(lib, "comsuppw.lib")

wstring stow(const string& s)
{
	_bstr_t t = s.c_str();
	wchar_t* pwchar = (wchar_t*)t;
	wstring result = pwchar;
	return result;
}


/**
 * @brief		获取网页源码
 * @param[in]	Url 网页链接
 * @return		返回网页源码
 * @note		代码来自 https://www.cnblogs.com/croot/p/3391003.html （有删改）
*/
string GetWebSrcCode(LPCTSTR Url)
{
	string strHTML;
	HINTERNET hSession = InternetOpen(L"IE6.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession != NULL)
	{
		HINTERNET hURL = InternetOpenUrl(hSession, Url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
		if (hURL != NULL)
		{
			const int nBlockSize = 1024;
			char Temp[nBlockSize] = { 0 };
			ULONG Number = 1;

			while (Number > 0)
			{
				InternetReadFile(hURL, Temp, nBlockSize - 1, &Number);
				for (int i = 0; i < (int)Number; i++)
					strHTML += Temp[i];
			}

			InternetCloseHandle(hURL);
			hURL = NULL;
		}

		InternetCloseHandle(hSession);
		hSession = NULL;
	}

	return strHTML;
}

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
 * @brief		根据域名查询网站的信息获取 ip 地址
 * @param[in]	_strUrl: 查询某域名的网址，注意不能填要查的域名
 * @param[in]	_strBeginSymbol: ip 信息首次出现时的 HTML 关键字
 * @param[in]	_strEndSymbol: ip 信息结束时的 HTML 关键字
 * @param[out]	_strIP: 传出获取到的 ip 地址字符串
 * @return		返回是否成功获取 ip
*/
bool GetIPInfo(string _strUrl, string _strBeginSymbol, char _strEndSymbol, string* _strIp)
{
	string str = GetWebSrcCode(stow(_strUrl).c_str());

	// 获取 ip 地址
	int indexIp = str.find(_strBeginSymbol);
	if (indexIp < 0) return false;
	string strIp;
	for (int i = indexIp + _strBeginSymbol.size(); str[i] != _strEndSymbol; i++)
		strIp += str[i];
	*_strIp = strIp;

	return true;
}


/**
 * @brief 显示获取 ip 错误，然后退出程序
*/
void ShowGetIpError()
{
	printf("\n\n"
		"\n-------- 错误信息 --------\n"
		"\n获取 ip 失败。\n"
		"\n请检查网络然后重试。如果还遇到此问题，有可能是域名解析 api 已经过时或者程序已经过时，"
		"\n您可以选择访问 huidong.xyz 寻找新版或联系 huidong_mail@163.com 尝试解决此问题。\n"
		"\n按任意键即可退出程序");
	_getwch();
	exit(-1);
}

int main()
{
	printf("\n");
	printf("  ________________________________                                         \n"
		" /   ____                         \\                                       \n"
		" |  /       .     |         |     |      Wanna Github ~~  【Github 登陆器】 \n"
		" |  |  ___  | |_  |__  |  | |__   |      version 1.0   2021-8-9            \n"
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

		// 查询 ip 列表
		const int nQueryNum = 5;

		// Github 相关域名
		string strDomain[nQueryNum] = {
			"github.com",
			"github.global.ssl.fastly.net",
			"codeload.github.com",
			"assets-cdn.github.com",
			"api.github.com"
		};

		// 各个域名对应的 ip 查询地址
		string strQuery[nQueryNum] = {
			"https://github.com.ipaddress.com/",
			"https://fastly.net.ipaddress.com/github.global.ssl.fastly.net",
			"https://github.com.ipaddress.com/codeload.github.com",
			"https://github.com.ipaddress.com/assets-cdn.github.com",
			"https://github.com.ipaddress.com/api.github.com"
		};

		// ip 信息首次出现在页面中时的 HTML 关键字
		string strIpBeginSymbol[nQueryNum] = {
			"<ul class=\"comma-separated\"><li>",
			"<ul class=\"comma-separated\"><li>",
			"<ul class=\"comma-separated\"><li>",
			"<a href=\"https://www.ipaddress.com/ipv4/",
			"<ul class=\"comma-separated\"><li>"
		};

		// ip 信息结束的 HTML 关键字
		char strIpEndSymbol[nQueryNum] = { '<','<','<','\"','<' };

		// 存储查询到的 ip
		string strIp[nQueryNum];

		for (int i = 0; i < nQueryNum; i++)
			if (!GetIPInfo(strQuery[i], strIpBeginSymbol[i], strIpEndSymbol[i], &strIp[i]))
				ShowGetIpError();
			else
				printf("*");

		printf("\n查询域名 ip 完成，耗时 %.2f 秒		\n", (double)(clock() - t) / CLOCKS_PER_SEC);

		Sleep(500);
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
		fputs("\n", fp);
		fputs((strRecordBegin + "\n").c_str(), fp);
		for (int i = 0; i < nQueryNum; i++)
		{
			fprintf_s(fp, "%s %s\n", strIp[i].c_str(), strDomain[i].c_str());
		}
		fputs(strRecordEnd.c_str(), fp);
		fputs("\n", fp);

		fclose(fp);
		fp = NULL;

		printf("写入 hosts 文件完成");

		// 刷新 dns 缓存
		system("ipconfig /flushdns");

		printf("\n\n");
		printf("\n注：若失败，则稍后片刻，多试几次。\n");
		printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		printf("\n>>>>>>>>>>>> 按 [Enter] 一键进入 Github !!! <<<<<<<<<<<<");
		printf("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
		printf("\n\n");

		while (true)
		{
			char x = _getch();
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
