#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "MyVector_2Dor3D.h"

#include <vector>
#include <functional>

struct Vertex{
	DirectX::XMFLOAT3 pos;//座標
	DirectX::XMFLOAT2 uv;
};

//PMD
struct PMDData {
	float version;
	char modelName[20];
	char comment[256];
	unsigned int vertexNum;//頂点数
};

//DirectXMath
struct t_Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 nomalVector;
	DirectX::XMFLOAT2 uv;
	unsigned short boneNum[2];
	unsigned char boneWeight;
	unsigned char edgeFlag;
};

//MyVector
#pragma pack(1)
struct PMDVertex {
	Vector3f pos;
	Vector3f nomalVector;
	Vector2f uv;
	unsigned short boneNum[2];
	unsigned char boneWeight;
	unsigned char edgeFlag;
};
#pragma pack()


struct Cbuffer {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX viewproj;
	DirectX::XMFLOAT3 diffuse;
	bool existtex;
};

class MyDirectX12
{
private:
	unsigned int bbindex;
	unsigned int descriptorSizeRTV;
	int count;

	HWND hwnd;
	IDXGIFactory6* dxgiFactory;

	//デバイス
	IDXGIAdapter4* adapter;
	ID3D12Device* dev;

	//コマンド系
	ID3D12CommandAllocator* cmdAllocator;
	ID3D12CommandQueue* cmdQueue;
	ID3D12GraphicsCommandList3* cmdList;

	//デスクリプターヒープ
	ID3D12DescriptorHeap* descriptorHeapRTV;

	//スワップチェイン
	IDXGISwapChain4* swapChain;

	//デスクリプターヒープ
	ID3D12DescriptorHeap* descriptorHeap = nullptr;

	//レンダーターゲット
	std::vector<ID3D12Resource*> renderTarget;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
	//CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandleRTV(descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());

	//頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ID3D12Resource* vertexBuffer;

	//インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	//深度バッファ
	D3D12_DEPTH_STENCIL_VIEW_DESC dbView = {};
	ID3D12Resource* depthBuffer;
	ID3D12DescriptorHeap* descriptorHeapDSB;

	//ルートシグネチャ
	ID3D12RootSignature* rootSignature;

	//パイプラインステート
	ID3D12PipelineState* piplineState;

	//ビューポート
	D3D12_VIEWPORT viewport;

	//シザーレクト
	D3D12_RECT scissorRect;

	//フェンス
	ID3D12Fence* fence;
	UINT64 fenceValue;
	void ExecuteCommand(unsigned int cmdlistnum);
	void WaitWithFence();
	//イベント
	HANDLE evhandle;

	//サンプラー
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	//ルートパラメーター
	D3D12_ROOT_PARAMETER rootParam[1] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};

	//テクスチャバッファ
	ID3D12Resource* textureBuffer;

	//定数バッファ 
	ID3D12Resource* constantBuffer;
	Cbuffer wvp = {};
	Cbuffer* cbuff;
	//DirectX::XMMATRIX* mat = nullptr;

	//テクスチャリソース
	/*ID3D12DescriptorHeap* srvDescHeap;*/

	//シェーダーリソースビュー
	ID3D12DescriptorHeap* rtvDescHeap;//RTV(レンダーターゲット)デスクリプタヒープ
	ID3D12DescriptorHeap* dsvDescHeap;//DSV(深度)デスクリプタヒープ
	ID3D12DescriptorHeap* rgstDescHeap;//その他(テクスチャ、定数)デスクリプタヒープ
	
	//PMD関連
	char magic[3];
	PMDData pmddata = {};
	std::vector<PMDVertex> pmdvertices;
	std::vector<unsigned short> pmdindices;


public:
	MyDirectX12(HWND _hwnd);
	~MyDirectX12();
	
	void OutLoopDx12();
	void InLoopDx12(float angle);

	//ファクトリーの作成
	void CreateDXGIFactory();

	//デバイス系
	void CreateDevice();
	ID3D12Device* GetDevice();

	//コマンド系
	void CreateCommandAllocator();
	void CreateCommandQueue();
	void CreateCommandList();
	ID3D12CommandAllocator*	GetCommandAllocator();
	ID3D12CommandQueue* GetCommandQueue();
	ID3D12GraphicsCommandList3* GetCommandList();

	//デスクリプター
	void CreateDescriptorHeapRTV();

	//スワップチェイン
	void CreateSwapChain();

	//レンダーターゲット
	void CreateRenderTarget();

	//ルートシグネチャ
	void CreateRootSignature();

	//フェンス
	void CreateFence();

	//ビューポート
	void SetViewPort();

	//シザーレクト
	void SetScissorRect();
	
	//頂点バッファ
	void CreateVertexBuffer();

	//インデックスバッファ
	void CreateIndexBuffer();

	//深度バッファ
	void CreateDepthBuffer();
	
	//シェーダー
	void CreateShader();

	//レジスタ系デスクリプター
	void CreateDescriptorHeapRegister();

	//テクスチャ
	void CreateTextureBuffer();

	//定数バッファ
	void CreateConstantBuffer();

	//ルートパラメーター
	void CreateRootParameter();

	//パイプラインステート
	void CreatePiplineState();

	//モデル関連
	void LoadPMDModelData();
};

