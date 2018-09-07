#include "DirectX12.h"
#include "Geometory.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <vector>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

const int screenBufferNum = 2;//画面バッファの数

DirectX12::DirectX12()
{
}


DirectX12::~DirectX12()
{
}

void DirectX12::Dx12(HWND hwnd)
{
	//デバイスの作成
	//ID3D12Device* dev = nullptr;

	D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL level = {};
	HRESULT result = S_OK;
	for (auto lev : levels)
	{
		result = D3D12CreateDevice(nullptr, lev, IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			level = lev;
			break;
		}
	}

	//コマンド系の作成
	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12CommandQueue* cmdQueue = nullptr;
	ID3D12CommandList* cmdList = nullptr;

	//コマンドアロケーターの作成
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));

	//コマンドリストの作成
	result = dev->CreateCommandList(0,//nodemask
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator,
		nullptr,
		IID_PPV_ARGS(&cmdList));

	//コマンドキューの作成
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	result = dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));



	//RTV用のデスクリプターヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.NumDescriptors = screenBufferNum;
	descHeapDesc.NodeMask = 0;
	ID3D12DescriptorHeap* descriptorHeapRTV = nullptr;
	result = dev->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&descriptorHeapRTV));

	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandleRTV(descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());



	//スワップチェインの作成
	IDXGIFactory6* dxgiFactory = nullptr;//dxgi
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));


	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

	swapChainDesc.Width = WindowWidth;//書き込み横範囲
	swapChainDesc.Height = WindowHeight;//書き込み縦範囲
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルの数
	swapChainDesc.SampleDesc.Quality = 0;//マルチサンプルの品質
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = screenBufferNum;//バックバッファの数(2)
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	result = dxgiFactory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapChainDesc, nullptr, nullptr, (IDXGISwapChain1**)(&swapChain));

	//swapChain->GetDesc(&swapChainDesc);

	int rtvNum = swapChainDesc.BufferCount;

	//レンダーターゲットの作成
	std::vector<ID3D12Resource*> renderTarget;
	renderTarget.resize(rtvNum);
	int descriptorSizeRTV = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < rtvNum; ++i)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget[i]));
		dev->CreateRenderTargetView(renderTarget[i], nullptr, descriptorHandleRTV);
		descriptorHandleRTV.Offset(descriptorSizeRTV);
	}

	ID3D12Fence* fence = nullptr;
	int a = 0;
}

