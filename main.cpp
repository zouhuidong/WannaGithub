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
 * @brief		����������ȡ ip ��ַ
 * @param[in]	_strDomain: ԭ����
 * @param[out]	_strIP: ������ȡ���� ip ��ַ�ַ���
 * @return		�����Ƿ�ɹ���ȡ ip
*/
bool GetIPFromDomain(string _strDomain, string* _strIp)
{
	// ��ǰ ip ��ѯ api ������https://ip-api.com/
	char* resData = sendRequest(("http://ip-api.com/xml/" + _strDomain).c_str());
	string str = resData;
	delete[] resData;

	// xml ����

	// ��ȡ״̬
	string strLabel = "<status>";
	int indexStatus = str.find(strLabel);
	if (indexStatus < 0) return false;
	string strStatus;
	for (int i = indexStatus + strLabel.size(); str[i] != '<'; i++)
		strStatus += str[i];
	if (strStatus != "success")	// ��ȡʧ��
		return false;

	// ��ȡ ip ��ַ
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
 * @brief ��ʾ��ȡ ip ����Ȼ���˳�����
*/
void ShowGetIpError()
{
	printf("\n��ȡ ip ʧ�ܣ���������Ȼ�����ԡ���������������⣬�п������������� api �Ѿ���ʱ���߳����Ѿ���ʱ��"
		"\n������ѡ����� huidong.xyz Ѱ���°����ϵ huidong_mail@163.com ���Խ�������⡣\n"
		"\n������������˳�����");
	_getwch();
	exit(-1);
}

int main()
{
	printf("\n");
	printf(	"  ________________________________                                         \n"
			" /   ____                         \\                                       \n"
			" |  /       .     |         |     |      Wanna Github ~~  ��Github ��½���� \n"
			" |  |  ___  | |_  |__  |  | |__   |      version 0.1(alpha)   2021-8-8     \n"
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

		// ���������б�
		const int nDomainNum = 3;
		string strDomain[nDomainNum] = { "github.com" ,"github.global.ssl.fastly.net" ,"codeload.Github.com" };
		string strIp[nDomainNum];

		for (int i = 0; i < nDomainNum; i++)
			if (!GetIPFromDomain(strDomain[i], &strIp[i]))
				ShowGetIpError();

		printf("��ѯ���� ip ��ɣ���ʱ %.2f ��		\r", (double)(clock() - t) / CLOCKS_PER_SEC);

		Sleep(300);
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
		fputs((strRecordBegin + "\n").c_str(), fp);
		for (int i = 0; i < nDomainNum; i++)
		{
			fprintf_s(fp, "%s %s\n", strIp[i].c_str(), strDomain[i].c_str());
		}
		fputs(strRecordEnd.c_str(), fp);

		fclose(fp);
		fp = NULL;

		printf("д�� hosts �ļ����");

		// ˢ�� dns ����
		system("ipconfig /flushdns");

		printf("\n\n");
		printf(">>>>>>>>>>>> �� [Enter] һ������ Github����ʼ��� <<<<<<<<<<<<\n");

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
