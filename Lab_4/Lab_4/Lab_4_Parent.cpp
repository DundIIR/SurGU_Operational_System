#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>

using namespace std;

int main()
{
    system("chcp 1251 > 0");
    
    HANDLE hReadPipe, hWritePipe;

    // ��������� ������ ����� � ���������, ����� ����������� �������������
    SECURITY_ATTRIBUTES sA;
    sA.nLength = sizeof(SECURITY_ATTRIBUTES); // ������ ��������� � ������
    sA.bInheritHandle = TRUE; // ������ � ������ ����� �������������� �����������
    sA.lpSecurityDescriptor = NULL; // ����� ������� � ����������� �� ������, ��������� �������� ������������ ������ ��������

    // �������� ���������� ������ 
    CreatePipe(&hReadPipe, &hWritePipe, &sA, 0);
    

    // �������� ��������
    STARTUPINFO sI;
    PROCESS_INFORMATION pI;

    ZeroMemory(&sI, sizeof(STARTUPINFO));
    sI.cb = sizeof(STARTUPINFO); // ������������� ������ ���� ������ ������� ����� ���������
    sI.hStdInput = hReadPipe;  // �������������� ����������� ���� �� ���������� ������ �� ���������� ������
    sI.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE); // ��������� ����������� �����
    sI.hStdError = GetStdHandle(STD_ERROR_HANDLE);   // ��������� ����������� ����� ������
    sI.dwFlags |= STARTF_USESTDHANDLES;  // ������������� ����, ��� ����������, ��� ���������� ������ ����� ������� 

    if (!CreateProcess(L"D:\\University-Git\\OC\\Lab_4_Child\\x64\\Debug\\Lab_4_Child.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &sI, &pI)) {
        return 1;
    }

    /*
    string s = "cmd.exe";
    LPWSTR ws = new wchar_t[s.size()-2];
    copy(s.begin(), s.end(), ws);
  
    if (!CreateProcess(NULL, ws, NULL, NULL, TRUE, 0, NULL, NULL, &sI, &pI)) {
        return 1;
    }
    */

    // �������� ������
    char message[] = "� ��������. � ��� ��������� ���������� �� ��������!";
    DWORD bWritten;
    WriteFile(hWritePipe, message, sizeof(message), &bWritten, NULL);

    // �������� ���������� �������� �������
    WaitForSingleObject(pI.hProcess, INFINITE);

    // ���������� �������� ������������
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);

    // ���������� �������� � ������
    CloseHandle(pI.hProcess);
    CloseHandle(pI.hThread);

	return 0;
}