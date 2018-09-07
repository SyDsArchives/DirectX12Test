#pragma once
#include <Windows.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>

class DirectX12
{
private:
	ID3D12Device* dev = nullptr;
	//IDXGIFactory6* dxgiFactory = nullptr;
	//IDXGISwapChain4* swapChain = nullptr;
	//ID3D12DescriptorHeap* descriptorHeapRTV = nullptr;
	
public:
	DirectX12();
	~DirectX12();
	
	void Dx12(HWND hwnd);
};

