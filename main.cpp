#include <stdio.h>
#include <string>
#include <time.h>
#include <io.h> 
#include <conio.h>
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
using namespace std;

// wstring��stringת��������Ҫ�õ�ͷ
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
 * @brief		��ȡ��ҳԴ��
 * @param[in]	Url ��ҳ����
 * @return		������ҳԴ��
 * @note		�������� https://www.cnblogs.com/croot/p/3391003.html ����ɾ�ģ�
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
 * @brief		�����ȡ����ԱȨ��
 * @param[in]	Param: ��������в���
 * @param[in]	Showcmd: �������ʾ��ʽ
 *
 * @return		0 - ��ȡ����ԱȨ��ʧ��
 *				1 - ��ȡ����ԱȨ�޳ɹ�
 *				2 - ���й���ԱȨ��
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
 * @brief		����������ѯ��վ����Ϣ��ȡ ip ��ַ
 * @param[in]	_strUrl: ��ѯĳ��������ַ��ע�ⲻ����Ҫ�������
 * @param[in]	_strBeginSymbol: ip ��Ϣ�״γ���ʱ�� HTML �ؼ���
 * @param[in]	_strEndSymbol: ip ��Ϣ����ʱ�� HTML �ؼ���
 * @param[out]	_strIP: ������ȡ���� ip ��ַ�ַ���
 * @return		�����Ƿ�ɹ���ȡ ip
*/
bool GetIPInfo(string _strUrl, string _strBeginSymbol, char _strEndSymbol, string* _strIp)
{
	string str = GetWebSrcCode(stow(_strUrl).c_str());

	// ��ȡ ip ��ַ
	int indexIp = str.find(_strBeginSymbol);
	if (indexIp < 0) return false;
	string strIp;
	for (int i = indexIp + _strBeginSymbol.size(); str[i] != _strEndSymbol; i++)
		strIp += str[i];
	*_strIp = strIp;

	return true;
}


/**
 * @brief ��ʾ��ȡ ip ����Ȼ���˳�����
*/
void ShowGetIpError()
{
	printf("\n\n"
		"\n-------- ������Ϣ --------\n"
		"\n��ȡ ip ʧ�ܡ�\n"
		"\n��������Ȼ�����ԡ���������������⣬�п������������� api �Ѿ���ʱ���߳����Ѿ���ʱ��"
		"\n������ѡ����� huidong.xyz Ѱ���°����ϵ huidong_mail@163.com ���Խ�������⡣\n"
		"\n������������˳�����");
	_getwch();
	exit(-1);
}

int main()
{
	printf("\n");
	printf("  ________________________________                                         \n"
		" /   ____                         \\                                       \n"
		" |  /       .     |         |     |      Wanna Github ~~  ��Github ��½���� \n"
		" |  |  ___  | |_  |__  |  | |__   |      version 1.0   2021-8-9            \n"
		" |  |____|  | |__ |  | |__| |__|  |      huidong<huidong_mail@163.com>     \n"
		" \\________________________________/      web: huidong.xyz                  \n");
	printf("\n\n");
	printf("------------ ע������ -----------\n\n");
	printf("��һ��ʹ�ô˹���ʱ�뽫 hosts �ļ��еĺ� github �йص�����ɾ������������޷����� dns\n");
	printf("\n\n");
	printf("------------ ������¼ ------------\n");

	int r = GetWindowsAdmin();
	if (r == 0)
	{
		MessageBox(GetConsoleWindow(), L"��ȡ����ԱȨ��ʧ�ܣ�����д�� hosts �ļ������Ȩ�ޡ�", L"wanna github", MB_OK);
	}

	// ���й���ԱȨ��
	else if (r == 2)
	{
		printf("��ʼ��ѯ Github ������ ip�����Ժ󡭡�\r");

		// ��¼���� ip ��ѯ����ʱ��
		int t = clock();

		// ��ѯ ip �б�
		const int nQueryNum = 5;

		// Github �������
		string strDomain[nQueryNum] = {
			"github.com",
			"github.global.ssl.fastly.net",
			"codeload.github.com",
			"assets-cdn.github.com",
			"api.github.com"
		};

		// ����������Ӧ�� ip ��ѯ��ַ
		string strQuery[nQueryNum] = {
			"https://github.com.ipaddress.com/",
			"https://fastly.net.ipaddress.com/github.global.ssl.fastly.net",
			"https://github.com.ipaddress.com/codeload.github.com",
			"https://github.com.ipaddress.com/assets-cdn.github.com",
			"https://github.com.ipaddress.com/api.github.com"
		};

		// ip ��Ϣ�״γ�����ҳ����ʱ�� HTML �ؼ���
		string strIpBeginSymbol[nQueryNum] = {
			"<ul class=\"comma-separated\"><li>",
			"<ul class=\"comma-separated\"><li>",
			"<ul class=\"comma-separated\"><li>",
			"<a href=\"https://www.ipaddress.com/ipv4/",
			"<ul class=\"comma-separated\"><li>"
		};

		// ip ��Ϣ������ HTML �ؼ���
		char strIpEndSymbol[nQueryNum] = { '<','<','<','\"','<' };

		// �洢��ѯ���� ip
		string strIp[nQueryNum];

		for (int i = 0; i < nQueryNum; i++)
			if (!GetIPInfo(strQuery[i], strIpBeginSymbol[i], strIpEndSymbol[i], &strIp[i]))
				ShowGetIpError();
			else
				printf("*");

		printf("\n��ѯ���� ip ��ɣ���ʱ %.2f ��		\n", (double)(clock() - t) / CLOCKS_PER_SEC);

		Sleep(500);
		printf("��ʼд�� hosts �ļ�					\r");

		FILE* fp = NULL;
		int nFileCode = fopen_s(&fp, "C:\\Windows\\System32\\drivers\\etc\\hosts", "rt+");
		if (nFileCode != 0)
		{
			printf("�޷��� hosts �ļ���fopen_s �����룺%d\n", nFileCode);
			printf("��������˳�����\n");
			_getwch();
			return -1;
		}

		char* pchHosts = new char[_filelength(_fileno(fp)) + 1];
		memset(pchHosts, 0, _filelength(_fileno(fp)) + 1);
		fread_s(pchHosts, _filelength(_fileno(fp)), _filelength(_fileno(fp)), 1, fp);
		string strHosts = pchHosts;
		delete[] pchHosts;

		// д�� hosts �ļ��е� Github ���ݿ����ʼ�ͽ����ı��
		string strRecordBegin = "# Github (hd)";
		string strRecordEnd = "# Github end";

		// ���ԭ���Ƿ����м�¼������У�����ԭ�ȵļ�¼
		int nBeginIndex = strHosts.find(strRecordBegin);
		if (nBeginIndex != -1)
		{
			int nEndIndex = strHosts.find(strRecordEnd);
			if (nEndIndex != -1)
			{
				// ��Ϊ�������ҲҪ������������Ҫ���Ͻ�����ǵĳ���
				nEndIndex += strRecordEnd.size();
				string strNewHosts;
				for (int i = 0; i < (int)strHosts.size(); i++)
				{
					if (!(i >= nBeginIndex && i <= nEndIndex))
					{
						strNewHosts += strHosts[i];
					}
				}

				// �� "w+" ��ʽ���´��ļ����������
				fclose(fp);
				nFileCode = fopen_s(&fp, "C:\\Windows\\System32\\drivers\\etc\\hosts", "w+");
				if (nFileCode != 0)
				{
					printf("�޷��� hosts �ļ���fopen_s �����룺%d\n", nFileCode);
					printf("��������˳�����\n");
					_getwch();
					return -1;
				}

				fputs(strNewHosts.c_str(), fp);
			}
		}

		// д���¼�¼
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

		printf("д�� hosts �ļ����");

		// ˢ�� dns ����
		system("ipconfig /flushdns");

		printf("\n\n");
		printf("\nע����ʧ�ܣ����Ժ�Ƭ�̣����Լ��Ρ�\n");
		printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		printf("\n>>>>>>>>>>>> �� [Enter] һ������ Github !!! <<<<<<<<<<<<");
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
