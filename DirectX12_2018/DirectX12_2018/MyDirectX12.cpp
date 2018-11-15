#include "MyDirectX12.h"
#include "Geometory.h"
#include "d3dx12.h"
#include <iostream>
#include "LoadImageFile.h"
#include "DirectXTex.h"
#include "VMD.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DirectXTex.lib")

const char* fname = "resource/model/miku/�����~�N.pmd";
const char* vmdfile = "resource/vmd/���S�R���_���X.vmd";

//�V�F�[�_�֑�����(���_���C�A�E�g)
D3D12_INPUT_ELEMENT_DESC inputLayouts[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "BONENO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

const int screenBufferNum = 2;//��ʃo�b�t�@�̐�

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
pmx(new PMX()),vmd(new VMD()),lastTime(0)
{
	//pmx->Load();
	vmd->Load(vmdfile);
	animationData = vmd->GetAnimationMapData();
	MyDirectX12::LoadPMDModelData(fname);
	MyDirectX12::CreateDXGIFactory();
	MyDirectX12::CreateDevice();
	MyDirectX12::CreateCommandQueue();
	MyDirectX12::CreateCommandAllocator();
	MyDirectX12::CreateCommandList();
	MyDirectX12::CreateDescriptorHeapRTV();
	MyDirectX12::CreateSwapChain();
	MyDirectX12::CreateRenderTarget();
	MyDirectX12::CreateFence();
	
	MyDirectX12::CreateVertexBuffer();
	MyDirectX12::CreateIndexBuffer();
	MyDirectX12::CreateDepthBuffer();
	MyDirectX12::CreateRootParameter();
	MyDirectX12::CreateRootSignature();
	MyDirectX12::CreateShader();
	MyDirectX12::CreatePiplineState();
	MyDirectX12::CreateDescriptorHeapRegister();
	MyDirectX12::CreateDescriptorHeapforMaterial();
	MyDirectX12::CreateTextureBuffer();
	MyDirectX12::CreateWhiteTextureBuffer();
	MyDirectX12::CreateConstantBuffer();
	MyDirectX12::CreateMaterialBuffer();
	MyDirectX12::CreateBoneBuffer();
	MyDirectX12::SetViewPort();
	MyDirectX12::SetScissorRect();
}


MyDirectX12::~MyDirectX12()
{
}

void MyDirectX12::OutLoopDx12()
{
}

void MyDirectX12::InLoopDx12(float angle)
{
	HRESULT result = S_OK;

	//�萔�o�b�t�@�p�f�[�^�̍X�V(���t���[��)
	//wvp.world = DirectX::XMMatrixRotationY(angle);

	//�J�����p�萔�o�b�t�@�̍X�V
	memcpy(cbuff, &wvp, sizeof(wvp));

	//30f�擾
	if (GetTickCount() - lastTime > vmd->GetDuration() * 33.33333f)
	{
		lastTime = GetTickCount();
	}
	
	//�{�[��������
	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());

	//PMD��VMD��K��������
	MotionUpdate(static_cast<float>(GetTickCount() - lastTime) / 33.33333f);
	
	//�{�[���X�V
	std::copy(boneMatrices.begin(), boneMatrices.end(), bBuff);

	//�w�i�F
	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.f };

	//RTVHandleInclement
	auto handleRTV = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
	handleRTV.ptr += (bbindex * descriptorSizeRTV);

	//DSV
	auto handleDSV = descriptorHeapDSB->GetCPUDescriptorHandleForHeapStart();

	//�A���P�[�^���Z�b�g
	result = cmdAllocator->Reset();
	//���X�g���Z�b�g
	result = cmdList->Reset(cmdAllocator, piplineState);

	//�r���[�|�[�g�̃Z�b�g
	cmdList->RSSetViewports(1, &viewport);

	//�V�U�[���N�g�̃Z�b�g
	cmdList->RSSetScissorRects(1, &scissorRect);

	//�f�v�X�o�b�t�@�̃N���A
	cmdList->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//�����_�[�^�[�Q�b�g�̃Z�b�g
	//�N���A
	cmdList->ClearRenderTargetView(handleRTV, clearColor, 0, nullptr);
	//�����_�[�^�[�Q�b�g�ݒ�
	cmdList->OMSetRenderTargets(1, &handleRTV, true, &handleDSV);

	//���[�g�V�O�l�`���̃Z�b�g
	cmdList->SetGraphicsRootSignature(rootSignature);

	//���_�o�b�t�@�̃Z�b�g
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	//�C���f�b�N�X�o�b�t�@�̃Z�b�g
	cmdList->IASetIndexBuffer(&ibView);

	//�V�F�[�_�[���W�X�^�p�f�X�N���v�^�[�q�[�v�̃Z�b�g
	cmdList->SetDescriptorHeaps(1,&rgstDescHeap);

	//�V�F�[�_�[���W�X�^�p�f�X�N���v�^�[�e�[�u���̎w��
	cmdList->SetGraphicsRootDescriptorTable(0, rgstDescHeap->GetGPUDescriptorHandleForHeapStart());

	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(renderTarget[bbindex],
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	//�p�C�v���C���̃Z�b�g
	cmdList->SetPipelineState(piplineState);

	//�`���̃Z�b�g
	//���C�����X�g
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�l�p
	//cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//�O�p�`�`��
	//cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);//���_���݂̂ł̕`��

	//���_�݂̂̃��f���`��
	//cmdList->DrawInstanced(pmddata.vertexNum, 1, 0, 0);

	//PMD���f���C���f�b�N�X����\��
	//cmdList->DrawIndexedInstanced(pmdindices.size(), 1, 0, 0, 0);

	//�{�[���p�萔�o�b�t�@�̃Z�b�g
	cmdList->SetGraphicsRootConstantBufferView(2, boneBuffer->GetGPUVirtualAddress());

	//�}�e���A���p�f�X�N���v�^�[�q�[�v�̃Z�b�g�ƃ��f���`��
	cmdList->SetDescriptorHeaps(1, &materialDescHeap);

	auto materialHandle = materialDescHeap->GetGPUDescriptorHandleForHeapStart();
	auto incrementSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	int offset = 0;
	for (auto m : pmdmaterials)
	{
		cmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
		
		materialHandle.ptr += incrementSize * 2;

		auto idxcount = m.faceVertCount;
		cmdList->DrawIndexedInstanced(idxcount, 1, offset, 0, 0);
		offset += idxcount;
	}

	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(renderTarget[bbindex],
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	cmdList->Close();//���X�g�̃N���[�Y

	ExecuteCommand(1);

	swapChain->Present(1, 0);

	//�R�}���h�̊�����ҋ@
	WaitWithFence();
}

void MyDirectX12::ExecuteCommand(unsigned int cmdlistnum)
{
	cmdQueue->ExecuteCommandLists(cmdlistnum, (ID3D12CommandList* const*)&cmdList);
	cmdQueue->Signal(fence, ++fenceValue);
}

void MyDirectX12::WaitWithFence()
{
	cmdQueue->Signal(fence, ++fenceValue);
	auto a = fence->GetCompletedValue();

	while (fence->GetCompletedValue() != fenceValue)
	{
		//�҂��̊ԃ��[�v����
	}

	bbindex = swapChain->GetCurrentBackBufferIndex();
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
}

void MyDirectX12::SetScissorRect()
{
	//�V�U�[���N�g
	scissorRect.left = 0;
	scissorRect.right = WindowWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WindowHeight;
}

void MyDirectX12::CreateShader()
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

	//�e�N�X�`���̃t�F���X(�҂�)
	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	cmdList->Close();
	ExecuteCommand(1);
	WaitWithFence();
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

	//�e�N�X�`���̃t�F���X(�҂�)
	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(whiteTextureBuffer,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	cmdList->Close();
	ExecuteCommand(1);
	WaitWithFence();
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
		//&CD3DX12_RESOURCE_DESC::Buffer(size),
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
	auto camera = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));
	auto projection = DirectX::XMMatrixPerspectiveLH(DirectX::XM_PIDIV4,
		aspectRatio,
		1.f,
		1000.f);

	//�萔�o�b�t�@�p�f�[�^�ɃZ�b�g
	wvp.world = world;
	wvp.viewproj = camera * projection;
	
	//�萔�o�b�t�@�p�f�[�^�̍X�V
	//*cbuff = wvp;
	memcpy(cbuff, &wvp, sizeof(wvp));
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

		SendMaterialforShader sendmat(diffuse, specular, ambientData);

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
	

	//�}�b�v������
	result = boneBuffer->Map(0, nullptr, (void**)&bBuff);
	//XMMATRIX�����f�[�^�𑗂�
	std::copy(boneMatrices.begin(), boneMatrices.end(), bBuff);
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

void MyDirectX12::CreateRootParameter()
{
	//�T���v���[ s[0]
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;//���ʂȃt�B���^���g�p���Ȃ�
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(U����)
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(V����)
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

	//�����W�̐ݒ�
	//t[0]
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//�V�F�[�_���\�[�X
	descriptorRange[0].BaseShaderRegister = 0;//���W�X�^�ԍ�
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//b[0]
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//�R���X�^���g�o�b�t�@
	descriptorRange[1].BaseShaderRegister = 0;//���W�X�^�ԍ�
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//b[1]
	materialRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//�R���X�^���g�o�b�t�@
	materialRange[0].BaseShaderRegister = 1;//���W�X�^�ԍ�
	materialRange[0].NumDescriptors = 1;
	materialRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//t[1]
	materialRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//�V�F�[�_���\�[�X
	materialRange[1].BaseShaderRegister = 1;//���W�X�^�ԍ�
	materialRange[1].NumDescriptors = pmdmaterials.size();
	materialRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//b[2]
	//boneRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//�R���X�^���g�o�b�t�@;
	//boneRange[0].BaseShaderRegister = 2;//���W�X�^�ԍ�;
	//boneRange[0].NumDescriptors = 1;
	//boneRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;;
	
	//���[�g�p�����[�^�[�̐ݒ�
	//register
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParam[0].DescriptorTable.pDescriptorRanges = descriptorRange;//�Ή����郌���W�ւ̃|�C���^
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//material
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.NumDescriptorRanges = _countof(materialRange);
	rootParam[1].DescriptorTable.pDescriptorRanges = materialRange;//�Ή����郌���W�ւ̃|�C���^
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//bone
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[2].Descriptor.RegisterSpace = 0;
	rootParam[2].Descriptor.ShaderRegister = 2;//���W�X�^�ԍ�
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
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

//VMD�֘A
////////////////////////////////////////////////////////////////////////////////////
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
			[_frameNo](const KeyFrame& k) {return k.frameNo == _frameNo;});

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
			float now = static_cast<float>(nextIt->frameNo);

			float next = static_cast<float>(nextIt->frameNo);

			float t = static_cast<float>(_frameNo - now) / (next - now);

			RotateBone(anim.first.c_str(), frameIt->quaternion, nextIt->quaternion, t);
		}

		
	}

	DirectX::XMMATRIX rootmat = DirectX::XMMatrixIdentity();
	MyDirectX12::RecursiveMatrixMultiply(boneMap["�Z���^�["], rootmat);
}
