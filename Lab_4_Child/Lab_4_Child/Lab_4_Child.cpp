#include <windows.h>
#include <iostream>

using namespace std;

int main()
{
    system("chcp 1251 > 0");
    char buffer[256] = {};

    ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer), NULL, NULL);

    cout << "Это дочернее консольное окно\n" << endl;

    cout << "Я потомок, получил следующее сообщение: \n\t" << buffer << endl;

    system("pause");

    return 0;
}