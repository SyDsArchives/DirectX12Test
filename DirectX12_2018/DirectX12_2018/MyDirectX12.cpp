#include "MyDirectX12.h"
#include "Geometory.h"
#include "d3dx12.h"
#include <iostream>


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

//�O�p
//Vertex vertices[] = { { { 0.0f,0.7f,0.0f } },
//					{ { 0.4f,-0.5f,0.0f } },
//					{ { -0.4f,-0.5f,0.0f } } };

//�l�p
Vertex vertices[] = {
	DirectX::XMFLOAT3(-0.5f,-0.9f,0.f),DirectX::XMFLOAT2(0,0),
	DirectX::XMFLOAT3(-0.5f,0.9f,0.f),DirectX::XMFLOAT2(0,1),
	DirectX::XMFLOAT3(0.5f,-0.9f,0.f),DirectX::XMFLOAT2(1,0),
	DirectX::XMFLOAT3(0.5f,0.9f,0.f),DirectX::XMFLOAT2(1,1),
};

std::vector<unsigned short> indices = { 0,1,2,1,3,2 };//�C���f�b�N�X(N)
//std::vector<unsigned short> indices = { 0,1,3,3,2,0 };//�C���f�b�N�X(Z)

const int screenBufferNum = 2;//��ʃo�b�t�@�̐�

MyDirectX12::MyDirectX12(HWND _hwnd) :bbindex(0), descriptorSizeRTV(0), hwnd(_hwnd), dxgiFactory(nullptr), adapter(nullptr), dev(nullptr),
cmdAllocator(nullptr), cmdQueue(nullptr), cmdList(nullptr), descriptorHeapRTV(nullptr), swapChain(nullptr), rootSignature(nullptr),
fence(nullptr), fenceValue(0), piplineState(nullptr), textureBuffer(nullptr), rtvDescHeap(nullptr), dsvDescHeap(nullptr), rgstDescHeap(nullptr)
{
	MyDirectX12::CreateDXGIFactory();
	MyDirectX12::CreateDevice();
	MyDirectX12::CreateCommandQueue();
	MyDirectX12::CreateCommandAllocator();
	MyDirectX12::CreateCommandList();
	MyDirectX12::CreateDescriptorHeap();
	MyDirectX12::CreateSwapChain();
	MyDirectX12::CreateRenderTarget();
	/*MyDirectX12::CreateRootSignature();*/
	MyDirectX12::CreateFence();
}


MyDirectX12::~MyDirectX12()
{
}

void MyDirectX12::OutLoopDx12()
{
	//���C�����[�v�O�ɓ������ޏꏊ

	HRESULT result = S_OK;

	//�o�[�e�b�N�X�o�b�t�@
	ID3D12Resource* vertexBuffer = nullptr;
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPU����GPU�֓]������
		D3D12_HEAP_FLAG_NONE,//�w��Ȃ�
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),//�T�C�Y
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptr�ŗǂ�
		IID_PPV_ARGS(&vertexBuffer));

	//�����W
	D3D12_RANGE range = { 0,0 };
	Vertex* vb = nullptr;
	char* pdata = nullptr;

	result = vertexBuffer->Map(0, &range, (void**)&vb);
	std::copy(std::begin(vertices), std::end(vertices), vb);
	vertexBuffer->Unmap(0, nullptr);

	//���_�o�b�t�@�r���[
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.StrideInBytes = sizeof(Vertex);
	vbView.SizeInBytes = sizeof(vertices);

	//�C���f�b�N�X�o�b�t�@
	ID3D12Resource* indexBuffer = nullptr;
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPU����GPU�֓]������
		D3D12_HEAP_FLAG_NONE,//�w��Ȃ�
		&CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]) * 4),//�T�C�Y
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptr�ŗǂ�
		IID_PPV_ARGS(&indexBuffer));

	unsigned short* idxdata = nullptr;
	D3D12_RANGE indexRange = { 0,0 };
	result = indexBuffer->Map(0, &indexRange, (void**)&idxdata);
	std::copy(indices.begin(), indices.end(), idxdata);
	indexBuffer->Unmap(0, nullptr);

	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;//�t�H�[�}�b�g(�^�̃T�C�Y(short = 16)�̂���R16)
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]) * 4;

	//�V�F�[�_�[
	ID3DBlob* vertexShader = nullptr;
	ID3DBlob* pixelShader = nullptr;
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "vs", "vs_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, nullptr);
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "ps", "ps_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, nullptr);

	//���[�g�V�O�l�`��
	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* error = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	result = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&rootSignatureBlob, &error);

	result = dev->CreateRootSignature(0,
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));

	//�V�F�[�_�֑�����
	D3D12_INPUT_ELEMENT_DESC inputLayouts[] = {
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	//�r���[�|�[�g
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WindowWidth;
	viewport.Height = WindowHeight;
	viewport.MinDepth = 0.f;//�߂�
	viewport.MaxDepth = 1.f;//����

							//�V�U�[���N�g
	scissorRect.left = 0;
	scissorRect.right = WindowWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WindowHeight;

	//�e�N�X�`���̓ǂݍ���
	BITMAPFILEHEADER bmpFileHeader = {};
	BITMAPINFOHEADER bmpInfoHeader = {};

	FILE* tiles;
	std::vector<char> data;
	tiles = fopen("resource/img/tiles.bmp", "rb");
	fread(&bmpFileHeader, sizeof(bmpFileHeader), 1, tiles);
	fread(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, tiles);
	data.resize(bmpInfoHeader.biWidth * bmpInfoHeader.biHeight * 4);
	//���]�ǂݍ��ݖh�~�̂��߈���ǂݍ���
	for (int line = bmpInfoHeader.biHeight - 1; line >= 0; --line)
	{
		for (int count = 0; count < bmpInfoHeader.biWidth * 4; count += 4)
		{
			unsigned int address = line * bmpInfoHeader.biWidth * 4;
			data[address + count] = 0;
			fread(&data[address + count + 1], sizeof(unsigned char), 3, tiles);
		}
	}
	fclose(tiles);

	//���\�[�X�֘A�H�H
	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	heapprop.CreationNodeMask = 1;
	heapprop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC texResourceDesc = {};
	texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texResourceDesc.Alignment = 0;
	texResourceDesc.Width = bmpInfoHeader.biWidth;//�摜�̉���
	texResourceDesc.Height = bmpInfoHeader.biHeight;//�摜�̏c��
	texResourceDesc.DepthOrArraySize = 1;
	texResourceDesc.MipLevels = 0;
	texResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texResourceDesc.SampleDesc.Count = 1;
	texResourceDesc.SampleDesc.Quality = 0;
	texResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	result = dev->CreateCommittedResource(&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&texResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureBuffer));

	//�e�N�X�`���̏�������
	D3D12_RESOURCE_DESC resdesc = {};
	resdesc = textureBuffer->GetDesc();
	D3D12_BOX box = {};
	box.left = 0;
	box.right = (resdesc.Width);
	box.top = 0;
	box.bottom = (resdesc.Height);
	box.front = 0;
	box.back = 1;
	result = textureBuffer->WriteToSubresource(0, &box, &data[0], 4 * box.right, bmpInfoHeader.biSizeImage);

	//�e�N�X�`���̃t�F���X(�҂�)
	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	cmdList->Close();
	ExecuteCommand();

	//�e�N�X�`�����\�[�X
	D3D12_DESCRIPTOR_HEAP_DESC texDescriptorHeapDesc_SRV = {};
	texDescriptorHeapDesc_SRV.NumDescriptors = 1;
	texDescriptorHeapDesc_SRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texDescriptorHeapDesc_SRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = dev->CreateDescriptorHeap(&texDescriptorHeapDesc_SRV, IID_PPV_ARGS(&rgstDescHeap));

	//�V�F�[�_���\�[�X�r���[
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvHandle = rgstDescHeap->GetCPUDescriptorHandleForHeapStart();

	dev->CreateShaderResourceView(textureBuffer, &srvDesc, srvHandle);

	//�T���v���[
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;//���ʂȃt�B���^���g�p���Ȃ�
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(U����)
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(V����)
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�G���J��Ԃ����(W����)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//MIPMAP����Ȃ�
	samplerDesc.MinLOD = 0.0f;//MIPMAP�����Ȃ�
	samplerDesc.MipLODBias = 0.0f;//MIPMAP�̃o�C�A�X
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�G�b�W�̐F(������)
	samplerDesc.ShaderRegister = 0;//�g�p����V�F�[�_���W�X�^
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�ǂ̂��炢�̃f�[�^���V�F�[�_�Ɍ����邩(�S��)
	samplerDesc.RegisterSpace = 0;//0�ł�����
	samplerDesc.MaxAnisotropy = 0;//Filter �� Anisotropy �̎��̂ݗL��
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;


	//�����W
	D3D12_DESCRIPTOR_RANGE descriptorRange = {};
	//�e�N�X�`���p
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//�V�F�[�_���\�[�X
	descriptorRange.BaseShaderRegister = 0;//���W�X�^�ԍ�
	descriptorRange.NumDescriptors = 1;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//���[�g�p�����[�^�[
	//�e�N�X�`���p
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;
	rootParam.DescriptorTable.pDescriptorRanges = &descriptorRange;//�Ή����郌���W�ւ̃|�C���^
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����Q��;

	MyDirectX12::CreateRootSignature();

	//�p�C�v���C���X�e�[�g
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//���[�g�V�O�l�`���ƒ��_���C�A�E�g
	gpsDesc.pRootSignature = rootSignature;
	gpsDesc.InputLayout.pInputElementDescs = inputLayouts;
	gpsDesc.InputLayout.NumElements = sizeof(inputLayouts) / sizeof(D3D12_INPUT_ELEMENT_DESC);;//_countof(inputLayouts);
	//�V�F�[�_
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
	//���X�^���C�U
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//�����_�[�^�[�Q�b�g
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//��v���Ă����K�v������
	//�[�x�X�e���V��
	gpsDesc.DepthStencilState.DepthEnable = false;//���Ƃ�
	gpsDesc.DepthStencilState.StencilEnable = false;//���Ƃ�
	//gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//DSV�K�{
	//gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;//���Ƃ�
	
	//���̑�
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.NodeMask = 0;
	gpsDesc.SampleDesc.Count = 1;//����
	gpsDesc.SampleDesc.Quality = 0;//����
	gpsDesc.SampleMask = 0xffffffff;//�S��1
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`

	result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&piplineState));

}

void MyDirectX12::InLoopDx12()
{
	//���C�����[�v���ɓ������ޏꏊ

	HRESULT result = S_OK;
	float clearColor[4] = { 0, 0, 0, 255 };
	auto heapStartCPU = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
	heapStartCPU.ptr += (bbindex * descriptorSizeRTV);

	result = cmdAllocator->Reset();//�A���P�[�^���Z�b�g
	result = cmdList->Reset(cmdAllocator, piplineState);//���X�g���Z�b�g

	std::vector<std::function<void(void)>> commandlist;
	commandlist.push_back([]() {std::cout << "SetRTV" << std::endl; });
	std::cout << "1" << std::endl;
	commandlist.push_back([]() {std::cout << "ClearRTV" << std::endl; });
	std::cout << "2" << std::endl;
	commandlist.push_back([]() {std::cout << "Close" << std::endl; });
	std::cout << "3" << std::endl;
	//��{���̉��ȍ~�ɒǉ�����
	//CD3DX12_GPU_DESCRIPTOR_HANDLE rtv(heapStartGPU, bbindex, descriptorSizeRTV);

	//�����_�[�^�[�Q�b�g�̃Z�b�g
	cmdList->OMSetRenderTargets(1, &heapStartCPU, false, nullptr);//�����_�[�^�[�Q�b�g�ݒ�
	cmdList->ClearRenderTargetView(descriptorHandle, clearColor, 0, nullptr);//�N���A

	//���[�g�V�O�l�`���̃Z�b�g
	cmdList->SetGraphicsRootSignature(rootSignature);

	//�e�N�X�`���p�f�X�N���v�^�[�q�[�v�̃Z�b�g
	cmdList->SetDescriptorHeaps(1,&rgstDescHeap);

	//�e�N�X�`���p�f�X�N���v�^�[�q�[�v�̎w��
	cmdList->SetGraphicsRootDescriptorTable(0, rgstDescHeap->GetGPUDescriptorHandleForHeapStart());

	//�p�C�v���C���̃Z�b�g
	cmdList->SetPipelineState(piplineState);

	//�r���[�|�[�g�̃Z�b�g
	cmdList->RSSetViewports(1, &viewport);

	//�V�U�[���N�g�̃Z�b�g
	cmdList->RSSetScissorRects(1, &scissorRect);

	//���_�o�b�t�@�̃Z�b�g
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	//�C���f�b�N�X�o�b�t�@�̃Z�b�g
	cmdList->IASetIndexBuffer(&ibView);

	//�`���̃Z�b�g
	//�O�p
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//�l�p
	//cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//�O�p�`�`��
	//cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);//���_���݂̂ł̕`��

	cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(renderTarget[bbindex],
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	cmdList->Close();//���X�g�̃N���[�Y

	for (auto& cmd : commandlist)
	{
		cmd();
	}

	ExecuteCommand();

	swapChain->Present(1, 0);
	bbindex = swapChain->GetCurrentBackBufferIndex();

	WaitWithFence();
}

void MyDirectX12::ExecuteCommand()
{
	cmdQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
	cmdQueue->Signal(fence, ++fenceValue);
}

void MyDirectX12::WaitWithFence()
{
	cmdQueue->Signal(fence, ++fenceValue);
	while (fence->GetCompletedValue() != fenceValue)
	{
		//std::cout << "�ӂ��񂷂���" << std::endl;
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
	//�A�_�v�^�[�̍쐬
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* adapter = nullptr;
	for (int i = 0; dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(adapter);
		//���̒����� NVIDIA �̓z��T��
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
	}
}

void MyDirectX12::CreateCommandAllocator()
{
	//�R�}���h�A���P�[�^�[�̍쐬

	HRESULT result = S_OK;

	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
		IID_PPV_ARGS(&cmdAllocator));
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
	int a = 0;
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
	descriptorSizeRTV = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < rtvNum; ++i)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget[i]));
		dev->CreateRenderTargetView(renderTarget[i], nullptr, descriptorHandleRTV);
		descriptorHandleRTV.Offset(descriptorSizeRTV);
		//descriptorHandleRTV.ptr += descriptorSizeRTV;
	}
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
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.NumParameters = 1;//�e�N�X�`���ƒ萔�p�����[�^�̍��v��
	rsDesc.pParameters = &rootParam;

	result = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	result = dev->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
}

void MyDirectX12::CreateFence()
{
	HRESULT result = S_OK;

	result = dev->CreateFence(fenceValue,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&fence));
}

