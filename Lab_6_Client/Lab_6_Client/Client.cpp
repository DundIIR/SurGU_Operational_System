#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <conio.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

string name;

DWORD WINAPI SendMessageThread(LPVOID lpVoid) {

    SOCKET ClientSocket = (SOCKET)lpVoid;
    vector<char> ClientBuff(1024);

    

    while (true) {
        string message;
        cout << "--> ";
        getline(cin, message);


        // Проверяем, не желает ли клиент завершить чат
        if (message == "Ушел") {
            shutdown(ClientSocket, SD_BOTH); // закрытие одного или обоих направлений потока данных для сокета send и recv
            closesocket(ClientSocket);
            return 0;
        }

        message = name + ": " + message;
        copy(message.begin(), message.end(), ClientBuff.begin());
        int packet_size = send(ClientSocket, ClientBuff.data(), ClientBuff.size(), 0);

        if (packet_size == SOCKET_ERROR) {
            cout << "Ошибка при отправке сообщения серверу: " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            return 1;
        }
    }

    return 0;
}

DWORD WINAPI ReceiveMessageThread(LPVOID lpVoid) {

    SOCKET ClientSocket = (SOCKET)lpVoid;
    vector<char> servBuff(1024);  // Буфер для данных от сервера

    while (true) {
        int packet_size = recv(ClientSocket, servBuff.data(), servBuff.size(), 0);
        if (packet_size <= 0) {
            // Ошибка при получении данных или сервер отключился
            cout << "Ошибка при получении сообщения от сервера: " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            return 1;
        }
        else {
            cout << "\r   \t\t<-- " << servBuff.data() << "\n--> ";
        }
    }
    return 0;
}

int main() {

    system("chcp 1251 > 0");

    const char IP_SERV[] = "192.168.1.58"; // IP-адрес локального сервера
    const int PORT_NUM = 1234; // Открытый рабочий порт сервера
    const short BUFF_SIZE = 1024; // Максимальный размер буфера для обмена информацией

    // IP в строковом формате преобразуется в числовой формат для функций сокета. Данные находятся в "ip_to_num"
    in_addr ip_to_num;
    int erStat = inet_pton(AF_INET, IP_SERV, &ip_to_num);
    if (erStat <= 0) {
        cout << "Ошибка при переводе IP-адреса в специальный числовой формат" << endl;
        return 1;
    }

    // --- Инициализация WinSock ---
    WSADATA wsData;
    erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (erStat != 0) {
        cout << "Ошибка при инициализации WinSock: " << WSAGetLastError() << endl;
        return 1;
    }
    else
        cout << "Инициализация WinSock прошла успешно";


    // --- Создание сокета для клиента ---
    SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "Ошибка при создании клиентского сокета: " << WSAGetLastError() << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "Сокет для клиента успешно создан" << endl;

    // --- Получаем имя клиента ---
    system("cls");
    cout << "Введите ваше имя: ";
    getline(cin, name);
    cout << endl;

    // --- Подключение к серверу ---
    sockaddr_in servInfo;

    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;	  // Server's IPv4 after inet_pton() function
    servInfo.sin_port = htons(PORT_NUM);

    erStat = connect(ClientSocket, (sockaddr*)&servInfo, sizeof(servInfo));

    if (erStat != 0) {
        cout << "Ошибка соединения сокета с сервером: " << WSAGetLastError() << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    else {
        send(ClientSocket, name.c_str(), name.size(), 0);
    }

    // Запуск потока для отправки сообщений
    HANDLE hSendThread = CreateThread(nullptr, 0, SendMessageThread, (LPVOID)ClientSocket, 0, nullptr);
    if (hSendThread == NULL) {
        cerr << "Ошибка при создании потока: " << GetLastError() << endl;
        return 1;
    }
    // Запуск потока для приема сообщений
    HANDLE hRecvThread = CreateThread(nullptr, 0, ReceiveMessageThread, (LPVOID)ClientSocket, 0, nullptr);
    if (hRecvThread == NULL) {
        cerr << "Ошибка при создании потока: " << GetLastError() << endl;
        return 1;
    }

    // Ожидание завершения потоков
    WaitForSingleObject(hSendThread, INFINITE);
    WaitForSingleObject(hRecvThread, INFINITE);

    // Закрытие дескрипторов потоков
    CloseHandle(hSendThread);
    CloseHandle(hRecvThread);

    // Закрытие сокетов и освобождение ресурсов
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}