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

const char* fname = "resource/model/miku/初音ミク.pmd";
const char* vmdfile = "resource/vmd/ヤゴコロダンス.vmd";

//シェーダへ送る情報(頂点レイアウト)
D3D12_INPUT_ELEMENT_DESC inputLayouts[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "BONENO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

const int screenBufferNum = 2;//画面バッファの数

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

	//定数バッファ用データの更新(毎フレーム)
	//wvp.world = DirectX::XMMatrixRotationY(angle);

	//カメラ用定数バッファの更新
	memcpy(cbuff, &wvp, sizeof(wvp));

	//30f取得
	if (GetTickCount() - lastTime > vmd->GetDuration() * 33.33333f)
	{
		lastTime = GetTickCount();
	}
	
	//ボーン初期化
	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());

	//PMDにVMDを適応させる
	MotionUpdate(static_cast<float>(GetTickCount() - lastTime) / 33.33333f);
	
	//ボーン更新
	std::copy(boneMatrices.begin(), boneMatrices.end(), bBuff);

	//背景色
	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.f };

	//RTVHandleInclement
	auto handleRTV = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
	handleRTV.ptr += (bbindex * descriptorSizeRTV);

	//DSV
	auto handleDSV = descriptorHeapDSB->GetCPUDescriptorHandleForHeapStart();

	//アロケータリセット
	result = cmdAllocator->Reset();
	//リストリセット
	result = cmdList->Reset(cmdAllocator, piplineState);

	//ビューポートのセット
	cmdList->RSSetViewports(1, &viewport);

	//シザーレクトのセット
	cmdList->RSSetScissorRects(1, &scissorRect);

	//デプスバッファのクリア
	cmdList->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//レンダーターゲットのセット
	//クリア
	cmdList->ClearRenderTargetView(handleRTV, clearColor, 0, nullptr);
	//レンダーターゲット設定
	cmdList->OMSetRenderTargets(1, &handleRTV, true, &handleDSV);

	//ルートシグネチャのセット
	cmdList->SetGraphicsRootSignature(rootSignature);

	//頂点バッファのセット
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	//インデックスバッファのセット
	cmdList->IASetIndexBuffer(&ibView);

	//シェーダーレジスタ用デスクリプターヒープのセット
	cmdList->SetDescriptorHeaps(1,&rgstDescHeap);

	//シェーダーレジスタ用デスクリプターテーブルの指定
	cmdList->SetGraphicsRootDescriptorTable(0, rgstDescHeap->GetGPUDescriptorHandleForHeapStart());

	cmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(renderTarget[bbindex],
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	//パイプラインのセット
	cmdList->SetPipelineState(piplineState);

	//形情報のセット
	//ラインリスト
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//四角
	//cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//三角形描画
	//cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);//頂点情報のみでの描画

	//頂点のみのモデル描画
	//cmdList->DrawInstanced(pmddata.vertexNum, 1, 0, 0);

	//PMDモデルインデックス入り表示
	//cmdList->DrawIndexedInstanced(pmdindices.size(), 1, 0, 0, 0);

	//ボーン用定数バッファのセット
	cmdList->SetGraphicsRootConstantBufferView(2, boneBuffer->GetGPUVirtualAddress());

	//マテリアル用デスクリプターヒープのセットとモデル描画
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

	cmdList->Close();//リストのクローズ

	ExecuteCommand(1);

	swapChain->Present(1, 0);

	//コマンドの完了を待機
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
		//待ちの間ループする
	}

	bbindex = swapChain->GetCurrentBackBufferIndex();
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
		if (result != S_OK)
		{
			const char* er_title = " CreateDevice関数内エラー";
			const char* er_message = "S_OK以外が返されました";
			int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
		}
	}
}

void MyDirectX12::CreateCommandAllocator()
{
	//コマンドアロケーターの作成

	HRESULT result = S_OK;

	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
		IID_PPV_ARGS(&cmdAllocator));
	if (result != S_OK)
	{
		const char* er_title = " CreateCommandAllocator関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateCommandQueue()
{
	//コマンドキューの作成

	HRESULT result = S_OK;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	result = dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
	if (result != S_OK)
	{
		const char* er_title = " CreateCommandQueue関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
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
	if (result != S_OK)
	{
		const char* er_title = " CreateCommandList関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
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
	if (result != S_OK)
	{
		const char* er_title = " CreateSwapChain関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
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
	if (result != S_OK)
	{
		const char* er_title = " CreateDescriptorHeapRTV関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
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
		if (result != S_OK)
		{
			const char* er_title = " CreateRenderTarget関数内エラー";
			const char* er_message = "S_OK以外が返されました";
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
		const char* er_title = " CreateFence関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
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

void MyDirectX12::CreateShader()
{
	HRESULT result = S_OK;
	//シェーダー
	//頂点シェーダ
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "vs", "vs_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, nullptr);
	if (result != S_OK)
	{
		const char* er_title = " CreateShader関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//ピクセルシェーダ
	result = D3DCompileFromFile((L"VertexShader.hlsl"), nullptr, nullptr, "ps", "ps_5_0", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, nullptr);
	if (result != S_OK)
	{
		const char* er_title = " CreateShader関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateTextureBuffer()
{
	HRESULT result = S_OK;

	//イメージ読み込み用クラス
	LoadImageFile lif;
	std::string filename;

	//ファイルアドレスの取得
	for (auto& mat : pmdmaterials)
	{
		if (std::strlen(mat.textureFileName) > 0)
		{
			filename = lif.SearchImageFile(mat.textureFileName);
		}
	}

	//画像データの読み込み
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
		texResourceDesc.Width = imgData.width;//画像の横幅
		texResourceDesc.Height = imgData.height;//画像の縦幅
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
			const char* er_title = " CreateTextureBuffer関数内エラー";
			const char* er_message = "S_OK以外が返されました";
			int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
		}
	}

	{
		//テクスチャの書き込み
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
			const char* er_title = " CreateTextureBuffer関数内エラー";
			const char* er_message = "S_OK以外が返されました";
			int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
		}
	}

	//テクスチャのフェンス(待ち)
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

	//白テクスチャ用
	std::vector<char> whiteTexData(4 * 4 * 4);
	std::fill(whiteTexData.begin(), whiteTexData.end(), 0xff);

	//バッファの生成
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_CUSTOM;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = 4;//画像の横幅
	desc.Height = 4;//画像の縦幅
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
		const char* er_title = " CreateWhiteTextureBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	result = whiteTextureBuffer->WriteToSubresource(0, nullptr, whiteTexData.data(), 4, 4 * 4);
	if (result != S_OK)
	{
		const char* er_title = " CreateWhiteTextureBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//テクスチャのフェンス(待ち)
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

	//モデルのデータフォーマット名を読み込む
	fread(&magic, sizeof(magic), 1, miku_pmd);

	//モデルの情報を読み込む
	fread(&pmddata, sizeof(pmddata), 1, miku_pmd);

	//頂点数分メモリを確保する
	pmdvertices.resize(pmddata.vertexNum);

	//頂点読み込み
	{
		for (UINT i = 0; i < pmddata.vertexNum; ++i)
		{
			fread(&pmdvertices[i], sizeof(PMDVertex), 1, miku_pmd);
		}
	}

	//インデックス読み込み
	//インデックス数の読み込み
	unsigned int indexNum = 0;
	fread(&indexNum, sizeof(unsigned int), 1, miku_pmd);

	//インデックス数分メモリを確保する
	pmdindices.resize(indexNum);

	//各頂点毎のインデックス情報を読み込む
	{
		for (int i = 0; i < indexNum; ++i)
		{
			fread(&pmdindices[i], sizeof(unsigned short), 1, miku_pmd);
		}
	}


	//マテリアルの読み込み
	//マテリアル数の読み込み
	materialNum = 0;
	fread(&materialNum, sizeof(unsigned int), 1, miku_pmd);

	//マテリアル数分メモリを確保する
	pmdmaterials.resize(materialNum);

	//マテリアル数分読み込む
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

	//ボーン読み込み
	//ボーン数の読み込み
	fread(&pmdbones.boneNum, sizeof(pmdbones.boneNum), 1, miku_pmd);

	//ボーンの数分メモリを確保する
	pmdbones.boneProp.resize(pmdbones.boneNum);

	//ボーン情報分読み込む
	{
		for (int i = 0; i < pmdbones.boneNum; ++i)
		{
			fread(&pmdbones.boneProp[i], sizeof(BoneProperty), 1, miku_pmd);
		}
	}

	//ファイルを閉じる
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
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPUからGPUへ転送する
		D3D12_HEAP_FLAG_NONE,//指定なし
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptrで良い
		IID_PPV_ARGS(&vertexBuffer));
	if (result != S_OK)
	{
		const char* er_title = " CreateVertexBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	PMDVertex* pmdvert = nullptr;
	//mapで頂点情報をGPUに送る
	result = vertexBuffer->Map(0, nullptr, (void**)(&pmdvert));
	if (result != S_OK)
	{
		const char* er_title = " CreateVertexBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	std::copy(pmdvertices.begin(), pmdvertices.end(), pmdvert);
	vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビュー
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
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPUからGPUへ転送する
		D3D12_HEAP_FLAG_NONE,//指定なし
		&CD3DX12_RESOURCE_DESC::Buffer(size),//サイズ
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptrで良い
		IID_PPV_ARGS(&indexBuffer));
	if (result != S_OK)
	{
		const char* er_title = " CreateIndexBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	unsigned short* idxdata = nullptr;
	result = indexBuffer->Map(0, nullptr, (void**)&idxdata);
	if (result != S_OK)
	{
		const char* er_title = " CreateIndexBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	std::copy(pmdindices.begin(), pmdindices.end(), idxdata);
	indexBuffer->Unmap(0, nullptr);

	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;//フォーマット(型のサイズ(short = 16)のためR16)
	ibView.SizeInBytes = size;
}

void MyDirectX12::CreateDepthBuffer()
{
	HRESULT result = S_OK;

	//バッファー生成
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
		const char* er_title = " CreateDepthBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//深度バッファ用デスクリプターの生成
	D3D12_DESCRIPTOR_HEAP_DESC descDescriptorHeapDSB = {};
	descDescriptorHeapDSB.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descDescriptorHeapDSB.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descDescriptorHeapDSB.NumDescriptors = 1;
	descDescriptorHeapDSB.NodeMask = 0;

	result = dev->CreateDescriptorHeap(&descDescriptorHeapDSB, IID_PPV_ARGS(&descriptorHeapDSB));
	if (result != S_OK)
	{
		const char* er_title = " CreateDepthBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//深度バッファビューの生成
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
		const char* er_title = " CreateConstantBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//マップする(定数バッファはアプリケーション終了までunmapを行わない)
	result = constantBuffer->Map(0, nullptr, (void**)&cbuff);
	if (result != S_OK)
	{
		const char* er_title = " CreateConstantBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	//定数バッファビューの設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC constdesc = {};
	constdesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	constdesc.SizeInBytes = size;

	auto handle = rgstDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto h_size = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.ptr += h_size;

	//定数バッファの生成
	dev->CreateConstantBufferView(&constdesc, handle);

	//アスペクト比の算出
	auto aspectRatio = 0.5;//static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight);
	
	//定数バッファ用のデータ設定
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

	//定数バッファ用データにセット
	wvp.world = world;
	wvp.viewproj = camera * projection;
	
	//定数バッファ用データの更新
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
			const char* er_title = " CreateMaterialBuffer関数内エラー";
			const char* er_message = "S_OK以外が返されました";
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

	//定数バッファビューの設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC materialdesc = {};
	materialdesc.SizeInBytes = size;

	auto handle = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto h_size = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	unsigned int idx = 0;
	for (auto& mbuff : materialBuffer)
	{
		//バッファの場所を取得
		materialdesc.BufferLocation = mbuff->GetGPUVirtualAddress();

		//定数バッファの生成
		dev->CreateConstantBufferView(&materialdesc, handle);
		
		//ハンドルをずらす
		handle.ptr += h_size;

		//シェーダリソースビュー
		if (std::strlen(pmdmaterials[idx].textureFileName) > 0) //テクスチャがあるとき
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = 1;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			dev->CreateShaderResourceView(textureBuffer, &desc, handle);
		}
		else //テクスチャがないとき
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = 1;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			dev->CreateShaderResourceView(whiteTextureBuffer, &desc, handle);
		}

		//ハンドルをずらす
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
		const char* er_title = " CreateConstantBuffer関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
	

	//マップをする
	result = boneBuffer->Map(0, nullptr, (void**)&bBuff);
	//XMMATRIX分ずつデータを送る
	std::copy(boneMatrices.begin(), boneMatrices.end(), bBuff);
}

void MyDirectX12::CreateDescriptorHeapRegister()
{
	HRESULT result = S_OK;
	//registerのCBV,SRV,UAV用のDescriptorHeapDescの生成
	D3D12_DESCRIPTOR_HEAP_DESC registerHeapDesc = {};
	registerHeapDesc.NumDescriptors = 2;
	registerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	registerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = dev->CreateDescriptorHeap(&registerHeapDesc, IID_PPV_ARGS(&rgstDescHeap));
	if (result != S_OK)
	{
		const char* er_title = " CreateDescriptorHeapRegister関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

void MyDirectX12::CreateDescriptorHeapforMaterial()
{
	HRESULT result = S_OK;

	int descNum = pmdmaterials.size() * 2;

	//MaterialのCBV,SRV,UAV用のDescriptorHeapDescの生成
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = descNum;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&materialDescHeap));
	if (result != S_OK)
	{
		const char* er_title = " CreateDescriptorHeapforMaterial関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
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

	//レンジの設定
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
	//b[1]
	materialRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//コンスタントバッファ
	materialRange[0].BaseShaderRegister = 1;//レジスタ番号
	materialRange[0].NumDescriptors = 1;
	materialRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//t[1]
	materialRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//シェーダリソース
	materialRange[1].BaseShaderRegister = 1;//レジスタ番号
	materialRange[1].NumDescriptors = pmdmaterials.size();
	materialRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//b[2]
	//boneRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//コンスタントバッファ;
	//boneRange[0].BaseShaderRegister = 2;//レジスタ番号;
	//boneRange[0].NumDescriptors = 1;
	//boneRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;;
	
	//ルートパラメーターの設定
	//register
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParam[0].DescriptorTable.pDescriptorRanges = descriptorRange;//対応するレンジへのポインタ
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//material
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.NumDescriptorRanges = _countof(materialRange);
	rootParam[1].DescriptorTable.pDescriptorRanges = materialRange;//対応するレンジへのポインタ
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//bone
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[2].Descriptor.RegisterSpace = 0;
	rootParam[2].Descriptor.ShaderRegister = 2;//レジスタ番号
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
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
	rsDesc.NumParameters = _countof(rootParam);//テクスチャと定数パラメータの合計数
	rsDesc.pParameters = rootParam;

	result = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (result != S_OK)
	{
		const char* er_title = " CreateRootSignature関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}

	result = dev->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	if (result != S_OK)
	{
		const char* er_title = " CreateRootSignature関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}


void MyDirectX12::CreatePiplineState()
{
	HRESULT result = S_OK;

	//パイプラインステート
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//ルートシグネチャと頂点レイアウト
	gpsDesc.pRootSignature = rootSignature;
	gpsDesc.InputLayout.pInputElementDescs = inputLayouts;
	gpsDesc.InputLayout.NumElements = _countof(inputLayouts);
	//シェーダ
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
	//ラスタライザ
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//レンダーターゲット
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//一致しておく必要がある
	//深度ステンシル
	gpsDesc.DepthStencilState.DepthEnable = true;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//DSV必須
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//その他
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.NodeMask = 0;
	gpsDesc.SampleDesc.Count = 1;//いる
	gpsDesc.SampleDesc.Quality = 0;//いる
	gpsDesc.SampleMask = 0xffffffff;//全部1
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形


	result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&piplineState));
	if (result != S_OK)
	{
		const char* er_title = " CreatePiplineState関数内エラー";
		const char* er_message = "S_OK以外が返されました";
		int message = MessageBox(hwnd, er_message, er_title, MB_OK | MB_ICONERROR);
	}
}

//VMD関連
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
	//ポーズするための回転を適応させる
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
	MyDirectX12::RecursiveMatrixMultiply(boneMap["センター"], rootmat);
}
