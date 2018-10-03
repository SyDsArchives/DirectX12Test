#include "MyDirectX12.h"
#include "Geometory.h"
#include "d3dx12.h"
#include <iostream>

#include "DirectXTex.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

#pragma comment(lib,"DirectXTex.lib")

//三角
//Vertex vertices[] = { { { 0.0f,0.7f,0.0f } },
//					{ { 0.4f,-0.5f,0.0f } },
//					{ { -0.4f,-0.5f,0.0f } } };

//四角
//Vertex vertices[] = {
//	DirectX::XMFLOAT3(-0.5f,-0.9f,0.f),DirectX::XMFLOAT2(0,1),
//	DirectX::XMFLOAT3(-0.5f,0.9f,0.f),DirectX::XMFLOAT2(0,0),
//	DirectX::XMFLOAT3(0.5f,-0.9f,0.f),DirectX::XMFLOAT2(1,1),
//	DirectX::XMFLOAT3(0.5f,0.9f,0.f),DirectX::XMFLOAT2(1,0),
//};

std::vector<unsigned short> indices = { 0,1,2,1,3,2 };//インデックス(N)
//std::vector<unsigned short> indices = { 0,1,3,3,2,0 };//インデックス(Z)

//シェーダへ送る情報
D3D12_INPUT_ELEMENT_DESC inputLayouts[] = {
	{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	//{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0},
	//{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
};

//PMD
//struct PMDData {
//	float version;
//	char modelName[20];
//	char comment[256];
//	unsigned int vertexNum;//頂点数
//};
//
//struct t_Vertex {
//	DirectX::XMFLOAT3 pos;
//	DirectX::XMFLOAT3 nomalVector;
//	DirectX::XMFLOAT2 uv;
//	unsigned short boneNum[2];
//	unsigned char boneWeight;
//	unsigned char edgeFlag;
//};38

char vertexSize = 38;
const int screenBufferNum = 2;//画面バッファの数


MyDirectX12::MyDirectX12(HWND _hwnd) :bbindex(0), descriptorSizeRTV(0), hwnd(_hwnd), dxgiFactory(nullptr), adapter(nullptr), dev(nullptr),
cmdAllocator(nullptr), cmdQueue(nullptr), cmdList(nullptr), descriptorHeapRTV(nullptr), swapChain(nullptr), rootSignature(nullptr),
fence(nullptr), fenceValue(0), piplineState(nullptr), textureBuffer(nullptr), rtvDescHeap(nullptr), dsvDescHeap(nullptr), rgstDescHeap(nullptr),
constantBuffer(nullptr), angle(3.14 / 2/*1 * (3.14 / 180)*/),m(nullptr), vertexBuffer(nullptr), vertexShader(nullptr), pixelShader(nullptr), cbuff(nullptr)
{
	/*MyDirectX12::CreateDXGIFactory();
	MyDirectX12::CreateDevice();
	MyDirectX12::CreateCommandQueue();
	MyDirectX12::CreateCommandAllocator();
	MyDirectX12::CreateCommandList();
	MyDirectX12::CreateDescriptorHeap();
	MyDirectX12::CreateSwapChain();
	MyDirectX12::CreateRenderTarget();
	MyDirectX12::CreateFence();
	MyDirectX12::CreateVertexBuffer();
	MyDirectX12::CreateIndexBuffer();
	MyDirectX12::CreateTextureBuffer();
	MyDirectX12::CreateRootParameter();
	MyDirectX12::CreateRootSignature();
	MyDirectX12::CreatePiplineState();
	MyDirectX12::SetViewPort();
	MyDirectX12::SetScissorRect();
	MyDirectX12::CreateConstantBuffer();*/


	MyDirectX12::CreateDXGIFactory();
	MyDirectX12::CreateDevice();
	MyDirectX12::CreateCommandQueue();
	MyDirectX12::CreateCommandAllocator();
	MyDirectX12::CreateCommandList();
	MyDirectX12::CreateDescriptorHeapRTV();
	MyDirectX12::CreateSwapChain();
	MyDirectX12::CreateRenderTarget();
	MyDirectX12::CreateFence();
	MyDirectX12::LoadPMDModelData();
	MyDirectX12::CreateVertexBuffer();
	MyDirectX12::CreateIndexBuffer();
	MyDirectX12::CreateRootParameter();
	MyDirectX12::CreateRootSignature();
	MyDirectX12::CreatePiplineState();
	MyDirectX12::CreateTextureBuffer();
	MyDirectX12::CreateConstantBuffer();
	MyDirectX12::SetViewPort();
	MyDirectX12::SetScissorRect();
	
}


MyDirectX12::~MyDirectX12()
{
}

void MyDirectX12::OutLoopDx12()
{
}

void MyDirectX12::InLoopDx12()
{
	HRESULT result = S_OK;

	//定数バッファ用データの更新(毎フレーム)
	*cbuff = wvp;

	//背景色
	float clearColor[4] = { 1, 0, 1, 1 };

	auto heapStartCPU = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
	heapStartCPU.ptr += (bbindex * descriptorSizeRTV);

	//アロケータリセット
	result = cmdAllocator->Reset();
	//リストリセット
	result = cmdList->Reset(cmdAllocator, piplineState);


	std::vector<std::function<void(void)>> commandlist;
	commandlist.push_back([]() {std::cout << "SetRTV" << std::endl; });
	std::cout << "1" << std::endl;
	commandlist.push_back([]() {std::cout << "ClearRTV" << std::endl; });
	std::cout << "2" << std::endl;
	commandlist.push_back([]() {std::cout << "Close" << std::endl; });
	std::cout << "3" << std::endl;

	//レンダーターゲットのセット
	//レンダーターゲット設定
	cmdList->OMSetRenderTargets(1, &heapStartCPU, false, nullptr);
	//クリア
	cmdList->ClearRenderTargetView(descriptorHandle, clearColor, 0, nullptr);

	//ルートシグネチャのセット
	cmdList->SetGraphicsRootSignature(rootSignature);

	//定数バッファのセット
	//cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress());

	//シェーダーレジスタ用デスクリプターヒープのセット
	cmdList->SetDescriptorHeaps(1,&rgstDescHeap);

	//シェーダーレジスタ用デスクリプターヒープの指定
	cmdList->SetGraphicsRootDescriptorTable(0, rgstDescHeap->GetGPUDescriptorHandleForHeapStart());

	//パイプラインのセット
	cmdList->SetPipelineState(piplineState);

	//ビューポートのセット
	cmdList->RSSetViewports(1, &viewport);

	//シザーレクトのセット
	cmdList->RSSetScissorRects(1, &scissorRect);

	//頂点バッファのセット
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	//インデックスバッファのセット
	//cmdList->IASetIndexBuffer(&ibView);

	//形情報のセット
	//ラインリスト
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	//四角
	//cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//三角形描画
	//cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);//頂点情報のみでの描画

	//cmdList->DrawIndexedInstanced(8, 1, 0, 0, 0);

	//頂点のみのモデル描画
	cmdList->DrawInstanced(pmddata.vertexNum, 1, 0, 0 );

	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(renderTarget[bbindex],
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	cmdList->Close();//リストのクローズ

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
		//std::cout << "ふぇんすだよ" << std::endl;
	}
}


ID3D12Device * MyDirectX12::GetDevice()
{
	//デバイスの取得
	return dev;
}

ID3D12CommandAllocator * MyDirectX12::GetCommandAllocator()
{
	//アロケータの取得
	return cmdAllocator;
}

ID3D12CommandQueue * MyDirectX12::GetCommandQueue()
{	
	//キューの取得
	return cmdQueue;
}

ID3D12GraphicsCommandList3 * MyDirectX12::GetCommandList()
{
	//リストの取得
	return cmdList;
}

void MyDirectX12::CreateDXGIFactory()
{
	//ファクトリーの作成
	
	HRESULT result = S_OK;
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
}

void MyDirectX12::CreateDevice()
{
	//アダプターの作成
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* adapter = nullptr;
	for (int i = 0; dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(adapter);
		//この中でNVIDIAアダプタを探す
		for (auto adpt : adapters) {
			DXGI_ADAPTER_DESC adesc = {};
			adpt->GetDesc(&adesc);
			std::wstring strDesc = adesc.Description;
			if (strDesc.find(L"NVIDIA") != std::string::npos) {//NVIDIAアダプタを強制
				adapter = adpt;
				break;
			}
		}
	}

	//デバイスの作成

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
	//コマンドアロケーターの作成

	HRESULT result = S_OK;

	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
		IID_PPV_ARGS(&cmdAllocator));
}

void MyDirectX12::CreateCommandQueue()
{
	//コマンドキューの作成

	HRESULT result = S_OK;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	result = dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
}

void MyDirectX12::CreateCommandList()
{
	//コマンドリストの作成

	HRESULT result = S_OK;
	int nodemask = 0;

	result = dev->CreateCommandList(nodemask,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator,
		nullptr,
		IID_PPV_ARGS(&cmdList));
}

void MyDirectX12::CreateDescriptorHeapRTV()
{
	//RTV用のデスクリプターヒープの作成

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
	//スワップチェインの作成

	HRESULT result = S_OK;
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
}

void MyDirectX12::CreateRenderTarget()
{
	//レンダーターゲットの作成

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
		//descriptorHandle.ptr += descriptorSizeRTV;
		//descriptorHandleRTV.ptr += descriptorSizeRTV;
	}
}

void MyDirectX12::CreateFence()
{
	HRESULT result = S_OK;

	result = dev->CreateFence(fenceValue,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&fence));
}

void MyDirectX12::SetViewPort()
{
	//ビューポート
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WindowWidth;
	viewport.Height = WindowHeight;
	viewport.MinDepth = 0.f;//近い
	viewport.MaxDepth = 1.f;//遠い
}

void MyDirectX12::SetScissorRect()
{
	//シザーレクト
	scissorRect.left = 0;
	scissorRect.right = WindowWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WindowHeight;
}

void MyDirectX12::CreateVertexBuffer()
{
	HRESULT result = S_OK;

	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPUからGPUへ転送する
		D3D12_HEAP_FLAG_NONE,//指定なし
		//&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),//サイズ
		&CD3DX12_RESOURCE_DESC::Buffer(vertex_t.size()),
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptrで良い
		IID_PPV_ARGS(&vertexBuffer));

	//result = dev->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPUからGPUへ転送する
	//	D3D12_HEAP_FLAG_NONE,//指定なし
	//						 //&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),//サイズ
	//	&CD3DX12_RESOURCE_DESC::Buffer(sizeof(pmddata.vertexNum)),
	//	D3D12_RESOURCE_STATE_GENERIC_READ,//???
	//	nullptr,//nullptrで良い
	//	IID_PPV_ARGS(&vertexBuffer));

	auto a = vertex_t.end();

	//レンジ
	D3D12_RANGE range = { 0,0 };
	//Vertex* vb = nullptr;
	//unsigned char* pdata = nullptr;
	t_Vertex* tv = nullptr;

	result = vertexBuffer->Map(0, &range, (void**)&tv);
	/*std::copy(std::begin(vertices), std::end(vertices), vb);*/
	std::copy(vertex_t.begin(), vertex_t.end(), tv);
	vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビュー
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.StrideInBytes = vertexSize;
	vbView.SizeInBytes = vertex_t.size();

	//vbView.StrideInBytes = sizeof(Vertex);
	//vbView.SizeInBytes = sizeof(vertices);
}

void MyDirectX12::CreateIndexBuffer()
{
	HRESULT result = S_OK;

	ID3D12Resource* indexBuffer = nullptr;
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPUからGPUへ転送する
		D3D12_HEAP_FLAG_NONE,//指定なし
		&CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]) * 4),//サイズ
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptrで良い
		IID_PPV_ARGS(&indexBuffer));

	unsigned short* idxdata = nullptr;
	D3D12_RANGE indexRange = { 0,0 };
	result = indexBuffer->Map(0, &indexRange, (void**)&idxdata);
	std::copy(indices.begin(), indices.end(), idxdata);
	indexBuffer->Unmap(0, nullptr);

	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;//フォーマット(型のサイズ(short = 16)のためR16)
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]) * 4;

	//シェーダー
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "vs", "vs_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, nullptr);
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "ps", "ps_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, nullptr);
}

void MyDirectX12::CreateTextureBuffer()
{
	HRESULT result = S_OK;

	//bmp
	//テクスチャの読み込み
	//BITMAPFILEHEADER bmpFileHeader = {};
	//BITMAPINFOHEADER bmpInfoHeader = {};

	//FILE* tiles;
	//std::vector<char> data;
	//tiles = fopen("resource/img/tiles.bmp", "rb");
	//fread(&bmpFileHeader, sizeof(bmpFileHeader), 1, tiles);
	//fread(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, tiles);
	//data.resize(bmpInfoHeader.biWidth * bmpInfoHeader.biHeight * 4);
	////反転読み込み防止のため一つずつ読み込む
	//for (int line = bmpInfoHeader.biHeight - 1; line >= 0; --line)
	//{
	//	for (int count = 0; count < bmpInfoHeader.biWidth * 4; count += 4)
	//	{
	//		unsigned int address = line * bmpInfoHeader.biWidth * 4;
	//		data[address + count] = 0;
	//		fread(&data[address + count + 1], sizeof(unsigned char), 3, tiles);
	//	}
	//}
	//fclose(tiles);

	//png
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage img;
	auto res = DirectX::LoadFromWICFile(L"resource/img/TrustHeart.png", 0, &metadata, img);

	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	heapprop.CreationNodeMask = 1;
	heapprop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC texResourceDesc = {};
	texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texResourceDesc.Width = metadata.width;//画像の横幅
	texResourceDesc.Height = metadata.height;//画像の縦幅
	texResourceDesc.DepthOrArraySize = metadata.arraySize;
	texResourceDesc.MipLevels = metadata.mipLevels;
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

	//テクスチャの書き込みpng
	D3D12_RESOURCE_DESC resdesc = {};
	resdesc = textureBuffer->GetDesc();
	D3D12_BOX box = {};
	box.left = 0;
	box.right = (resdesc.Width);
	box.top = 0;
	box.bottom = (resdesc.Height);
	box.front = 0;
	box.back = 1;
	result = textureBuffer->WriteToSubresource(0, &box, img.GetPixels(), 4 * box.right, img.GetPixelsSize());

	////テクスチャの書き込みbmp
	//D3D12_RESOURCE_DESC resdesc = {};
	//resdesc = textureBuffer->GetDesc();
	//D3D12_BOX box = {};
	//box.left = 0;
	//box.right = (resdesc.Width);
	//box.top = 0;
	//box.bottom = (resdesc.Height);
	//box.front = 0;
	//box.back = 1;
	//result = textureBuffer->WriteToSubresource(0, &box, &data[0], 4 * box.right, bmpInfoHeader.biSizeImage);

	//テクスチャのフェンス(待ち)
	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	cmdList->Close();
	ExecuteCommand();

	//register用のDescriptorHeapDescの生成
	D3D12_DESCRIPTOR_HEAP_DESC texDescriptorHeapDesc = {};
	texDescriptorHeapDesc.NumDescriptors = 2;
	texDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = dev->CreateDescriptorHeap(&texDescriptorHeapDesc, IID_PPV_ARGS(&rgstDescHeap));

	//シェーダリソースビュー
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvHandle = rgstDescHeap->GetCPUDescriptorHandleForHeapStart();

	dev->CreateShaderResourceView(textureBuffer, &srvDesc, srvHandle);
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
    desc.Width = (sizeof(Cbuffer) + 0xff)&~0xff;
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

	//定数バッファビューの設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC constdesc = {};
	constdesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	constdesc.SizeInBytes = size;
	auto handle = rgstDescHeap->GetCPUDescriptorHandleForHeapStart();

	//定数バッファの生成
	dev->CreateConstantBufferView(&constdesc, handle);

	handle.ptr += dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//マップする(定数バッファはアプリケーション終了までunmapを行わない)
	D3D12_RANGE cbRange = {0,0};
	result = constantBuffer->Map(0, &cbRange, (void**)&cbuff);

	//アスペクト比の算出
	auto aspectRatio = static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight);

	//定数バッファ用のデータ設定
	DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
	DirectX::XMFLOAT3 eye(0, 0, -10.f);
	DirectX::XMFLOAT3 target(0, 0, 0);
	DirectX::XMFLOAT3 up(0, 1, 0);
	auto camera = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye), 
											XMLoadFloat3(&target), 
											XMLoadFloat3(&up));
	auto projection = DirectX::XMMatrixPerspectiveLH(DirectX::XM_PIDIV2, 
													 aspectRatio, 
													 0.1f, 
													 300.f);
	//定数バッファ用データにセット
	wvp.world = mat;
	wvp.viewproj = camera * projection;
	
	//定数バッファ用データの更新
	*cbuff = wvp;
}

void MyDirectX12::CreateRootParameter()
{
	//サンプラー s[0]
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;//特別なフィルタを使用しない
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//絵が繰り返される(U方向)
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//絵が繰り返される(V方向)
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//絵が繰り返される(W方向)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//MIPMAP上限なし
	samplerDesc.MinLOD = 0.0f;//MIPMAP下限なし
	samplerDesc.MipLODBias = 0.0f;//MIPMAPのバイアス
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//エッジの色(黒透明)
	samplerDesc.ShaderRegister = 0;//使用するシェーダレジスタ
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//シェーダーに対するデータ公開範囲
	samplerDesc.RegisterSpace = 0;
	samplerDesc.MaxAnisotropy = 0;//.Filter が Anisotropy の時のみ有効
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	//レンジ
	//t[0]
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//シェーダリソース
	descriptorRange[0].BaseShaderRegister = 0;//レジスタ番号
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//b[0]
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//コンスタントバッファ
	descriptorRange[1].BaseShaderRegister = 0;//レジスタ番号
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメーター
	//t[0]
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0];//対応するレンジへのポインタ
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	//b[0]
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[1].DescriptorTable.pDescriptorRanges = &descriptorRange[1];//対応するレンジへのポインタ
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

}

void MyDirectX12::CreateRootSignature()
{
	//ルートシグネチャの作成
	//ルートシグネチャとは、パイプラインの型に合うアプリケーションに必要なものを作るもの

	HRESULT result = S_OK;

	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumStaticSamplers = 1;//サンプラの合計数
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.NumParameters = 2;//テクスチャと定数パラメータの合計数
	rsDesc.pParameters = rootParam;

	result = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	result = dev->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
}


void MyDirectX12::CreatePiplineState()
{
	HRESULT result = S_OK;

	//パイプラインステート
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//ルートシグネチャと頂点レイアウト
	gpsDesc.pRootSignature = rootSignature;
	gpsDesc.InputLayout.pInputElementDescs = inputLayouts;
	gpsDesc.InputLayout.NumElements = sizeof(inputLayouts) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	//シェーダ
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
	//ラスタライザ
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//レンダーターゲット
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//一致しておく必要がある
	//深度ステンシル
	gpsDesc.DepthStencilState.DepthEnable = false;//あとで
	gpsDesc.DepthStencilState.StencilEnable = false;//あとで
	//gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//DSV必須
	//gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;//あとで

	//その他
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.NodeMask = 0;
	gpsDesc.SampleDesc.Count = 1;//いる
	gpsDesc.SampleDesc.Quality = 0;//いる
	gpsDesc.SampleMask = 0xffffffff;//全部1
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形


	result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&piplineState));
}

void MyDirectX12::LoadPMDModelData()
{
	//pmd
	FILE* miku_pmd = fopen("resource/model/miku/初音ミク.pmd", "rb");

	fread(&magic, sizeof(magic), 1, miku_pmd);
	fread(&pmddata, sizeof(pmddata), 1, miku_pmd);

	//std::vector<char> pmdvertices(pmddata.vertexNum * vertexSize);
	//fread(&pmdvertices[0], pmdvertices.size(), 1, miku_pmd);

	/*pmdvertices.resize(pmddata.vertexNum  * vertexSize);
	fread(pmdvertices.data(), pmdvertices.size(), 1, miku_pmd);*/
	vertex_t.resize(pmddata.vertexNum * vertexSize);
	fread(&vertex_t[0], vertex_t.size(), 1, miku_pmd);

	fclose(miku_pmd);
}

