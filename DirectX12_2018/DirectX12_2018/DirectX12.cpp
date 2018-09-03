#include "DirectX12.h"
#include "Geometory.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

DirectX12::DirectX12()
{
}


DirectX12::~DirectX12()
{
}

void DirectX12::Dx12()
{
	//�f�o�C�X�̍쐬
	ID3D12Device* dev = nullptr;

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

	//�R�}���h�n�̍쐬
	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12CommandQueue* cmdQueue = nullptr;
	ID3D12CommandList* cmdList = nullptr;

	//�R�}���h�A���P�[�^�[�̍쐬
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));

	//�R�}���h���X�g�̍쐬
	result = dev->CreateCommandList(0,//nodemask
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator,
		nullptr,
		IID_PPV_ARGS(&cmdList));

	//�R�}���h�L���[�̍쐬
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	result = dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));


	//�X���b�v�`�F�C���̍쐬
	IDXGIFactory2* factory = nullptr;
	result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	IDXGISwapChain* swapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;

	swapChainDesc.Width = WindowWidth;//�������݉��͈�
	swapChainDesc.Height = WindowHeight;//�������ݏc�͈�
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;//�}���`�T���v���̐�
	swapChainDesc.SampleDesc.Quality = 0;//�}���`�T���v���̕i��
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;//�o�b�N�o�b�t�@�̐�(2)
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	//esult = factory->CreateSwapChainForHwnd(cmdQueue,hwnd, &swapChainDesc,nullptr,nullptr, (IDXGISwapChain1**)(&swapChain));

	int a = 0;
}

