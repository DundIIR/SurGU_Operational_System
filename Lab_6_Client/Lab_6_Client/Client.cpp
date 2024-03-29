#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <conio.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define BUFF_SIZE 1024 // ������������ ������ ������ ��� ������ �����������
#define NAME_SIZE 30

string name;

DWORD WINAPI SendMessageThread(LPVOID lpVoid) {

    SOCKET ClientSocket = (SOCKET)lpVoid;
    vector<char> ClientBuff(BUFF_SIZE);

    

    while (true) {
        string message;
        cout << "--> ";
        getline(cin, message);
        if (message.length() > BUFF_SIZE - NAME_SIZE - 2) {
            message.resize(BUFF_SIZE - NAME_SIZE - 2);
        }
        else if (message.length() < 1) {
            continue;
        }

        // ���������, �� ������ �� ������ ��������� ���
        if (message == "����") {
            shutdown(ClientSocket, SD_BOTH); // �������� ������ ��� ����� ����������� ������ ������ ��� ������ send � recv
            closesocket(ClientSocket);
            return 0;
        }

        message = name + ": " + message;
        copy(message.begin(), message.end(), ClientBuff.begin());
        int packet_size = send(ClientSocket, ClientBuff.data(), ClientBuff.size(), 0);

        if (packet_size == SOCKET_ERROR) {
            cout << "������ ��� �������� ��������� �������: " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            return 1;
        }

        memset(ClientBuff.data(), 0, ClientBuff.size());
    }

    return 0;
}

DWORD WINAPI ReceiveMessageThread(LPVOID lpVoid) {

    SOCKET ClientSocket = (SOCKET)lpVoid;
    vector<char> ServBuff(BUFF_SIZE);  // ����� ��� ������ �� �������

    while (true) {
        int packet_size = recv(ClientSocket, ServBuff.data(), ServBuff.size(), 0);
        if (packet_size <= 0) {
            // ������ ��� ��������� ������ ��� ������ ����������
            cout << "������ ��� ��������� ��������� �� �������: " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            return 1;
        }
        else {
            cout << "\r   \t\t<-- " << ServBuff.data() << "\n--> ";
            memset(ServBuff.data(), 0, ServBuff.size());
        }
    }
    return 0;
}

int main() {

    system("chcp 1251 > 0");

    const char IP_SERV[] = "172.20.10.2"; // IP-����� ���������� �������
    const int PORT_NUM = 1234; // �������� ������� ���� �������
   

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
    SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "������ ��� �������� ����������� ������: " << WSAGetLastError() << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    else
        cout << "����� ��� ������� ������� ������" << endl;

    // --- �������� ��� ������� ---
    while (true) {
        system("cls");
        cout << "������� ���� ���: ";
        getline(cin, name);
        if (name.length() > NAME_SIZE || name.length() < 1) {
            continue;
        }
        break;
    }
    cout << endl;

    // --- ����������� � ������� ---
    sockaddr_in servInfo;

    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;	  // Server's IPv4 after inet_pton() function
    servInfo.sin_port = htons(PORT_NUM);

    erStat = connect(ClientSocket, (sockaddr*)&servInfo, sizeof(servInfo));

    if (erStat != 0) {
        cout << "������ ���������� ������ � ��������: " << WSAGetLastError() << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    else {
        send(ClientSocket, name.c_str(), name.size(), 0);
    }

    // ������ ������ ��� �������� ���������
    HANDLE hSendThread = CreateThread(nullptr, 0, SendMessageThread, (LPVOID)ClientSocket, 0, nullptr);
    if (hSendThread == NULL) {
        cerr << "������ ��� �������� ������: " << GetLastError() << endl;
        return 1;
    }
    // ������ ������ ��� ������ ���������
    HANDLE hRecvThread = CreateThread(nullptr, 0, ReceiveMessageThread, (LPVOID)ClientSocket, 0, nullptr);
    if (hRecvThread == NULL) {
        cerr << "������ ��� �������� ������: " << GetLastError() << endl;
        return 1;
    }

    // �������� ���������� �������
    WaitForSingleObject(hSendThread, INFINITE);
    WaitForSingleObject(hRecvThread, INFINITE);

    // �������� ������������ �������
    CloseHandle(hSendThread);
    CloseHandle(hRecvThread);

    // �������� ������� � ������������ ��������
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}