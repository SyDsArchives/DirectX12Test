#pragma once
#include <Windows.h>
class DirectX12
{
private:
	DirectX12();
	virtual ~DirectX12();
	
	void operator=(const DirectX12& dx12) {}//����֎~
	DirectX12(const DirectX12 &dx12) {}//�R�s�[�֎~

	
public:
	static DirectX12& Dx12Instance()
	{
		static DirectX12 instance;
		return instance;
	}
	//void Dx12();
	void Dx12(HWND hwnd);
};

