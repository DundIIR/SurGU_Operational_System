#include <windows.h>
#include <iostream>	 	 
#include "buffer.h"
#include <ctime>

#define cProducers 3	/*���������� ��������������*/	 
#define cConsumers 4	/*���������� ������������*/	 
#define BufferSize 100	/*������ ������*/	 

int cOperations = 100; /*���������� �������� ��� �������*/

HANDLE mutex, semaphoreMin, semaphoreMax;

DWORD __stdcall getkey(void* b) {
	cin.get();

	return cOperations = 0;

}

// ������-�������������
DWORD __stdcall producer(void* b) {
	while (cOperations-- > 0) {

		WaitForSingleObject(semaphoreMax, INFINITE);

		WaitForSingleObject(mutex, INFINITE);
		int item = rand();
		cout << "��������: " << item << endl;
		((Buffer*)b)->PutItem(item);
		ReleaseMutex(mutex);

		ReleaseSemaphore(semaphoreMin, 1, NULL);
		
		Sleep(500 + rand() % 100);
	}
	return 0;
}

// ������-����������� 
DWORD __stdcall consumer(void* b) {
	while (cOperations-- > 0) {

		WaitForSingleObject(semaphoreMin, INFINITE);

		WaitForSingleObject(mutex, INFINITE);
		cout << "\t��������: " << ((Buffer*)b)->GetItem() << endl;
		ReleaseMutex(mutex);

		ReleaseSemaphore(semaphoreMax, 1, NULL);

		Sleep(500 + rand() % 100);
	}
	return 0;
}

int main() {

	system("chcp 1251 > 0");
	srand(time(nullptr));

	Buffer* Buf = Buffer::CreateBuffer(BufferSize); // �������� ������
	HANDLE hThreads[cProducers + cConsumers];

	mutex = CreateMutex(
		nullptr, // ��������� �� ���������� ������������ ��� ������ ��������
		FALSE, // ����, �����������, ������ �� �����, ��������� �������, ���� ��� ����������
		nullptr // ��������� �� ������, ���������� ��� ��������
	);

	semaphoreMin = CreateSemaphore(NULL, 0, BufferSize, NULL);
	semaphoreMax = CreateSemaphore(NULL, BufferSize, BufferSize, NULL);

	CreateThread(0, 0, getkey, 0, 0, 0); // ��������������� �����, ��������� ������� �������

	for (int i = 0; i < cProducers; i++)
		hThreads[i] = CreateThread(0, 0, producer, Buf, 0, 0); // �������� �������-��������������

	for (int i = cProducers; i < cProducers + cConsumers; i++) 
		hThreads[i] = CreateThread(0, 0, consumer, Buf, 0, 0); // �������� �������-������������

	WaitForMultipleObjects(cProducers + cConsumers, hThreads, true, INFINITE);


	
	cin.get();

	return 0;
}
