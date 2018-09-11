#include "MyDirectX12.h"
#include "Geometory.h"
//#include <DirectXMath.h>
#include "d3dx12.h"


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

const int screenBufferNum = 2;//��ʃo�b�t�@�̐�

MyDirectX12::MyDirectX12(HWND _hwnd) :hwnd(_hwnd),dxgiFactory(nullptr), adapter(nullptr), dev(nullptr), cmdAllocator(nullptr), cmdQueue(nullptr), cmdList(nullptr),
descriptorHeapRTV(nullptr), swapChain(nullptr),rootSignature(nullptr),signature(nullptr),error(nullptr)
{
	MyDirectX12::CreateDXGIFactory();
	MyDirectX12::CreateDevice();
	MyDirectX12::CreateCommandQueue();
	MyDirectX12::CreateCommandAllocator();
	MyDirectX12::CreateCommandList();
	MyDirectX12::CreateDescriptorHeap();
	MyDirectX12::CreateSwapChain();
	MyDirectX12::CreateRenderTarget();
	MyDirectX12::CreateRootSignature();
}


MyDirectX12::~MyDirectX12()
{
}



void MyDirectX12::Dx12()
{
	//���C�����[�v�ɓ������ޏꏊ

	HRESULT result = S_OK;
	float clearColor[4] = { 255, 255, 255, 255 };
	auto heapStart = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();

	cmdAllocator->Reset();//�A���P�[�^���Z�b�g
	cmdList->Reset(cmdAllocator, nullptr);//���X�g���Z�b�g
	cmdList->OMSetRenderTargets(1, &heapStart, false, nullptr);//�����_�[�^�[�Q�b�g�ݒ�
	cmdList->ClearRenderTargetView(descriptorHandle, clearColor, 0, nullptr);//�N���A
	cmdList->Close();//���X�g�̃N���[�Y

	ID3D12CommandList* cmdlists[] = { cmdList };
	
	cmdQueue->ExecuteCommandLists(1, cmdlists);

	swapChain->Present(1, 0);
}

ID3D12Device * MyDirectX12::GetDevice()
{
	return dev;
}

ID3D12CommandAllocator * MyDirectX12::GetCommandAllocator()
{
	return cmdAllocator;
}

ID3D12CommandQueue * MyDirectX12::GetCommandQueue()
{	
	return nullptr;
}

ID3D12GraphicsCommandList3 * MyDirectX12::GetCommandList()
{
	return cmdList;
}

void MyDirectX12::CreateDXGIFactory()
{
	//�t�@�N�g���[�̍쐬
	
	HRESULT result = S_OK;
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
}

void MyDirectX12::CreateDevice()
{

	//�f�o�C�X�̍쐬

	HRESULT result = S_OK;
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL level = {};

	for (auto lev : levels)
	{
		result = D3D12CreateDevice(nullptr, lev, IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			level = lev;
			break;
		}
	}
}

void MyDirectX12::CreateCommandAllocator()
{
	//�R�}���h�A���P�[�^�[�̍쐬

	HRESULT result = S_OK;

	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
}

void MyDirectX12::CreateCommandQueue()
{
	//�R�}���h�L���[�̍쐬

	HRESULT result = S_OK;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	result = dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
}

void MyDirectX12::CreateCommandList()
{
	//�R�}���h���X�g�̍쐬

	HRESULT result = S_OK;
	int nodemask = 0;

	result = dev->CreateCommandList(nodemask,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator,
		nullptr,
		IID_PPV_ARGS(&cmdList));
}

void MyDirectX12::CreateDescriptorHeap()
{
	//RTV�p�̃f�X�N���v�^�[�q�[�v�̍쐬

	HRESULT result = S_OK;
	
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.NumDescriptors = screenBufferNum;
	descHeapDesc.NodeMask = 0;
	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descriptorHeapRTV));
}

void MyDirectX12::CreateSwapChain()
{
	//�X���b�v�`�F�C���̍쐬

	HRESULT result = S_OK;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

	swapChainDesc.Width = WindowWidth;//�������݉��͈�
	swapChainDesc.Height = WindowHeight;//�������ݏc�͈�
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;//�}���`�T���v���̐�
	swapChainDesc.SampleDesc.Quality = 0;//�}���`�T���v���̕i��
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = screenBufferNum;//�o�b�N�o�b�t�@�̐�(2)
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	result = dxgiFactory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapChainDesc, nullptr, nullptr, (IDXGISwapChain1**)(&swapChain));
}

void MyDirectX12::CreateRenderTarget()
{
	//�����_�[�^�[�Q�b�g�̍쐬

	HRESULT result = S_OK;
	int rtvNum = screenBufferNum;
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandleRTV(descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());
	descriptorHandle = descriptorHandleRTV;

	renderTarget.resize(rtvNum);
	int descriptorSizeRTV = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < rtvNum; ++i)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget[i]));
		dev->CreateRenderTargetView(renderTarget[i], nullptr, descriptorHandleRTV);
		descriptorHandleRTV.Offset(descriptorSizeRTV);
	}
}

void MyDirectX12::CreateRootSignature()
{
	//���[�g�V�O�l�`���̍쐬
	//���[�g�V�O�l�`���Ƃ́A�p�C�v���C���̌^�ɍ����A�v���P�[�V�����ɕK�v�Ȃ��̂�������

	HRESULT result = S_OK;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	result = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	result = dev->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
}

