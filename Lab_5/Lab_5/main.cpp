#include <windows.h>
#include <iostream>	 	 
#include "buffer.h"
#include <ctime>

#define cProducers 3	/*Количество производителей*/	 
#define cConsumers 5	/*Количество потребителей*/	 
#define BufferSize 1000	/*Размер буфера*/	 

int cOperations = 100; /*Количество операций над буфером*/

HANDLE mutex;

DWORD __stdcall getkey(void* b) {
	cin.get();

	return cOperations = 0;

}

// потока-производителя
DWORD __stdcall producer(void* b) {
	while (cOperations-- > 0) {

		WaitForSingleObject(mutex, INFINITE);
		int item = rand();
		cout << "Произвел: " << item << endl;
		((Buffer*)b)->PutItem(item);
		ReleaseMutex(mutex);
		
		Sleep(500 + rand() % 100);
	}
	return 0;
}

// потока-потребителя 
DWORD __stdcall consumer(void* b) {
	while (cOperations-- > 0) {

		WaitForSingleObject(mutex, INFINITE);

		cout << "\tПотребил: " << ((Buffer*)b)->GetItem() << endl;
		ReleaseMutex(mutex);

		Sleep(500 + rand() % 100);
	}
	return 0;
}

int main() {

	system("chcp 1251 > 0");
	srand(time(nullptr));

	Buffer* Buf = Buffer::CreateBuffer(BufferSize); // Создание буфера
	HANDLE hThreads[cProducers + cConsumers];

	mutex = CreateMutex(
		nullptr, // указатель на дескриптор безопасности для нового мьютекса
		FALSE, // флаг, указывающий, должен ли поток, создавший мьютекс, быть его владельцем
		nullptr // указатель на строку, содержащую имя мьютекса
	);

	CreateThread(0, 0, getkey, 0, 0, 0); // Вспомогательный поток, ожидающий нажатие клавиши

	for (int i = 0; i < cProducers; i++)
		hThreads[i] = CreateThread(0, 0, producer, Buf, 0, 0); // Создание потоков-производителей

	for (int i = cProducers; i < cProducers + cConsumers; i++) 
		hThreads[i] = CreateThread(0, 0, consumer, Buf, 0, 0); // Создание потоков-потребителей

	WaitForMultipleObjects(cProducers + cConsumers, hThreads, true, INFINITE);

	cin.get();

	return 0;
}
