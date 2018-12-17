#include "MyDirectX12.h"
#include "Geometory.h"
#include "d3dx12.h"
#include <iostream>
#include "LoadImageFile.h"
#include "DirectXTex.h"
#include "VMD.h"
#include "PlaneMesh.h"
#include <cmath>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DirectXTex.lib")

const char* fname = "resource/model/miku/�����~�N.pmd";
const char* vmdfile = "resource/vmd/���S�R���_���X.vmd";

const int screenBufferNum = 2;//��ʃo�b�t�@�̐�

Vertex vertices[] = {
	DirectX::XMFLOAT3(-1,-1,0)	,DirectX::XMFLOAT2(0,1),
	DirectX::XMFLOAT3(-1,1,0)	,DirectX::XMFLOAT2(0,0),
	DirectX::XMFLOAT3(1,-1,0)	,DirectX::XMFLOAT2(1,1),
	DirectX::XMFLOAT3(1,1,0)	,DirectX::XMFLOAT2(1,0),
};

//Vertex vertices[] = {
//	DirectX::XMFLOAT3(-0.5f,-0.5f,0),DirectX::XMFLOAT2(0,1),
//	DirectX::XMFLOAT3(-0.5f,0.5f,0) ,DirectX::XMFLOAT2(0,0),
//	DirectX::XMFLOAT3(0.5f,-0.5f,0) ,DirectX::XMFLOAT2(1,1),
//	DirectX::XMFLOAT3(0.5f,0.5f,0)  ,DirectX::XMFLOAT2(1,0),
//};

MyDirectX12::MyDirectX12(HWND _hwnd) : hwnd(_hwnd),
bbindex(0), descriptorSizeRTV(0),
dxgiFactory(nullptr), adapter(nullptr), dev(nullptr),
cmdAllocator(nullptr), cmdQueue(nullptr), cmdList(nullptr),
descriptorHeapRTV(nullptr), rgstDescHeap(nullptr), descriptorHeapDSB(nullptr),
swapChain(nullptr),
rootSignature(nullptr),
piplineState(nullptr),
fence(nullptr), fenceValue(0),
textureBuffer(nullptr),
vertexShader(nullptr), pixelShader(nullptr),
constantBuffer(nullptr), cbuff(nullptr), vertexBuffer(nullptr), depthBuffer(nullptr), materialDescHeap(nullptr), whiteTextureBuffer(nullptr),
bBuff(nullptr), boneBuffer(nullptr),
pmx(new PMX()),vmd(new VMD()),lastTime(0),
//�}���`�p�X�p
descriptorHeapRTV_FP(nullptr),
descriptorHeapSRV_FP(nullptr)
//firstpassBuffer(nullptr)
{
	//pmx->Load();
	vmd->Load(vmdfile);
	animationData = vmd->GetAnimationMapData();
	MyDirectX12::LoadPMDModelData(fname);

	MyDirectX12::CreateDXGIFactory();

	MyDirectX12::CreateDevice();

	plane = std::make_shared<PlaneMesh>(dev,DirectX::XMFLOAT3(0,0,0),50,50);

	MyDirectX12::CreateDescriptorHeapRTV();
	MyDirectX12::CreateDescriptorHeapRegister();
	MyDirectX12::CreateDescriptorHeapforMaterial();
	MyDirectX12::CreateDescriptorHeapforBone();
	MyDirectX12::CreateDescriptorHeapRTVforFirstPass();
	MyDirectX12::CreateDescriptorHeapSRVforFirstPass();
	MyDirectX12::CreateDescriptorHeapRTVforSecondPass();
	MyDirectX12::CreateDescriptorHeapSRVforSecondPass();
	MyDirectX12::CreateDescriptorHeapforPeraTexture();
	MyDirectX12::CreateDescriptorHeapSRVforToon();

	MyDirectX12::CreateCommandQueue();
	MyDirectX12::CreateCommandAllocator();
	MyDirectX12::CreateCommandList();
	
	MyDirectX12::CreateSwapChain();
	MyDirectX12::CreateRenderTarget();
	MyDirectX12::CreateRenderTargetforFirstPass();
	MyDirectX12::CreateShaderResourceforFirstPass();

	MyDirectX12::CreateRenderTargetforSecondPass();
	MyDirectX12::CreateShaderResourceforSecondPass();
	MyDirectX12::CreateFence();

	MyDirectX12::CreateVertexBuffer();
	MyDirectX12::CreateIndexBuffer();
	MyDirectX12::CreateDepthBuffer();

	MyDirectX12::CreateSamplerState();
	MyDirectX12::CreateRootParameter();
	MyDirectX12::CreateRootSignature();

	MyDirectX12::CreatePiplineState();

	MyDirectX12::CreateTextureBuffer();
	MyDirectX12::CreateWhiteTextureBuffer();
	MyDirectX12::CreateConstantBuffer();
	MyDirectX12::CreateMaterialBuffer();
	MyDirectX12::CreateBoneBuffer();
	MyDirectX12::CreateToonTextureBuffer();

	MyDirectX12::CreateVertexBufferforPeraPolygon();
	MyDirectX12::CreatePeraPolygonTexture();
	MyDirectX12::CreateSamplerStateforPeraPolygon();
	MyDirectX12::CreateRootParameterforPeraPolygon();
	MyDirectX12::CreateRootSignatureforPeraPolygon();
	MyDirectX12::CreatePiplineStateforPeraPolygon();
	
	MyDirectX12::CreatePiplineStateforPlane();

	MyDirectX12::CreateShadowmap();
	MyDirectX12::CreateShadowmapRootSignature();
	MyDirectX12::CreateShadowmapPiplineState();

	MyDirectX12::SetViewPort();
	MyDirectX12::SetScissorRect();
}


MyDirectX12::~MyDirectX12()
{
}

void MyDirectX12::Update(float angle)
{
	HRESULT result = S_OK;
	float clearColor[4] = { 0.8f, 0.8f, 0.8f, 1.f };

	///////////////////////////////
	//	1�p�X��
	///////////////////////////////
	//���f��
	{

		//�J�����p�萔�o�b�t�@�̍X�V
		*cbuff = wvp;

		//�{�[��������
		std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());
		//PMD��VMD��K��������
		MotionUpdate(static_cast<float>(GetTickCount() - lastTime) / 33.33333f);
		//�{�[���X�V
		std::copy(boneMatrices.begin(), boneMatrices.end(), bBuff);

		//�A���P�[�^���Z�b�g
		result = cmdAllocator->Reset();
		//���X�g���Z�b�g
		result = cmdList->Reset(cmdAllocator, piplineState);

		//�p�C�v���C���̃Z�b�g
		cmdList->SetPipelineState(piplineState);

		//�r���[�|�[�g�̃Z�b�g
		cmdList->RSSetViewports(1, &viewport);
		//�V�U�[���N�g�̃Z�b�g
		cmdList->RSSetScissorRects(1, &scissorRect);

		auto handleDSV = descriptorHeapDSB->GetCPUDescriptorHandleForHeapStart();
		auto handleRTV = descriptorHeapRTV_SP->GetCPUDescriptorHandleForHeapStart();

		cmdList->OMSetRenderTargets(1, &handleRTV, false, &handleDSV);

		cmdList->ClearRenderTargetView(handleRTV, clearColor, 0, nullptr);
		cmdList->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//���[�g�V�O�l�`���̃Z�b�g
		cmdList->SetGraphicsRootSignature(rootSignature);
		//���_�o�b�t�@�̃Z�b�g
		cmdList->IASetVertexBuffers(0, 1, &vbView);
		//�C���f�b�N�X�o�b�t�@�̃Z�b�g
		cmdList->IASetIndexBuffer(&ibView);
		//�V�F�[�_�[���W�X�^�p�f�X�N���v�^�[�q�[�v�̃Z�b�g
		cmdList->SetDescriptorHeaps(1, &rgstDescHeap);
		//�V�F�[�_�[���W�X�^�p�f�X�N���v�^�[�e�[�u���̎w��
		cmdList->SetGraphicsRootDescriptorTable(0, rgstDescHeap->GetGPUDescriptorHandleForHeapStart());

		//�{�[���o�b�t�@�̃Z�b�g
		cmdList->SetDescriptorHeaps(1, &boneDescriptorHeap);
		cmdList->SetGraphicsRootDescriptorTable(2, boneDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		//�g�D�[���o�b�t�@�Z�b�g
		cmdList->SetDescriptorHeaps(1, &toonDescriptorHeap);
		cmdList->SetGraphicsRootDescriptorTable(3, toonDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		//�V���h�E�}�b�v�p�o�b�t�@�Z�b�g
		cmdList->SetDescriptorHeaps(1, &shadowSRVDescriptorHeap);
		cmdList->SetGraphicsRootDescriptorTable(4, shadowSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		//�}�e���A���o�b�t�@�Z�b�g
		cmdList->SetDescriptorHeaps(1, &materialDescHeap);
		auto materialHandle = materialDescHeap->GetGPUDescriptorHandleForHeapStart();
		auto incrementSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		//���C�����X�g
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		int offset = 0;
		for (auto m : pmdmaterials)
		{
			cmdList->SetGraphicsRootDescriptorTable(1, materialHandle);

			materialHandle.ptr += incrementSize * 2;

			auto idxcount = m.faceVertCount;

			cmdList->DrawIndexedInstanced(idxcount, 1, offset, 0, 0);

			offset += idxcount;
		}
	}

	//����
	{
		cmdList->SetGraphicsRootSignature(rootSignature);
		cmdList->SetPipelineState(piplineState_Plane);
		cmdList->IASetVertexBuffers(0, 1, &plane->GetVertexBufferView());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		cmdList->DrawInstanced(4, 1, 0, 0);
	}

	cmdList->Close();
	ExecuteCommand(1);
	WaitWithFence();

	///////////////////////////////
	//	2�p�X�� : �y���|��
	///////////////////////////////
	{
		//�A���P�[�^���Z�b�g
		result = cmdAllocator->Reset();
		//���X�g���Z�b�g
		result = cmdList->Reset(cmdAllocator, piplineState_pera);

		cmdList->SetGraphicsRootSignature(rootSignature_pera);
		cmdList->SetPipelineState(piplineState_pera);

		//�o�b�N�o�b�t�@�C���f�b�N�X
		bbindex = swapChain->GetCurrentBackBufferIndex();

		cmdList->RSSetViewports(1, &viewport);
		cmdList->RSSetScissorRects(1, &scissorRect);

		auto handleRTV = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
		auto handleSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		handleRTV.ptr += bbindex * handleSize;
		cmdList->OMSetRenderTargets(1, &handleRTV, false, nullptr);

		cmdList->IASetVertexBuffers(0, 1, &vbView_pera);

		cmdList->SetDescriptorHeaps(1, &descriptorHeapSRV_SP);
		cmdList->SetGraphicsRootDescriptorTable(0, descriptorHeapSRV_SP->GetGPUDescriptorHandleForHeapStart());

		cmdList->SetDescriptorHeaps(1, &shadowSRVDescriptorHeap);
		cmdList->SetGraphicsRootDescriptorTable(1, shadowSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);
	}

	cmdList->Close();
	ExecuteCommand(1);
	WaitWithFence();


	///////////////////////////////
	//	�V���h�E�}�b�v
	///////////////////////////////
	//{
	//	//�A���P�[�^���Z�b�g
	//	result = cmdAllocator->Reset();
	//	//���X�g���Z�b�g
	//	result = cmdList->Reset(cmdAllocator, shadowPiplineState);

	//	//���[�g�V�O�l�`���̃Z�b�g
	//	cmdList->SetGraphicsRootSignature(shadowRootSignature);
	//	//�p�C�v���C���̃Z�b�g
	//	cmdList->SetPipelineState(shadowPiplineState);

	//	//�r���[�|�[�g�̃Z�b�g
	//	cmdList->RSSetViewports(1, &viewport);
	//	//�V�U�[���N�g�̃Z�b�g
	//	cmdList->RSSetScissorRects(1, &scissorRect);

	//	auto handleDSV = shadowDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//	auto handleRTV = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
	//	auto handleSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//	handleRTV.ptr += bbindex * handleSize;

	//	cmdList->OMSetRenderTargets(1, &handleRTV, false, &handleDSV);

	//	cmdList->ClearRenderTargetView(handleRTV, clearColor, 0, nullptr);
	//	cmdList->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	//	
	//	//���_�o�b�t�@�̃Z�b�g
	//	cmdList->IASetVertexBuffers(0, 1, &vbView);
	//	//�C���f�b�N�X�o�b�t�@�̃Z�b�g
	//	cmdList->IASetIndexBuffer(&ibView);
	//	
	//	cmdList->SetDescriptorHeaps(1, &shadowSRVDescriptorHeap);
	//	cmdList->SetGraphicsRootDescriptorTable(0, shadowSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//	cmdList->SetDescriptorHeaps(1, &rgstDescHeap);
	//	cmdList->SetGraphicsRootDescriptorTable(0, rgstDescHeap->GetGPUDescriptorHandleForHeapStart());

	//	//���C�����X�g
	//	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	cmdList->DrawIndexedInstanced(pmdindices.size() * sizeof(unsigned short), 1, 0, 0, 0);
	//}

	//����
	{
		cmdList->SetGraphicsRootSignature(rootSignature);
		cmdList->SetPipelineState(piplineState_Plane);
		cmdList->IASetVertexBuffers(0, 1, &plane->GetVertexBufferView());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		cmdList->DrawInstanced(4, 1, 0, 0);
	}

	cmdList->Close();
	ExecuteCommand(1);
	WaitWithFence();

	//30f�擾
	if (GetTickCount() - lastTime > vmd->GetDuration() * 33.33333f)
	{
		lastTime = GetTickCount();
	}

	swapChain->Present(1, 0);
}


void MyDirectX12::ExecuteCommand(unsigned int cmdlistnum)
{
	cmdQueue->ExecuteCommandLists(cmdlistnum, (ID3D12CommandList* const*)&cmdList);
	cmdQueue->Signal(fence, ++fenceValue);
}

void MyDirectX12::WaitWithFence()
{
	cmdQueue->Signal(fence, ++fenceValue);
	while (fence->GetCompletedValue() < fenceValue)
	{
		//�҂��̊ԃ��[�v����
	}
}

ID3D12Device * MyDirectX12::GetDevice()
{
	//�f�o�C�X�̎擾
	return dev;
}

ID3D12CommandAllocator * MyDirectX12::GetCommandAllocator()
{
	//�A���P�[�^�̎擾
	return cmdAllocator;
}

ID3D12CommandQueue * MyDirectX12::GetCommandQueue()
{	
	//�L���[�̎擾
	return cmdQueue;
}

ID3D12GraphicsCommandList3 * MyDirectX12::GetCommandList()
{
	//���X�g�̎擾
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

	{
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
		}
	}



	//�A�_�v�^�[�̍쐬
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* adapter = nullptr;
	for (int i = 0; dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(adapter);
		//���̒���NVIDIA�A�_�v�^��T��
		for (auto adpt : adapters) {
			DXGI_ADAPTER_DESC adesc = {};
			adpt->GetDesc(&adesc);
			std::wstring strDesc = adesc.Description;
			if (strDesc.find(L"NVIDIA") != std::string::npos) {//NVIDIA�A�_�v�^������
				adapter = adpt;
				break;
			}
		}
	}

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
		result = D3D12CreateDevice(adapter, lev, IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			level = lev;
			break;
		}
		if (result != S_OK)
		{
			const char* er_title = " CreateDevice�֐����G���[";
			const char* er_message = "S_OK�ȊO���Ԃ���܂���";
			int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
		}
	}
}

void MyDirectX12::CreateCommandAllocator()
{
	//�R�}���h�A���P�[�^�[�̍쐬

	HRESULT result = S_OK;

	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
		IID_PPV_ARGS(&cmdAllocator));
	if (result != S_OK)
	{
		const char* er_title = " CreateCommandAllocator�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateCommandQueue()
{
	//�R�}���h�L���[�̍쐬

	HRESULT result = S_OK;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	result = dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
	if (result != S_OK)
	{
		const char* er_title = " CreateCommandQueue�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
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
	if (result != S_OK)
	{
		const char* er_title = " CreateCommandList�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
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
	if (result != S_OK)
	{
		const char* er_title = " CreateSwapChain�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateDescriptorHeapRTV()
{
	//RTV�p�̃f�X�N���v�^�[�q�[�v�̍쐬

	HRESULT result = S_OK;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.NumDescriptors = screenBufferNum;
	descHeapDesc.NodeMask = 0;

	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descriptorHeapRTV));
	if (result != S_OK)
	{
		const char* er_title = " CreateDescriptorHeapRTV�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateRenderTarget()
{
	//�����_�[�^�[�Q�b�g�̍쐬

	HRESULT result = S_OK;
	int rtvNum = screenBufferNum;
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandleRTV(descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());
	descriptorHandle = descriptorHandleRTV;


	renderTarget.resize(rtvNum);
	descriptorSizeRTV = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < rtvNum; ++i)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget[i]));
		if (result != S_OK)
		{
			const char* er_title = " CreateRenderTarget�֐����G���[";
			const char* er_message = "S_OK�ȊO���Ԃ���܂���";
			int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
		}

		dev->CreateRenderTargetView(renderTarget[i], nullptr, descriptorHandle);
		descriptorHandle.ptr += descriptorSizeRTV;
	}
}

void MyDirectX12::CreateFence()
{
	HRESULT result = S_OK;

	result = dev->CreateFence(fenceValue,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&fence));
	if (result != S_OK)
	{
		const char* er_title = " CreateFence�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::SetViewPort()
{
	//�r���[�|�[�g
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WindowWidth;
	viewport.Height = WindowHeight;
	viewport.MinDepth = 0.f;//�߂�
	viewport.MaxDepth = 1.f;//����

	viewport2.TopLeftX = 0;
	viewport2.TopLeftY = WindowHeight;
	viewport2.Width = WindowWidth;
	viewport2.Height = WindowHeight * 2;
	viewport2.MinDepth = 0.f;//�߂�
	viewport2.MaxDepth = 1.f;//����
}

void MyDirectX12::SetScissorRect()
{
	//�V�U�[���N�g
	scissorRect.left = 0;
	scissorRect.right = WindowWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WindowHeight;
}

void MyDirectX12::CreateTextureBuffer()
{
	HRESULT result = S_OK;

	//�C���[�W�ǂݍ��ݗp�N���X
	LoadImageFile lif;
	std::string filename;

	//�t�@�C���A�h���X�̎擾
	for (auto& mat : pmdmaterials)
	{
		if (std::strlen(mat.textureFileName) > 0)
		{
			filename = lif.SearchImageFile(mat.textureFileName);
		}
	}

	//�摜�f�[�^�̓ǂݍ���
	ImageFileData imgData = lif.Load(filename.c_str());

	{
		D3D12_HEAP_PROPERTIES heapprop = {};
		heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapprop.CreationNodeMask = 1;
		heapprop.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC texResourceDesc = {};
		texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texResourceDesc.Width = imgData.width;//�摜�̉���
		texResourceDesc.Height = imgData.height;//�摜�̏c��
		texResourceDesc.DepthOrArraySize = 1;
		texResourceDesc.MipLevels = 1;
		texResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texResourceDesc.SampleDesc.Count = 1;
		texResourceDesc.SampleDesc.Quality = 0;
		texResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		result = dev->CreateCommittedResource(&heapprop,
			D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
			&texResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureBuffer));
		if (result != S_OK)
		{
			const char* er_title = " CreateTextureBuffer�֐����G���[";
			const char* er_message = "S_OK�ȊO���Ԃ���܂���";
			int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
		}
	}

	{
		//�e�N�X�`���̏�������
		D3D12_RESOURCE_DESC desc = {};
		desc = textureBuffer->GetDesc();
		D3D12_BOX box = {};
		box.left = 0;
		box.right = (desc.Width);
		box.top = 0;
		box.bottom = (desc.Height);
		box.front = 0;
		box.back = 1;
		result = textureBuffer->WriteToSubresource(0, &box, imgData.data.data(), 4 * box.right, imgData.imageSize);
		if (result != S_OK)
		{
			const char* er_title = " CreateTextureBuffer�֐����G���[";
			const char* er_message = "S_OK�ȊO���Ԃ���܂���";
			int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
		}
	}
}

void MyDirectX12::CreateWhiteTextureBuffer()
{
	HRESULT result = S_OK;

	//���e�N�X�`���p
	std::vector<char> whiteTexData(4 * 4 * 4);
	std::fill(whiteTexData.begin(), whiteTexData.end(), 0xff);

	//�o�b�t�@�̐���
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_CUSTOM;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = 4;//�摜�̉���
	desc.Height = 4;//�摜�̏c��
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	result = dev->CreateCommittedResource(&prop,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&whiteTextureBuffer));
	if (result != S_OK)
	{
		const char* er_title = " CreateWhiteTextureBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	result = whiteTextureBuffer->WriteToSubresource(0, nullptr, whiteTexData.data(), 4, 4 * 4);
	if (result != S_OK)
	{
		const char* er_title = " CreateWhiteTextureBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::LoadPMDModelData(const char* _modelFilename)
{
	//pmd(MyVectorStruct)
	FILE* miku_pmd = fopen(_modelFilename, "rb");

	//���f���̃f�[�^�t�H�[�}�b�g����ǂݍ���
	fread(&magic, sizeof(magic), 1, miku_pmd);

	//���f���̏���ǂݍ���
	fread(&pmddata, sizeof(pmddata), 1, miku_pmd);

	//���_�������������m�ۂ���
	pmdvertices.resize(pmddata.vertexNum);

	//���_�ǂݍ���
	{
		for (UINT i = 0; i < pmddata.vertexNum; ++i)
		{
			fread(&pmdvertices[i], sizeof(PMDVertex), 1, miku_pmd);
		}
	}

	//�C���f�b�N�X�ǂݍ���
	//�C���f�b�N�X���̓ǂݍ���
	unsigned int indexNum = 0;
	fread(&indexNum, sizeof(unsigned int), 1, miku_pmd);

	//�C���f�b�N�X�������������m�ۂ���
	pmdindices.resize(indexNum);

	//�e���_���̃C���f�b�N�X����ǂݍ���
	{
		for (int i = 0; i < indexNum; ++i)
		{
			fread(&pmdindices[i], sizeof(unsigned short), 1, miku_pmd);
		}
	}


	//�}�e���A���̓ǂݍ���
	//�}�e���A�����̓ǂݍ���
	materialNum = 0;
	fread(&materialNum, sizeof(unsigned int), 1, miku_pmd);

	//�}�e���A���������������m�ۂ���
	pmdmaterials.resize(materialNum);

	//�}�e���A�������ǂݍ���
	{
		for(int i = 0; i < materialNum; ++i)
		{
			fread(&pmdmaterials[i].diffuse, sizeof(pmdmaterials[i].diffuse), 1, miku_pmd);
			fread(&pmdmaterials[i].alpha, sizeof(pmdmaterials[i].alpha), 1, miku_pmd);
			fread(&pmdmaterials[i].specularity, sizeof(pmdmaterials[i].specularity), 1, miku_pmd);
			fread(&pmdmaterials[i].specularityColor, sizeof(pmdmaterials[i].specularityColor), 1, miku_pmd);
			fread(&pmdmaterials[i].mirror, sizeof(pmdmaterials[i].mirror), 1, miku_pmd);
			fread(&pmdmaterials[i].toonIndex, sizeof(pmdmaterials[i].toonIndex), 1, miku_pmd);
			fread(&pmdmaterials[i].edgeFlag, sizeof(pmdmaterials[i].edgeFlag), 1, miku_pmd);
			fread(&pmdmaterials[i].faceVertCount, sizeof(pmdmaterials[i].faceVertCount), 1, miku_pmd);
			fread(&pmdmaterials[i].textureFileName, sizeof(pmdmaterials[i].textureFileName), 1, miku_pmd);
		}
	}

	//�{�[���ǂݍ���
	//�{�[�����̓ǂݍ���
	fread(&pmdbones.boneNum, sizeof(pmdbones.boneNum), 1, miku_pmd);

	//�{�[���̐������������m�ۂ���
	pmdbones.boneProp.resize(pmdbones.boneNum);

	//�{�[����񕪓ǂݍ���
	{
		for (int i = 0; i < pmdbones.boneNum; ++i)
		{
			fread(&pmdbones.boneProp[i], sizeof(BoneProperty), 1, miku_pmd);
		}
	}


	//IK�ǂݍ���
	//IK�̓ǂݍ���
	unsigned short ikNum = 0;
	fread(&ikNum, sizeof(ikNum), 1, miku_pmd);

	pmdIK.clear();
	pmdIK.resize(ikNum);

	//IK��񕪓ǂݍ���
	{
		for (int i = 0; i < ikNum; ++i)
		{
			fread(&pmdIK[i].ikIndex, sizeof(pmdIK[i].ikIndex), 1, miku_pmd);
			fread(&pmdIK[i].targetBoneIndex, sizeof(pmdIK[i].targetBoneIndex), 1, miku_pmd);
			fread(&pmdIK[i].ikLength, sizeof(pmdIK[i].ikLength), 1, miku_pmd);
			fread(&pmdIK[i].iterations, sizeof(pmdIK[i].iterations), 1, miku_pmd);
			fread(&pmdIK[i].control_weight, sizeof(pmdIK[i].control_weight), 1, miku_pmd);
			pmdIK[i].ikChildrenIndex.resize(pmdIK[i].ikLength);
			for (int j = 0; j < pmdIK[i].ikChildrenIndex.size(); ++j)
			{
				fread(&pmdIK[i].ikChildrenIndex[j], sizeof(unsigned short), 1, miku_pmd);
			}
			
		}
	}

	unsigned short skinNum = 0;
	fread(&skinNum, sizeof(unsigned short), 1, miku_pmd);

	{
		for (int i = 0; i < skinNum; ++i) {
			fseek(miku_pmd, 20, SEEK_CUR);
			unsigned int vertNum = 0;
			fread(&vertNum, sizeof(vertNum), 1, miku_pmd);
			fseek(miku_pmd, 1, SEEK_CUR);
			fseek(miku_pmd, 16 * vertNum, SEEK_CUR);
		}
	}

	unsigned char skinDispNum = 0;
	fread(&skinDispNum, sizeof(skinDispNum), 1, miku_pmd);
	fseek(miku_pmd, skinDispNum * sizeof(unsigned short), SEEK_CUR);

	//�\���p�{�[����
	unsigned char boneDispNum = 0;
	fread(&boneDispNum, sizeof(boneDispNum), 1, miku_pmd);
	fseek(miku_pmd, 50 * boneDispNum, SEEK_CUR);

	//�\���{�[�����X�g
	unsigned int dispBoneNum = 0;
	fread(&dispBoneNum, sizeof(dispBoneNum), 1, miku_pmd);

	fseek(miku_pmd, 3 * dispBoneNum, SEEK_CUR);

	//�p��
	//�p���Ή��t���O
	unsigned char englishFlg = 0;
	fread(&englishFlg, sizeof(englishFlg), 1, miku_pmd);
	if (englishFlg) {
		//���f����20�o�C�g+256�o�C�g�R�����g
		fseek(miku_pmd, 20 + 256, SEEK_CUR);
		//�{�[����20�o�C�g*�{�[����
		fseek(miku_pmd, pmdbones.boneNum * 20, SEEK_CUR);
		//(�\�-1)*20�o�C�g�B-1�Ȃ̂̓x�[�X�����Ԃ�
		fseek(miku_pmd, (skinNum - 1) * 20, SEEK_CUR);
		//�{�[����*50�o�C�g�B
		fseek(miku_pmd, boneDispNum * 50, SEEK_CUR);
	}

	fread(toonTexNames.data(), sizeof(char) * 100, toonTexNames.size(), miku_pmd);

	//�t�@�C�������
	fclose(miku_pmd);

	boneMatrices.resize(pmdbones.boneProp.size());
	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());
	
	auto& mbones = pmdbones.boneProp;
	{
		for (int i = 0; i < pmdbones.boneProp.size(); ++i)
		{
			auto& b = pmdbones.boneProp[i];
			auto& bonenode = boneMap[b.boneName];
			bonenode.boneIdx = i;
			bonenode.startPos = b.boneHeadPos;
			bonenode.endPos = mbones[b.tailPosBoneIndex].boneHeadPos;

		}
		for (auto& b : boneMap)
		{
			if (mbones[b.second.boneIdx].parentBoneIndex >= mbones.size())
			{
				continue;
			}
			auto parentName = mbones[mbones[b.second.boneIdx].parentBoneIndex].boneName;
			boneMap[parentName].children.push_back(&b.second);
		}
	}
}

std::string MyDirectX12::GetToonPathFromIdx(int idx)
{
	auto& toonPaths = toonTexNames;

	std::string path = "resource/img/toon/";

	path = path + toonPaths[idx];

	return path;
}

void MyDirectX12::CreateDescriptorHeapSRVforToon()
{
	HRESULT result;
	auto descNum = pmdmaterials.size();
	//�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC hDesc = {};
	hDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hDesc.NodeMask = 0;
	hDesc.NumDescriptors = descNum;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&hDesc, IID_PPV_ARGS(&toonDescriptorHeap));
}

void MyDirectX12::CreateToonTextureBuffer()
{
	HRESULT result = S_OK;

	//�C���[�W�ǂݍ��ݗp�N���X
	LoadImageFile lif;
	auto size = toonTexNames.size();
	auto handle = toonDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto h_size = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto& mat : pmdmaterials)
	{
		if (mat.toonIndex == 0)
		{
			toonBuffer = InitBuffer(256, 256);

			struct Color {
				Color() :r(0), g(0), b(0), a(0) {}
				Color(unsigned char inr, unsigned char ing, unsigned char inb, unsigned
					char ina) :r(inr), g(ing), b(inb), a(ina) {}
				unsigned char r, g, b, a;
			};

			std::vector<Color> data(4 * 256);
			auto it = data.begin();
			unsigned char brightness = 255;

			for (; it != data.end(); it += 4) {
				std::fill_n(it, 4, Color(brightness, brightness, brightness, 0xff));
				--brightness;
			}

			//�e�N�X�`���̏�������
			D3D12_RESOURCE_DESC desc = {};
			desc = toonBuffer->GetDesc();
			D3D12_BOX box = {};
			box.left = 0;
			box.right = (desc.Width);
			box.top = 0;
			box.bottom = (desc.Height);
			box.front = 0;
			box.back = 1;
			result = toonBuffer->WriteToSubresource(0, nullptr, data.data(), 4 * sizeof(Color), data.size() * sizeof(Color));

			/*cmdList->Close();
			ExecuteCommand(1);
			WaitWithFence();
			cmdAllocator->Reset();
			cmdList->Reset(cmdAllocator, nullptr);*/
		}
		else 
		{
			//auto toonTexture = GetToonPathFromIdx(mat.toonIndex);
			//ImageFileData imgData = lif.Load(toonTexture.c_str());
			ImageFileData imgData = lif.Load("resource/img/toon/toon01.bmp");

			toonBuffer = InitBuffer(imgData.width, imgData.height);

			//�e�N�X�`���̏�������
			D3D12_RESOURCE_DESC desc = {};
			desc = toonBuffer->GetDesc();
			D3D12_BOX box = {};
			box.left = 0;
			box.right = (desc.Width);
			box.top = 0;
			box.bottom = (desc.Height);
			box.front = 0;
			box.back = 1;
			result = toonBuffer->WriteToSubresource(0, &box, imgData.data.data(), 4 * box.right, imgData.imageSize);

			/*cmdList->Close();
			ExecuteCommand(1);
			WaitWithFence();
			cmdAllocator->Reset();
			cmdList->Reset(cmdAllocator, nullptr);*/
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		dev->CreateShaderResourceView(toonBuffer, &desc, handle);
		handle.ptr += h_size;
	}
}

ID3D12Resource* MyDirectX12::InitBuffer(UINT64 _width,UINT64 _height)
{
	HRESULT result;
	ID3D12Resource* retBuffer;

	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_CUSTOM;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = _width;//�摜�̉���
	desc.Height = _height;//�摜�̏c��
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	result = dev->CreateCommittedResource(&prop,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&retBuffer));

	return retBuffer;
}

void MyDirectX12::CreateVertexBuffer()
{
	HRESULT result = S_OK;
	auto size = pmdvertices.size() * sizeof(PMDVertex);
	auto stride = sizeof(PMDVertex);

	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPU����GPU�֓]������
		D3D12_HEAP_FLAG_NONE,//�w��Ȃ�
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptr�ŗǂ�
		IID_PPV_ARGS(&vertexBuffer));
	if (result != S_OK)
	{
		const char* er_title = " CreateVertexBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	PMDVertex* pmdvert = nullptr;
	//map�Œ��_����GPU�ɑ���
	result = vertexBuffer->Map(0, nullptr, (void**)(&pmdvert));
	if (result != S_OK)
	{
		const char* er_title = " CreateVertexBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	std::copy(pmdvertices.begin(), pmdvertices.end(), pmdvert);
	vertexBuffer->Unmap(0, nullptr);

	//���_�o�b�t�@�r���[
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = static_cast<unsigned int>(size);
	vbView.StrideInBytes = stride;
	
}

void MyDirectX12::CreateIndexBuffer()
{
	HRESULT result = S_OK;

	auto size = pmdindices.size() * sizeof(pmdindices[0]);

	ID3D12Resource* indexBuffer = nullptr;
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPU����GPU�֓]������
		D3D12_HEAP_FLAG_NONE,//�w��Ȃ�
		&CD3DX12_RESOURCE_DESC::Buffer(size),//�T�C�Y
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptr�ŗǂ�
		IID_PPV_ARGS(&indexBuffer));
	if (result != S_OK)
	{
		const char* er_title = " CreateIndexBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	unsigned short* idxdata = nullptr;
	result = indexBuffer->Map(0, nullptr, (void**)&idxdata);
	if (result != S_OK)
	{
		const char* er_title = " CreateIndexBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	std::copy(pmdindices.begin(), pmdindices.end(), idxdata);
	indexBuffer->Unmap(0, nullptr);

	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;//�t�H�[�}�b�g(�^�̃T�C�Y(short = 16)�̂���R16)
	ibView.SizeInBytes = size;
}

void MyDirectX12::CreateDepthBuffer()
{
	HRESULT result = S_OK;

	//�o�b�t�@�[����
	D3D12_RESOURCE_DESC descDepth = {};
	descDepth.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	descDepth.Width = WindowWidth;
	descDepth.Height = WindowHeight;
	descDepth.DepthOrArraySize = 1;
	descDepth.MipLevels = 0;
	descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepth.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES depthProperties = {};
	depthProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	depthProperties.CreationNodeMask = 0;
	depthProperties.VisibleNodeMask = 0;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil.Depth = 1.f;
	depthClearValue.DepthStencil.Stencil = 0;

	result = dev->CreateCommittedResource(&depthProperties,
										 D3D12_HEAP_FLAG_NONE,
										 &descDepth,
										 D3D12_RESOURCE_STATE_DEPTH_WRITE,
										 &depthClearValue,
										 IID_PPV_ARGS(&depthBuffer));
	if (result != S_OK)
	{
		const char* er_title = " CreateDepthBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//�[�x�o�b�t�@�p�f�X�N���v�^�[�̐���
	D3D12_DESCRIPTOR_HEAP_DESC descDescriptorHeapDSB = {};
	descDescriptorHeapDSB.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descDescriptorHeapDSB.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descDescriptorHeapDSB.NumDescriptors = 1;
	descDescriptorHeapDSB.NodeMask = 0;

	result = dev->CreateDescriptorHeap(&descDescriptorHeapDSB, IID_PPV_ARGS(&descriptorHeapDSB));
	if (result != S_OK)
	{
		const char* er_title = " CreateDepthBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//�[�x�o�b�t�@�r���[�̐���
	D3D12_DEPTH_STENCIL_VIEW_DESC dbView = {};
	dbView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dbView.Format = DXGI_FORMAT_D32_FLOAT;
	dbView.Texture2D.MipSlice = 0;

	dev->CreateDepthStencilView(depthBuffer,&dbView,descriptorHeapDSB->GetCPUDescriptorHandleForHeapStart());
}

void MyDirectX12::CreateConstantBuffer()
{
	HRESULT result = S_OK;

	size_t size = sizeof(Cbuffer);
	size = (size + 0xff)&~0xff;

	D3D12_HEAP_PROPERTIES cbvHeapProperties = {};
	cbvHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	cbvHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	cbvHeapProperties.CreationNodeMask = 1;
	cbvHeapProperties.VisibleNodeMask = 1;
	cbvHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	result = dev->CreateCommittedResource(&cbvHeapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer));
	if (result != S_OK)
	{
		const char* er_title = " CreateConstantBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//�}�b�v����(�萔�o�b�t�@�̓A�v���P�[�V�����I���܂�unmap���s��Ȃ�)
	result = constantBuffer->Map(0, nullptr, (void**)&cbuff);
	if (result != S_OK)
	{
		const char* er_title = " CreateConstantBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//�萔�o�b�t�@�r���[�̐ݒ�
	D3D12_CONSTANT_BUFFER_VIEW_DESC constdesc = {};
	constdesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	constdesc.SizeInBytes = size;

	auto handle = rgstDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto h_size = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.ptr += h_size;

	//�萔�o�b�t�@�̐���
	dev->CreateConstantBufferView(&constdesc, handle);

	//�A�X�y�N�g��̎Z�o
	auto aspectRatio = 0.5;//static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight);
	
	//�萔�o�b�t�@�p�̃f�[�^�ݒ�
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMFLOAT3 eye(0.f, 15.f, -60.f);
	DirectX::XMFLOAT3 target(0.f, 10.f, 0.f);
	DirectX::XMFLOAT3 up(0.f, 1.f, 0.f);
	DirectX::XMFLOAT3 toLight(-1, 1, -1);
	auto camera = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));
	auto projection = DirectX::XMMatrixPerspectiveLH(DirectX::XM_PIDIV4,
		aspectRatio,
		1.f,
		1000.f);

	/*float mag = ((target.x - eye.x) * (target.x - eye.x)) + ((target.y - eye.y) * (target.y - eye.y)) + ((target.z - eye.z) * (target.z - eye.z));
	DirectX::XMFLOAT3 lightPos = DirectX::XMFLOAT3((toLight.x * mag) , (toLight.y * mag) , (toLight.z * mag));*/

	//�萔�o�b�t�@�p�f�[�^�ɃZ�b�g
	wvp.world = world;
	wvp.viewproj = camera * projection;
	/*wvp.lvp = wvp.world * DirectX::XMMatrixLookAtLH(XMLoadFloat3(&lightPos), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up)) * (DirectX::XMMatrixOrthographicLH(40, 40, 0.1f, 300.0f));*/
	DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(XMLoadFloat3(&toLight), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
	DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicLH(40, 40, 50, 100);
	DirectX::XMMATRIX lightViewProj = lightView * lightProj;
	wvp.lvp = lightViewProj;

	//�萔�o�b�t�@�p�f�[�^�̍X�V
	*cbuff = wvp;
}

void MyDirectX12::CreateMaterialBuffer()
{
	HRESULT result = S_OK;

	size_t size = sizeof(SendMaterialforShader);
	size = (size + 0xff)&~0xff;

	D3D12_HEAP_PROPERTIES materialHeapProperties = {};
	materialHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	materialHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	materialHeapProperties.CreationNodeMask = 1;
	materialHeapProperties.VisibleNodeMask = 1;
	materialHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	int midx = 0;

	material.resize(pmdmaterials.size());
	materialBuffer.resize(pmdmaterials.size());

	for (auto& mbuff : materialBuffer)
	{
		result = dev->CreateCommittedResource(&materialHeapProperties,
			D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mbuff));
		if (result != S_OK)
		{
			const char* er_title = " CreateMaterialBuffer�֐����G���[";
			const char* er_message = "S_OK�ȊO���Ԃ���܂���";
			int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
		}

		MaterialColorRGBA diffuse(
			pmdmaterials[midx].diffuse.x, 
			pmdmaterials[midx].diffuse.y, 
			pmdmaterials[midx].diffuse.z,
			0);

		MaterialColorRGBA specular(
			pmdmaterials[midx].specularityColor.x, 
			pmdmaterials[midx].specularityColor.y, 
			pmdmaterials[midx].specularityColor.z,
			0);

		float ambient = 0.5f;
		MaterialColorRGBA ambientData(ambient, ambient, ambient, ambient);

		SendMaterialforShader sendmat(diffuse, specular, ambientData, pmdmaterials[midx].specularity);

		result = mbuff->Map(0, nullptr, (void**)&material[midx]);

		*material[midx] = sendmat;

		mbuff->Unmap(0, nullptr);

		++midx;
	}

	//�萔�o�b�t�@�r���[�̐ݒ�
	D3D12_CONSTANT_BUFFER_VIEW_DESC materialdesc = {};
	materialdesc.SizeInBytes = size;

	auto handle = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto h_size = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	unsigned int idx = 0;
	for (auto& mbuff : materialBuffer)
	{
		//�o�b�t�@�̏ꏊ���擾
		materialdesc.BufferLocation = mbuff->GetGPUVirtualAddress();

		//�萔�o�b�t�@�̐���
		dev->CreateConstantBufferView(&materialdesc, handle);
		
		//�n���h�������炷
		handle.ptr += h_size;

		//�V�F�[�_���\�[�X�r���[
		if (std::strlen(pmdmaterials[idx].textureFileName) > 0) //�e�N�X�`��������Ƃ�
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = 1;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			dev->CreateShaderResourceView(textureBuffer, &desc, handle);
		}
		else //�e�N�X�`�����Ȃ��Ƃ�
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = 1;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			dev->CreateShaderResourceView(whiteTextureBuffer, &desc, handle);
		}

		//�n���h�������炷
		handle.ptr += h_size;
		++idx;
	}
}

void MyDirectX12::CreateBoneBuffer()
{
	HRESULT result = S_OK;

	size_t size = sizeof(DirectX::XMMATRIX) * boneMatrices.size();
	size = (size + 0xff)&~0xff;

	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&boneBuffer));

	if (result != S_OK)
	{
		const char* er_title = " CreateConstantBuffer�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = boneBuffer->GetGPUVirtualAddress();
	desc.SizeInBytes = size;

	auto handle = boneDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto hsize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//handle.ptr += hsize;

	dev->CreateConstantBufferView(&desc, handle);

	//�}�b�v������
	result = boneBuffer->Map(0, nullptr, (void**)&bBuff);
	//XMMATRIX�����f�[�^�𑗂�
	std::copy(boneMatrices.begin(), boneMatrices.end(), bBuff);
}

void MyDirectX12::CreateDescriptorHeapforBone()
{
	HRESULT result = S_OK;
	//register��CBV,SRV,UAV�p��DescriptorHeapDesc�̐���
	D3D12_DESCRIPTOR_HEAP_DESC registerHeapDesc = {};
	registerHeapDesc.NumDescriptors = 1;
	registerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	registerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = dev->CreateDescriptorHeap(&registerHeapDesc, IID_PPV_ARGS(&boneDescriptorHeap));
	if (result != S_OK)
	{
		const char* er_title = " CreateDescriptorHeapRegister�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateDescriptorHeapRegister()
{
	HRESULT result = S_OK;
	//register��CBV,SRV,UAV�p��DescriptorHeapDesc�̐���
	D3D12_DESCRIPTOR_HEAP_DESC registerHeapDesc = {};
	registerHeapDesc.NumDescriptors = 2;
	registerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	registerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = dev->CreateDescriptorHeap(&registerHeapDesc, IID_PPV_ARGS(&rgstDescHeap));
	if (result != S_OK)
	{
		const char* er_title = " CreateDescriptorHeapRegister�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateDescriptorHeapforMaterial()
{
	HRESULT result = S_OK;

	int descNum = pmdmaterials.size() * 2;

	//Material��CBV,SRV,UAV�p��DescriptorHeapDesc�̐���
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = descNum;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&materialDescHeap));
	if (result != S_OK)
	{
		const char* er_title = " CreateDescriptorHeapforMaterial�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateSamplerState()
{
	//�T���v���[ s[0]
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//���ʂȃt�B���^���g�p���Ȃ�D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//�G���J��Ԃ����(U����)
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//�G���J��Ԃ����(V����)
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(W����)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//MIPMAP����Ȃ�
	samplerDesc.MinLOD = 0.0f;//MIPMAP�����Ȃ�
	samplerDesc.MipLODBias = 0.0f;//MIPMAP�̃o�C�A�X
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�G�b�W�̐F(������)
	samplerDesc.ShaderRegister = 0;//�g�p����V�F�[�_���W�X�^
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�V�F�[�_�[�ɑ΂���f�[�^���J�͈�
	samplerDesc.RegisterSpace = 0;
	samplerDesc.MaxAnisotropy = 0;//.Filter �� Anisotropy �̎��̂ݗL��
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
}

void MyDirectX12::CreateRootParameter()
{
	//�����W�̐ݒ�
	//t[0]
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].BaseShaderRegister = 0;//���W�X�^�ԍ�
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//b[0]
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descriptorRange[1].BaseShaderRegister = 0;//���W�X�^�ԍ�
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//b[1]
	materialRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	materialRange[0].BaseShaderRegister = 1;//���W�X�^�ԍ�
	materialRange[0].NumDescriptors = 1;
	materialRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//t[1]
	materialRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	materialRange[1].BaseShaderRegister = 1;//���W�X�^�ԍ�
	materialRange[1].NumDescriptors = 1;
	materialRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//b[2]
	boneRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	boneRange[0].BaseShaderRegister = 2;//���W�X�^�ԍ�
	boneRange[0].NumDescriptors = 1;
	boneRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//t[2]
	spRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	spRange[0].BaseShaderRegister = 2;//���W�X�^�ԍ�
	spRange[0].NumDescriptors = 1;
	spRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//t[3]
	toonRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	toonRange[0].BaseShaderRegister = 3;//���W�X�^�ԍ�
	toonRange[0].NumDescriptors = 1;
	toonRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//t[4]
	shadowMapRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	shadowMapRange[0].BaseShaderRegister = 4;//���W�X�^�ԍ�
	shadowMapRange[0].NumDescriptors = 1;
	shadowMapRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	//���[�g�p�����[�^�[�̐ݒ�
	//register
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParam[0].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//material
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.NumDescriptorRanges = _countof(materialRange);
	rootParam[1].DescriptorTable.pDescriptorRanges = materialRange;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//bone
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[2].DescriptorTable.NumDescriptorRanges = _countof(boneRange);
	rootParam[2].DescriptorTable.pDescriptorRanges = boneRange;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//seccondpass
	rootParam[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[3].DescriptorTable.NumDescriptorRanges = _countof(spRange);
	rootParam[3].DescriptorTable.pDescriptorRanges = spRange;
	rootParam[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//toon
	rootParam[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[4].DescriptorTable.NumDescriptorRanges = _countof(toonRange);
	rootParam[4].DescriptorTable.pDescriptorRanges = toonRange;
	rootParam[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//shadowMap
	rootParam[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[5].DescriptorTable.NumDescriptorRanges = _countof(shadowMapRange);
	rootParam[5].DescriptorTable.pDescriptorRanges = shadowMapRange;
	rootParam[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
}

void MyDirectX12::CreateRootSignature()
{
	//���[�g�V�O�l�`���̍쐬
	//���[�g�V�O�l�`���Ƃ́A�p�C�v���C���̌^�ɍ����A�v���P�[�V�����ɕK�v�Ȃ��̂�������

	HRESULT result = S_OK;

	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumStaticSamplers = 1;//�T���v���̍��v��
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.NumParameters = _countof(rootParam);//�e�N�X�`���ƒ萔�p�����[�^�̍��v��
	rsDesc.pParameters = rootParam;

	result = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	if (result != S_OK)
	{
		const char* er_title = " CreateRootSignature�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	result = dev->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	if (result != S_OK)
	{
		const char* er_title = " CreateRootSignature�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreatePiplineState()
{
	HRESULT result = S_OK;

	//�V�F�[�_�[
	//���_�V�F�[�_
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "vs", "vs_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, nullptr);
	if (result != S_OK)
	{
		const char* er_title = " CreateShader�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//�s�N�Z���V�F�[�_
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "ps", "ps_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, nullptr);
	if (result != S_OK)
	{
		const char* er_title = " CreateShader�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//�V�F�[�_�֑�����(���_���C�A�E�g)
	D3D12_INPUT_ELEMENT_DESC inputLayouts[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "BONENO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	//�p�C�v���C���X�e�[�g
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//���[�g�V�O�l�`���ƒ��_���C�A�E�g
	gpsDesc.pRootSignature = rootSignature;
	gpsDesc.InputLayout.pInputElementDescs = inputLayouts;
	gpsDesc.InputLayout.NumElements = _countof(inputLayouts);
	//�V�F�[�_
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
	//���X�^���C�U
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//�����_�[�^�[�Q�b�g
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//��v���Ă����K�v������
	//�[�x�X�e���V��
	gpsDesc.DepthStencilState.DepthEnable = true;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//DSV�K�{
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//���̑�
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.NodeMask = 0;
	gpsDesc.SampleDesc.Count = 1;//����
	gpsDesc.SampleDesc.Quality = 0;//����
	gpsDesc.SampleMask = 0xffffffff;//�S��1
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`


	result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&piplineState));
	if (result != S_OK)
	{
		const char* er_title = " CreatePiplineState�֐����G���[";
		const char* er_message = "S_OK�ȊO���Ԃ���܂���";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

/////////////////////////////////////////////
//				VMD�֘A
////////////////////////////////////////////
void MyDirectX12::RecursiveMatrixMultiply(BoneNode & node, DirectX::XMMATRIX & inMat)
{
	boneMatrices[node.boneIdx] *= inMat;
	for (auto& cnode : node.children)
	{
		//assert(node.boneIdx >= 0);
		RecursiveMatrixMultiply(*cnode, boneMatrices[node.boneIdx]);
	}
}

void MyDirectX12::RotateBone(const char* bonename, const DirectX::XMFLOAT4& q, const DirectX::XMFLOAT4& q2, float t)
{
	auto& bonenode = boneMap[bonename];
	DirectX::XMFLOAT3 startPos(bonenode.startPos.x, bonenode.startPos.y, bonenode.startPos.z);
	auto vec = DirectX::XMLoadFloat3(&startPos);
	auto quaternion = XMLoadFloat4(&q);
	auto quaternion2 = XMLoadFloat4(&q2);

	boneMatrices[bonenode.boneIdx] =
		DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorScale(vec, -1))*
		DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionSlerp(quaternion, quaternion2, t))*
		DirectX::XMMatrixTranslationFromVector(vec);
}

void MyDirectX12::MotionUpdate(int _frameNo)
{
	//�|�[�Y���邽�߂̉�]��K��������
	for (auto& anim : animationData)
	{
		auto& localKeyFrames = anim.second;
		auto frameIt = std::find_if(localKeyFrames.rbegin(), localKeyFrames.rend(),
			[_frameNo](const KeyFrame& k) {return k.frameNo <= _frameNo;});

		if (frameIt == localKeyFrames.rend())
		{
			continue;
		}

		auto nextIt = frameIt.base();

		if (nextIt == localKeyFrames.end())
		{
			RotateBone(anim.first.c_str(), frameIt->quaternion);
		}
		else
		{
			float now = static_cast<float>(frameIt->frameNo);

			float next = static_cast<float>(nextIt->frameNo);

			float t = static_cast<float>(_frameNo - now) / (next - now);

			RotateBone(anim.first.c_str(), frameIt->quaternion, nextIt->quaternion, t);
		}
	}

	DirectX::XMMATRIX rootmat = DirectX::XMMatrixIdentity();
	MyDirectX12::RecursiveMatrixMultiply(boneMap["�Z���^�["], rootmat);
}

void MyDirectX12::CreateShadowmap()
{
	HRESULT result;

	//DSV�쐬
	{
		D3D12_RESOURCE_DESC desc = {};

		auto size = RoundupPowerOf2(max(WindowWidth, WindowHeight));

		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.Width = size;
		desc.Height = size;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.MipLevels = 1;
		desc.Alignment = 0;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		//���̃N���A�o�����[���d�v�ȈӖ������̂ō���͍���Ă���
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;

		result = dev->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(&shadowBuffer));
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//�[�x�X�e���V��
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		//�[�x�l�p�f�X�N���v�^�[�q�[�v�̍쐬
		result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&shadowDSVDescriptorHeap));
	}

	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Flags = D3D12_DSV_FLAG_NONE;
		desc.Texture2D.MipSlice = 0;

		auto handle = shadowDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		dev->CreateDepthStencilView(shadowBuffer, &desc, handle);
	}

	//SRV�쐬
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;
		desc.NumDescriptors = 1;

		result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&shadowSRVDescriptorHeap));
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.PlaneSlice = 0;
		desc.Texture2D.MostDetailedMip = 0;
		
		auto handle = shadowSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		//dev->CreateShaderResourceView(shadowBuffer, &desc, handle);
		dev->CreateShaderResourceView(depthBuffer, &desc, handle);
	}

}

void MyDirectX12::CreateShadowmapRootSignature()
{
	D3D12_DESCRIPTOR_RANGE shadowRange[2] = {};

	//���[���h�r���[�v���W�F�N�V����
	shadowRange[0].BaseShaderRegister = 0;
	shadowRange[0].NumDescriptors = 1;
	shadowRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	shadowRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

	//�[�x�o�b�t�@
	shadowRange[1].BaseShaderRegister = 0;
	shadowRange[1].NumDescriptors = 1;
	shadowRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	shadowRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	//���[�g�p�����[�^�̐ݒ�
	D3D12_ROOT_PARAMETER rootParam[2] = {};

	//���[���h�r���[�v���W�F�N�V����
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[0].DescriptorTable.pDescriptorRanges = &shadowRange[0];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//�[�x�o�b�t�@
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[1].DescriptorTable.pDescriptorRanges = &shadowRange[1];
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//�T���v���[
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_DESC rDesc = {};
	rDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rDesc.NumParameters = _countof(rootParam);
	rDesc.pParameters = rootParam;
	rDesc.pStaticSamplers = &samplerDesc;
	rDesc.NumStaticSamplers = 1;

	ID3DBlob * signature = nullptr;
	ID3DBlob * error = nullptr;

	auto result = D3D12SerializeRootSignature(&rDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	//���[�g�V�O�l�`���̐���
	result = dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&shadowRootSignature));
}

void MyDirectX12::CreateShadowmapPiplineState()
{
	ID3DBlob * shadowVS = nullptr;
	ID3DBlob * shadowPS = nullptr;
	ID3DBlob * error = nullptr;

	HRESULT result = S_OK;

	result = D3DCompileFromFile((L"LightShader.hlsl"), nullptr, nullptr, "vs", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shadowVS, &error);
	result = D3DCompileFromFile((L"LightShader.hlsl"), nullptr, nullptr, "ps", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shadowPS, &error);

	//���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayoutDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//�p�C�v���C���X�e�[�g�I�u�W�F�N�g(PSO)�����
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//���[�g�V�O�l�`���ƒ��_���C�A�E�g
	gpsDesc.pRootSignature = shadowRootSignature;
	gpsDesc.InputLayout.pInputElementDescs = inputLayoutDescs;
	gpsDesc.InputLayout.NumElements = _countof(inputLayoutDescs);

	//���_�V�F�[�_
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(shadowVS);

	//�s�N�Z���V�F�[�_
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(shadowPS);

	//���X�^���C�U
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	//�����_�[�^�[�Q�b�g
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//��v�����Ă�����

	//�[�x�X�e���V��
	gpsDesc.DepthStencilState.DepthEnable = true;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;	//COMPAR = ��r, LESS = ��菬����
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	//���̑�
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.NodeMask = 0;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.SampleMask = 0xffffffff;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`

	result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&shadowPiplineState));
}

size_t MyDirectX12::RoundupPowerOf2(size_t size) {
	size_t bit = 0x8000000;
	for (size_t i = 31; i >= 0; --i) {//1���r�b�g��������
		if (size&bit)break;//�����Ă���Ȃ炻���Ŕ�����
		bit >>= 1;
	}
	return bit << 1;//��s����������߂�
}


///////////////////////////////////////////
//				�t�@�[�X�g�p�X
///////////////////////////////////////////

void MyDirectX12::CreateDescriptorHeapRTVforFirstPass()
{
	HRESULT result;
	//�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC hDesc = {};
	hDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	hDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hDesc.NodeMask = 0;
	hDesc.NumDescriptors = 1;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&hDesc, IID_PPV_ARGS(&descriptorHeapRTV_FP));
}

void MyDirectX12::CreateDescriptorHeapSRVforFirstPass()
{
	HRESULT result;
	//�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC hDesc = {};
	hDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hDesc.NodeMask = 0;
	hDesc.NumDescriptors = 1;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&hDesc, IID_PPV_ARGS(&descriptorHeapSRV_FP));
}

void MyDirectX12::CreateRenderTargetforFirstPass()
{
	HRESULT result;
	//�����_�[�^�[�Q�b�g����
	CreateMultiPassResource(firstpassBuffer, true);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(descriptorHeapRTV_FP->GetCPUDescriptorHandleForHeapStart());

	result = swapChain->GetBuffer(0, IID_PPV_ARGS(&firstpassBuffer));
	
	dev->CreateRenderTargetView(firstpassBuffer, nullptr, handle);
}

void MyDirectX12::CreateShaderResourceforFirstPass()
{
	HRESULT result;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	auto handle = descriptorHeapSRV_FP->GetCPUDescriptorHandleForHeapStart();
	auto hsize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	dev->CreateShaderResourceView(firstpassBuffer, &srvDesc, handle);
}

void MyDirectX12::CreateMultiPassResource(ID3D12Resource* _resource, bool rtvflg)
{
	HRESULT result;

	D3D12_HEAP_PROPERTIES hProp = {};
	hProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	hProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	hProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	hProp.CreationNodeMask = 1;
	hProp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Width = WindowWidth;//�摜�̉���
	rDesc.Height = WindowHeight;//�摜�̏c��
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.Flags = rtvflg ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;

	result = dev->CreateCommittedResource(
		&hProp,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&rDesc,
		rtvflg ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr,
		IID_PPV_ARGS(&_resource));
}

///////////////////////////////////////////
//				�Z�J���h�p�X
///////////////////////////////////////////
void MyDirectX12::CreateDescriptorHeapRTVforSecondPass()
{
	HRESULT result;
	//�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC hDesc = {};
	hDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	hDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hDesc.NodeMask = 0;
	hDesc.NumDescriptors = 1;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&hDesc, IID_PPV_ARGS(&descriptorHeapRTV_SP));
}

void MyDirectX12::CreateRenderTargetforSecondPass()
{
	HRESULT result;
	//�����_�[�^�[�Q�b�g����

	D3D12_HEAP_PROPERTIES hProp = {};
	hProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	hProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	hProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	hProp.CreationNodeMask = 1;
	hProp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Width = WindowWidth;//�摜�̉���
	rDesc.Height = WindowHeight;//�摜�̏c��
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	result = dev->CreateCommittedResource(
		&hProp,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&rDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		nullptr,
		IID_PPV_ARGS(&secondpassBuffer));

	dev->CreateRenderTargetView(secondpassBuffer, nullptr, descriptorHeapRTV_SP->GetCPUDescriptorHandleForHeapStart());
}

void MyDirectX12::CreateDescriptorHeapSRVforSecondPass()
{
	HRESULT result;
	//�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC hDesc = {};
	hDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hDesc.NodeMask = 0;
	hDesc.NumDescriptors = 1;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&hDesc, IID_PPV_ARGS(&descriptorHeapSRV_SP));
}

void MyDirectX12::CreateShaderResourceforSecondPass()
{
	HRESULT result;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	auto handle = descriptorHeapSRV_SP->GetCPUDescriptorHandleForHeapStart();

	dev->CreateShaderResourceView(secondpassBuffer, &srvDesc, descriptorHeapSRV_SP->GetCPUDescriptorHandleForHeapStart());
}

///////////////////////////////////////////
//				�y���|���S��
///////////////////////////////////////////
void MyDirectX12::CreateVertexBufferforPeraPolygon()
{
	HRESULT result;
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer_pera));

	//map�Œ��_����GPU�ɑ���
	Vertex* vertex = nullptr;
	result = vertexBuffer_pera->Map(0, nullptr, (void**)(&vertex));
	std::copy(std::begin(vertices), std::end(vertices), vertex);
	vertexBuffer_pera->Unmap(0, nullptr);

	vbView_pera.BufferLocation = vertexBuffer_pera->GetGPUVirtualAddress();
	vbView_pera.SizeInBytes = sizeof(vertices);
	vbView_pera.StrideInBytes = sizeof(Vertex);
}

void MyDirectX12::CreateDescriptorHeapforPeraTexture()
{
	HRESULT result;
	//�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC hDesc = {};
	hDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hDesc.NodeMask = 0;
	hDesc.NumDescriptors = 1;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&hDesc, IID_PPV_ARGS(&peraTextureDescriptorHeap));
}

void MyDirectX12::CreatePeraPolygonTexture()
{
	HRESULT result;

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = 1;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto handle = peraTextureDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	
	dev->CreateShaderResourceView(secondpassBuffer, &desc, handle);
}

void MyDirectX12::CreateSamplerStateforPeraPolygon()
{
	//�T���v���[ s[0]
	samplerDesc_pera.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc_pera.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(U����)
	samplerDesc_pera.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(V����)
	samplerDesc_pera.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(W����)
	samplerDesc_pera.MaxLOD = D3D12_FLOAT32_MAX;//MIPMAP����Ȃ�
	samplerDesc_pera.MinLOD = 0.0f;//MIPMAP�����Ȃ�
	samplerDesc_pera.MipLODBias = 0.0f;//MIPMAP�̃o�C�A�X
	samplerDesc_pera.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�G�b�W�̐F(������)
	samplerDesc_pera.ShaderRegister = 0;//�g�p����V�F�[�_���W�X�^
	samplerDesc_pera.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�V�F�[�_�[�ɑ΂���f�[�^���J�͈�
	samplerDesc_pera.RegisterSpace = 0;
	samplerDesc_pera.MaxAnisotropy = 0;//.Filter �� Anisotropy �̎��̂ݗL��
	samplerDesc_pera.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
}

void MyDirectX12::CreateRootParameterforPeraPolygon()
{
	//�����W�̐ݒ�
	//t[0]
	descriptorRange_pera[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//�V�F�[�_���\�[�X
	descriptorRange_pera[0].BaseShaderRegister = 0;//���W�X�^�ԍ�
	descriptorRange_pera[0].NumDescriptors = 1;
	descriptorRange_pera[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange_pera[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//�V�F�[�_���\�[�X
	descriptorRange_pera[1].BaseShaderRegister = 1;//���W�X�^�ԍ�
	descriptorRange_pera[1].NumDescriptors = 1;
	descriptorRange_pera[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//���[�g�p�����[�^�[�̐ݒ�
	rootParam_pera[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam_pera[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParam_pera[0].DescriptorTable.pDescriptorRanges = &descriptorRange_pera[0];//�Ή����郌���W�ւ̃|�C���^
	rootParam_pera[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam_pera[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam_pera[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam_pera[1].DescriptorTable.pDescriptorRanges = &descriptorRange_pera[1];//�Ή����郌���W�ւ̃|�C���^
	rootParam_pera[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
}

void MyDirectX12::CreateRootSignatureforPeraPolygon()
{
	HRESULT result = S_OK;

	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc_pera;
	rsDesc.NumParameters = _countof(rootParam_pera);
	rsDesc.pParameters = rootParam_pera;

	result = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	result = dev->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_pera));
}

void MyDirectX12::CreatePiplineStateforPeraPolygon()
{
	HRESULT result = S_OK;

	//�V�F�[�_�[
	ID3DBlob* peraVS = nullptr;
	ID3DBlob* peraPS = nullptr;
	//���_�V�F�[�_
	result = D3DCompileFromFile((L"PeraShader.hlsl"), nullptr, nullptr, "PeraVertex", "vs_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &peraVS, nullptr);

	//�s�N�Z���V�F�[�_
	result = D3DCompileFromFile((L"PeraShader.hlsl"), nullptr, nullptr, "PeraPixel", "ps_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &peraPS, nullptr);

	//�V�F�[�_�֑�����(���_���C�A�E�g)
	D3D12_INPUT_ELEMENT_DESC inputLayouts[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	//�p�C�v���C���X�e�[�g
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//���[�g�V�O�l�`���ƒ��_���C�A�E�g
	gpsDesc.pRootSignature = rootSignature_pera;
	gpsDesc.InputLayout.pInputElementDescs = inputLayouts;
	gpsDesc.InputLayout.NumElements = _countof(inputLayouts);
	//�V�F�[�_
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(peraVS);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(peraPS);
	//���X�^���C�U
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//�����_�[�^�[�Q�b�g
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
													   
	gpsDesc.DepthStencilState.DepthEnable = false;//true??
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//���̑�
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.NodeMask = 0;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.SampleMask = 0xffffffff;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;


	result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&piplineState_pera));
}


void MyDirectX12::CreatePiplineStateforPlane()
{
	HRESULT result = S_OK;

	//�V�F�[�_�[
	ID3DBlob* vertexShaderPlane = nullptr;
	ID3DBlob* pixelShaderPlane = nullptr;
	//���_�V�F�[�_
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "PrimitiveVS", "vs_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShaderPlane, nullptr);

	//�s�N�Z���V�F�[�_
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "PrimitivePS", "ps_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShaderPlane, nullptr);

	//�V�F�[�_�֑�����(���_���C�A�E�g)
	D3D12_INPUT_ELEMENT_DESC inputLayouts[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	//�p�C�v���C���X�e�[�g
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//���[�g�V�O�l�`���ƒ��_���C�A�E�g
	gpsDesc.pRootSignature = rootSignature;
	gpsDesc.InputLayout.pInputElementDescs = inputLayouts;
	gpsDesc.InputLayout.NumElements = _countof(inputLayouts);
	//�V�F�[�_
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderPlane);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderPlane);
	//���X�^���C�U
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//�����_�[�^�[�Q�b�g
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	gpsDesc.DepthStencilState.DepthEnable = true;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//���̑�
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.NodeMask = 0;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.SampleMask = 0xffffffff;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;


	result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&piplineState_Plane));
}