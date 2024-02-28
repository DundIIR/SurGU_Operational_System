#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>

using namespace std;

int main()
{
    system("chcp 1251 > 0");
    
    HANDLE hReadPipe, hWritePipe;

    // Установка нужных полей в структуру, чтобы дескрипторы наследовались
    SECURITY_ATTRIBUTES sA;
    sA.nLength = sizeof(SECURITY_ATTRIBUTES); // размер структуры в байтах
    sA.bInheritHandle = TRUE; // доступ к каналу через унаследованные дескрипторы
    sA.lpSecurityDescriptor = NULL; // права доступа и ограничения на объект, наследует атрибуты безопасности своего родителя

    // Создание анонимного канала 
    CreatePipe(&hReadPipe, &hWritePipe, &sA, 0);
    

    // Создание процесса
    STARTUPINFO sI;
    PROCESS_INFORMATION pI;

    ZeroMemory(&sI, sizeof(STARTUPINFO));
    sI.cb = sizeof(STARTUPINFO); // устанавливает размер поля равным размеру самой структуры
    sI.hStdInput = hReadPipe;  // перенаправляем стандартный ввод на дескриптор чтения из анонимного канала
    sI.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE); // оставляем стандартный вывод
    sI.hStdError = GetStdHandle(STD_ERROR_HANDLE);   // оставляем стандартный вывод ошибок
    sI.dwFlags |= STARTF_USESTDHANDLES;  // устанавливаем флаг, для оповещения, что дескриптор потока ввода изменен 

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

    // Передача данных
    char message[] = "Я родитель. И это сообщение отправлено от родителя!";
    DWORD bWritten;
    WriteFile(hWritePipe, message, sizeof(message), &bWritten, NULL);

    // Ожидание завершения процесса потомка
    WaitForSingleObject(pI.hProcess, INFINITE);

    // Закрывание ненужных дескрипторов
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);

    // Закрывание процесса и потока
    CloseHandle(pI.hProcess);
    CloseHandle(pI.hThread);

	return 0;
}