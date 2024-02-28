#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {

    system("chcp 1251 > 0");

    const char IP_SERV[] = "192.168.1.58";			// Enter local Server IP address
    const int PORT_NUM = 1234;				// Enter Open working server port
    const short BUFF_SIZE = 1024;			// Maximum size of buffer for exchange info between server and client

    // Привязка сокета к адресу и порту
    in_addr ip_to_num;
    int erStat = inet_pton(AF_INET, IP_SERV, &ip_to_num);
    if (erStat <= 0) {
        cout << "Ошибка при переводе IP-адреса в специальный числовой формат" << endl;
        return 1;
    }

    // Инициализация WinSock
    WSADATA wsData;

    erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (erStat != 0) { 
        cout << "Ошибка при инициализации WinSock: " << WSAGetLastError() << endl;
        return 1;
    }
    else
        cout << "Инициализация WinSock прошла успешно";


    // Создание сокета для сервера
    SOCKET ServSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServSocket == INVALID_SOCKET) {
        cout << "Ошибка при создании серверного сокета: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "Сокет для сервера успешно создан" << endl;


    

    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(PORT_NUM);

    erStat = bind(ServSocket, (sockaddr*)&servInfo, sizeof(servInfo));
    if (erStat != 0) {
        cout << "Ошибка привязки сокета к информации о сервере: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "Привязка сокета к информации о сервере прошла успешно" << endl;
    



    // Прослушивание входящих соединений
    erStat = listen(ServSocket, SOMAXCONN); // для ограничения SOMAXCONN_HINT(N)

    if (erStat != 0) {
        cout << "Ошибка прослушивания входящих соединений: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        WSACleanup();
        return 1;
    }
    else {
        cout << "Прослушивание настроено..." << endl;
    }

    // Принятие входящего соединения
    sockaddr_in clientInfo;

    ZeroMemory(&clientInfo, sizeof(clientInfo));

    int clientInfo_size = sizeof(clientInfo);

    SOCKET ClientConnect = accept(ServSocket, (sockaddr*)&clientInfo, &clientInfo_size);

    if (ClientConnect == INVALID_SOCKET) {
        cout << "Ошибка при принятии входящего соединения: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        closesocket(ClientConnect);
        WSACleanup();
        return 1;
    }
    else {
        cout << "Клиент подключен!\n" << endl;
        cout << "Connection to a client established successfully" << endl;
        char clientIP[22];

        inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
    }


    //Exchange text data between Server and Client. Disconnection if a client send "xxx"

    vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);							// Creation of buffers for sending and receiving data
    short packet_size = 0;												// The size of sending / receiving packet in bytes

    while (true) {
        packet_size = recv(ClientConnect, servBuff.data(), servBuff.size(), 0);					// Receiving packet from client. Program is waiting (system pause) until receive
        cout << "Client's message: " << servBuff.data() << endl;

        cout << "Your (host) message: ";
        fgets(clientBuff.data(), clientBuff.size(), stdin);

        // Check whether server would like to stop chatting 
        if (clientBuff[0] == 'x' && clientBuff[1] == 'x' && clientBuff[2] == 'x') {
            shutdown(ClientConnect, SD_BOTH);
            closesocket(ServSocket);
            closesocket(ClientConnect);
            WSACleanup();
            return 0;
        }

        packet_size = send(ClientConnect, clientBuff.data(), clientBuff.size(), 0);

        if (packet_size == SOCKET_ERROR) {
            cout << "Can't send message to Client. Error # " << WSAGetLastError() << endl;
            closesocket(ServSocket);
            closesocket(ClientConnect);
            WSACleanup();
            return 1;
        }

    }


    // Закрытие сокетов и освобождение ресурсов
    closesocket(ClientConnect);
    closesocket(ServSocket);
    WSACleanup();

    return 0;
}