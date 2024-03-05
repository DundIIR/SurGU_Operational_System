#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>
#include <algorithm>

using namespace std;

#define N 20000

vector<int> Array(N);

void FillRand()
{
    for (int i = 0; i < Array.size(); i++)
        Array[i] = rand() % N;

}


DWORD WINAPI SortBubbleFlag(LPVOID lpParam) {
    vector<int> tempArray(Array);
    int Size = tempArray.size();

    int i, j, bSwap = 1;

    for (i = 1; i < Size && bSwap; i++)
        for (bSwap = 0, j = Size - 1; j >= i; j--) {
            if (tempArray[j - 1] > tempArray[j])
            {
                swap(tempArray[j], tempArray[j - 1]);
                bSwap = 1;
            }
        }


    return 0;
}

DWORD WINAPI SortBubbleIndex(LPVOID lpParam) {

    vector<int> tempArray(Array);
    int Size = tempArray.size();


    int j, IndexOfChanged, Low = 0;
    while (Low < Size - 1)
    {
        for (IndexOfChanged = j = Size - 1; j > Low; j--) {
            if (tempArray[j - 1] > tempArray[j])
            {
                swap(tempArray[j], tempArray[j - 1]);
                IndexOfChanged = j;
            }
        }
        Low = IndexOfChanged;
    }

    return 0; // ���������� 0 ��� �������� ��� ����������
}

void PrintThreadTimes(HANDLE hThread) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetThreadTimes(hThread, &creationTime, &exitTime, &kernelTime, &userTime)) {
        SYSTEMTIME cT, eT, kT, uT;

        FileTimeToSystemTime(&creationTime, &cT);
        FileTimeToSystemTime(&exitTime, &eT);
        FileTimeToSystemTime(&kernelTime, &kT);
        FileTimeToSystemTime(&userTime, &uT);

        cout << "����� �������� ������: ";
        cout << cT.wHour << ":" << cT.wMinute << ":" << cT.wSecond << ":" << cT.wMilliseconds << endl;

        cout << "����� ���������� ������: ";
        cout << eT.wHour << ":" << eT.wMinute << ":" << eT.wSecond << ":" << eT.wMilliseconds << endl;

        cout << "����� ���������� � ������ ����: ";
        cout << kT.wSecond << ":" << kT.wMilliseconds << endl;

        cout << "����� ���������� � ������ ������������: ";
        cout << uT.wSecond << ":" << uT.wMilliseconds << endl;
    }
    else {
        cerr << "������ ��� ��������� ������� ������: " << GetLastError() << endl;
    }
}

int main()
{
	system("chcp 1251 > 0");
	srand(time(NULL));

	FillRand();
    
    // --- ��������� �� ��������� ---
    cout << "\n\n--- ���������� �� ��������� ---" << endl;

    
    // �������� ������� ��� ���������� �������
	HANDLE hThread1 = CreateThread(nullptr, 0, &SortBubbleFlag, nullptr, 0, nullptr);
    HANDLE hThread2 = CreateThread(nullptr, 0, &SortBubbleIndex, nullptr, 0, nullptr);

    // �������� ���������� ������ �������
    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);

    // ����� ��������� �������������
    PrintThreadTimes(hThread1);
    PrintThreadTimes(hThread2);

    // �������� �������
    CloseHandle(hThread1);
    CloseHandle(hThread2);

    // --- ��������� SortBubbleFlag ���� ---
    cout << "\n\n--- ��������� ������ SortBubbleFlag ���� ---" << endl;

    // �������� ������� ��� ���������� �������
    HANDLE hThread3 = CreateThread(nullptr, 0, &SortBubbleFlag, nullptr, CREATE_SUSPENDED, nullptr);
    HANDLE hThread4 = CreateThread(nullptr, 0, &SortBubbleIndex, nullptr, CREATE_SUSPENDED, nullptr);

    // ��������� ����������� ������ �����
    SetThreadPriority(hThread3, THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(hThread4, THREAD_PRIORITY_LOWEST);

    // ������ ���������� �������
    ResumeThread(hThread3);
    ResumeThread(hThread4);

    // �������� ���������� ������ �������
    WaitForSingleObject(hThread3, INFINITE);
    WaitForSingleObject(hThread4, INFINITE);

    // ����� ��������� �������������
    PrintThreadTimes(hThread3);
    PrintThreadTimes(hThread4);

    // �������� �������
    CloseHandle(hThread3);
    CloseHandle(hThread4);
    

    // --- ��������� SortBubbleIndex ���� ---
    cout << "\n\n--- ��������� ������ SortBubbleIndex ���� ---" << endl;

    // �������� ������� ��� ���������� �������
    HANDLE hThread5 = CreateThread(nullptr, 0, &SortBubbleFlag, nullptr, CREATE_SUSPENDED, nullptr);
    HANDLE hThread6 = CreateThread(nullptr, 0, &SortBubbleIndex, nullptr, CREATE_SUSPENDED, nullptr);


    // ��������� ����������� ������ �����
    SetThreadPriority(hThread6, THREAD_PRIORITY_LOWEST);
    SetThreadPriority(hThread5, THREAD_PRIORITY_HIGHEST);

    
    // ������ ������������� ��������� ����������� �������
    SetThreadPriorityBoost(hThread5, TRUE);
    SetThreadPriorityBoost(hThread6, TRUE);

    // ��������� ����� ��������� ��� ������� ������� �� ����� ����
    DWORD_PTR dwMask = 0x01; // ����� ��� ����������� �� ������ ����
    DWORD_PTR dwPM1 = SetThreadAffinityMask(hThread5, dwMask);
    DWORD_PTR dwPM2 = SetThreadAffinityMask(hThread6, dwMask);
    if (dwPM1 == 0 || dwPM2 == 0) {
        DWORD dwError = GetLastError();
        cout << dwError << endl;
        return 1;
    }
    

    // ������ ���������� �������
    ResumeThread(hThread5);
    ResumeThread(hThread6);

    // �������� ���������� ������ �������
    WaitForSingleObject(hThread5, INFINITE);
    WaitForSingleObject(hThread6, INFINITE);

    // ����� ��������� �������������
    PrintThreadTimes(hThread5);
    PrintThreadTimes(hThread6);

    // �������� �������
    CloseHandle(hThread5);
    CloseHandle(hThread6);

    cout << GetLastError() << endl;

	return 0;
}