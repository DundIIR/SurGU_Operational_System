#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <mutex>
#include <list>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

mutex mutx; // Мьютекс для безопасного доступа к общим данным
list<SOCKET> clients; // Список сокетов всех подключенных клиентов
list<HANDLE> hThreads; // Потоки для обработки клиентов

bool OutNameClient(SOCKET СlientSocket) {
    vector<char> servBuff(1024); // Буфер для данных от клиента

    int packet_size = recv(СlientSocket, servBuff.data(), servBuff.size(), 0);
    if (packet_size <= 0) {
        mutx.lock();
        clients.remove(СlientSocket); // Удаляем сокет клиента из списка
        closesocket(СlientSocket);
        mutx.unlock();
        return 1;
    }
    else {
        string name(servBuff.data(), packet_size);
        string message = "\r\t<-- Подключился новый пользователь: " + name + " -->\n";
        copy(message.begin(), message.end(), servBuff.begin());
    }
    for (SOCKET otherClient : clients) {
        if (otherClient != СlientSocket) {
            send(otherClient, servBuff.data(), servBuff.size(), 0);
        }
    }
    return 0;
}

string GetIP(SOCKET ClientSocket) {
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    getpeername(ClientSocket, (sockaddr*)&clientAddr, &addrLen);

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

    return string(clientIP);
}

DWORD WINAPI HandleClient(LPVOID lpVoid) {
    vector<char> ServBuff(1024); // Буфер для данных от клиента
    SOCKET СlientSocket = (SOCKET)lpVoid;

    bool active = TRUE;

    if (OutNameClient(СlientSocket)) {
        cout << "Не успешная попытка ввода имени" << endl;
    }

    while (true) {
        
        int packet_size = recv(СlientSocket, ServBuff.data(), ServBuff.size(), 0);
        if (packet_size <= 0) {
            // Ошибка при получении данных или клиент отключился
            mutx.lock();
            clients.remove(СlientSocket); // Удаляем сокет клиента из списка
            mutx.unlock();
    

            string message = "\r\t<-- Клиент " + GetIP(СlientSocket) + " отключился -->";
            closesocket(СlientSocket);
            copy(message.begin(), message.end(), ServBuff.begin());
            cout << message << endl;
            active = FALSE;
        }

        // Отправляем сообщение от клиента всем остальным клиентам
        mutx.lock();
        for (SOCKET otherClient : clients) {
            if (otherClient != СlientSocket) {
                send(otherClient, ServBuff.data(), ServBuff.size(), 0);
            }
        }
        mutx.unlock();
        memset(ServBuff.data(), 0, ServBuff.size());
        if (!active) return 1;
    }
    return 0;
}

void CreateClientThread(SOCKET СlientSocket) {
    HANDLE hThread = CreateThread(nullptr, 0, HandleClient, (LPVOID)СlientSocket, 0, nullptr);
    if (hThread != NULL) {
        hThreads.push_back(hThread);
    }
    else {
        // Обработка ошибки при создании потока
    }
}

void CloseClientThreads() {
    for (HANDLE hThread : hThreads) {
        CloseHandle(hThread);
    }
}


int main() {

    system("chcp 1251 > 0");

    const char IP_SERV[] = "172.20.10.2"; // IP-адрес локального сервера
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


    // --- Создание сокета для сервера ---
    SOCKET ServSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServSocket == INVALID_SOCKET) {
        cout << "Ошибка при создании серверного сокета: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "Сокет для сервера успешно создан" << endl;


    
    // --- Привязка серверного сокета ---
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
    



    // --- Прослушивание входящих соединений с клиентом ---
    erStat = listen(ServSocket, SOMAXCONN); // для ограничения SOMAXCONN_HINT(N)

    if (erStat != 0) {
        cout << "Ошибка прослушивания входящих соединений: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        WSACleanup();
        return 1;
    }
    else {
        cout << "Прослушивание настроено, ждем клиента..." << endl;
    }

    
    while (true) {
        // Принятие клиентского сокета в случае подключения
        sockaddr_in clientInfo;
        int clientInfoSize = sizeof(clientInfo);
        SOCKET ClientSocket = accept(ServSocket, (sockaddr*)&clientInfo, &clientInfoSize);
        if (ClientSocket == INVALID_SOCKET) {
            cout << "Ошибка при принятии входящего соединения: " << WSAGetLastError() << endl;
            closesocket(ServSocket);
            WSACleanup();
            return 1;
        }
        else {
            // Получение клиентского IP
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
            cout << "Соединение с клиентом: "<< clientIP << " установлено успешно!" << endl;
            
            mutx.lock();
            clients.push_back(ClientSocket); // Добавляем сокет клиента в общий список
            mutx.unlock();

            CreateClientThread(ClientSocket); // Запускаем поток для обработки клиента
        }
    }

    // Закрытие сокетов и освобождение ресурсов
    closesocket(ServSocket);
    WSACleanup();

    return 0;
}