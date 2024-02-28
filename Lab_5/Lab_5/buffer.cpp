#include "buffer.h"

Buffer* Buffer::Buf = 0;

class _Buffer : public Buffer
{
private:
	int  	_Capacity;
	int  	_Count;
	int  	_Anybody;
	int* _b;
	int _GetItem(void);
	void _PutItem(int V);

public:
	virtual  int	GetItem(void);
	virtual  void PutItem(int V);
	_Buffer(int _Cap) : _Capacity(_Cap), _Count(0), _Anybody(0) {
		if (_Cap > 0) _b = new int[_Cap];

		else throw "Capacity must be greater then zero!";
	}
	virtual ~_Buffer() { if (_b) delete[] _b; }
};

Buffer* Buffer::CreateBuffer(int _Cap) {

	if (Buf) return Buf;

	else return Buf = new _Buffer(_Cap);
}

int  _Buffer::GetItem(void) {

	if (_Anybody > 0) {
		cout << "Buffer is busy!\n";
		return 0;
	}
	_Anybody++;
	if (_Count <= 0) {
		cout << "Buffer is empty!\n";
		_Anybody--;

		return 0;
	}
	return _GetItem();
}

void _Buffer::PutItem(int V)

{
	if (_Anybody > 0) {
		cout << "Buffer is busy!\n";
		return;
	}
	_Anybody++;
	if (_Count >= _Capacity) {
		cout << "Buffer is full!\n"; _Anybody--;

		return;
	}
	_PutItem(V);
	return;
}

int  _Buffer::_GetItem(void) {

	Sleep(rand() % 20);
	_Anybody--;
	return _b[--_Count];
}

void _Buffer::_PutItem(int V) {
	Sleep(rand() % 20); _Anybody--;
	_b[_Count++] = V;
	return;
}
