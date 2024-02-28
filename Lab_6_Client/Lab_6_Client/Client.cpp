#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {

    system("chcp 1251 > 0");

    const char SERVER_IP[] = "192.168.1.58";					// Enter IPv4 address of Server
    const short SERVER_PORT_NUM = 1234;				// Enter Listening port on Server side
    const short BUFF_SIZE = 1024;

    // Инициализация WinSock
    WSADATA wsData;
    int erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (erStat != 0) {
        cout << "Ошибка при инициализации WinSock: " << WSAGetLastError() << endl;
        return 1;
    }
    else
        cout << "Инициализация WinSock прошла успешно";


    // Создание сокета для сервера
    SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "Ошибка при создании серверного сокета: " << WSAGetLastError() << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "Сокет для сервера успешно создан" << endl;

    // Подключение к серверу
    in_addr ip_to_num;
    erStat = inet_pton(AF_INET, SERVER_IP, &ip_to_num);
    if (erStat <= 0) {
        cout << "Error in IP translation to special numeric format" << endl;
        return 1;
    }

    sockaddr_in servInfo;

    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;	  // Server's IPv4 after inet_pton() function
    servInfo.sin_port = htons(SERVER_PORT_NUM);

    erStat = connect(ClientSocket, (sockaddr*)&servInfo, sizeof(servInfo));

    if (erStat != 0) {
        cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "Connection established SUCCESSFULLY. Ready to send a message to Server"
        << endl;



    vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);							// Buffers for sending and receiving data
    short packet_size = 0;												// The size of sending / receiving packet in bytes

    while (true) {

        cout << "Your (Client) message to Server: ";
        fgets(clientBuff.data(), clientBuff.size(), stdin);

        // Check whether client like to stop chatting 
        if (clientBuff[0] == 'x' && clientBuff[1] == 'x' && clientBuff[2] == 'x') {
            shutdown(ClientSocket, SD_BOTH);
            closesocket(ClientSocket);
            WSACleanup();
            return 0;
        }

        packet_size = send(ClientSocket, clientBuff.data(), clientBuff.size(), 0);

        if (packet_size == SOCKET_ERROR) {
            cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        packet_size = recv(ClientSocket, servBuff.data(), servBuff.size(), 0);

        if (packet_size == SOCKET_ERROR) {
            cout << "Can't receive message from Server. Error # " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        else
            cout << "Server message: " << servBuff.data() << endl;

    }

    // Закрытие сокетов и освобождение ресурсов
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}