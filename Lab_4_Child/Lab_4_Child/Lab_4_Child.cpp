#include <windows.h>
#include <iostream>

using namespace std;

int main()
{
    system("chcp 1251 > 0");
    char buffer[256] = {};
    //DWORD bRead;

    ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer), NULL, NULL);

    cout << "Я потомок, получил следующее сообщение: \n\t" << buffer << endl;

    return 0;
}