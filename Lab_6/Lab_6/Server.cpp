#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <mutex>
#include <list>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

mutex mutx; // ������� ��� ����������� ������� � ����� ������
list<SOCKET> clients; // ������ ������� ���� ������������ ��������
list<HANDLE> hThreads; // ������ ��� ��������� ��������

bool OutNameClient(SOCKET �lientSocket) {
    vector<char> servBuff(1024); // ����� ��� ������ �� �������

    int packet_size = recv(�lientSocket, servBuff.data(), servBuff.size(), 0);
    if (packet_size <= 0) {
        mutx.lock();
        clients.remove(�lientSocket); // ������� ����� ������� �� ������
        closesocket(�lientSocket);
        mutx.unlock();
        return 1;
    }
    else {
        string name(servBuff.data(), packet_size);
        string message = "\r\t<-- ����������� ����� ������������: " + name + " -->\n";
        copy(message.begin(), message.end(), servBuff.begin());
    }
    for (SOCKET otherClient : clients) {
        if (otherClient != �lientSocket) {
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
    vector<char> ServBuff(1024); // ����� ��� ������ �� �������
    SOCKET �lientSocket = (SOCKET)lpVoid;

    bool active = TRUE;

    if (OutNameClient(�lientSocket)) {
        cout << "�� �������� ������� ����� �����" << endl;
    }

    while (true) {
        
        int packet_size = recv(�lientSocket, ServBuff.data(), ServBuff.size(), 0);
        if (packet_size <= 0) {
            // ������ ��� ��������� ������ ��� ������ ����������
            mutx.lock();
            clients.remove(�lientSocket); // ������� ����� ������� �� ������
            mutx.unlock();
    

            string message = "\r\t<-- ������ " + GetIP(�lientSocket) + " ���������� -->";
            closesocket(�lientSocket);
            copy(message.begin(), message.end(), ServBuff.begin());
            cout << message << endl;
            active = FALSE;
        }

        // ���������� ��������� �� ������� ���� ��������� ��������
        mutx.lock();
        for (SOCKET otherClient : clients) {
            if (otherClient != �lientSocket) {
                send(otherClient, ServBuff.data(), ServBuff.size(), 0);
            }
        }
        mutx.unlock();
        memset(ServBuff.data(), 0, ServBuff.size());
        if (!active) return 1;
    }
    return 0;
}

void CreateClientThread(SOCKET �lientSocket) {
    HANDLE hThread = CreateThread(nullptr, 0, HandleClient, (LPVOID)�lientSocket, 0, nullptr);
    if (hThread != NULL) {
        hThreads.push_back(hThread);
    }
    else {
        // ��������� ������ ��� �������� ������
    }
}

void CloseClientThreads() {
    for (HANDLE hThread : hThreads) {
        CloseHandle(hThread);
    }
}


int main() {

    system("chcp 1251 > 0");

    const char IP_SERV[] = "172.20.10.2"; // IP-����� ���������� �������
    const int PORT_NUM = 1234; // �������� ������� ���� �������
    const short BUFF_SIZE = 1024; // ������������ ������ ������ ��� ������ �����������

    // IP � ��������� ������� ������������� � �������� ������ ��� ������� ������. ������ ��������� � "ip_to_num"
    in_addr ip_to_num;
    int erStat = inet_pton(AF_INET, IP_SERV, &ip_to_num);
    if (erStat <= 0) {
        cout << "������ ��� �������� IP-������ � ����������� �������� ������" << endl;
        return 1;
    }

    // --- ������������� WinSock ---
    WSADATA wsData;

    erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (erStat != 0) { 
        cout << "������ ��� ������������� WinSock: " << WSAGetLastError() << endl;
        return 1;
    }
    else
        cout << "������������� WinSock ������ �������";


    // --- �������� ������ ��� ������� ---
    SOCKET ServSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServSocket == INVALID_SOCKET) {
        cout << "������ ��� �������� ���������� ������: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "����� ��� ������� ������� ������" << endl;


    
    // --- �������� ���������� ������ ---
    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(PORT_NUM);

    erStat = bind(ServSocket, (sockaddr*)&servInfo, sizeof(servInfo));
    if (erStat != 0) {
        cout << "������ �������� ������ � ���������� � �������: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "�������� ������ � ���������� � ������� ������ �������" << endl;
    



    // --- ������������� �������� ���������� � �������� ---
    erStat = listen(ServSocket, SOMAXCONN); // ��� ����������� SOMAXCONN_HINT(N)

    if (erStat != 0) {
        cout << "������ ������������� �������� ����������: " << WSAGetLastError() << endl;
        closesocket(ServSocket);
        WSACleanup();
        return 1;
    }
    else {
        cout << "������������� ���������, ���� �������..." << endl;
    }

    
    while (true) {
        // �������� ����������� ������ � ������ �����������
        sockaddr_in clientInfo;
        int clientInfoSize = sizeof(clientInfo);
        SOCKET ClientSocket = accept(ServSocket, (sockaddr*)&clientInfo, &clientInfoSize);
        if (ClientSocket == INVALID_SOCKET) {
            cout << "������ ��� �������� ��������� ����������: " << WSAGetLastError() << endl;
            closesocket(ServSocket);
            WSACleanup();
            return 1;
        }
        else {
            // ��������� ����������� IP
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
            cout << "���������� � ��������: "<< clientIP << " ����������� �������!" << endl;
            
            mutx.lock();
            clients.push_back(ClientSocket); // ��������� ����� ������� � ����� ������
            mutx.unlock();

            CreateClientThread(ClientSocket); // ��������� ����� ��� ��������� �������
        }
    }

    // �������� ������� � ������������ ��������
    closesocket(ServSocket);
    WSACleanup();

    return 0;
}