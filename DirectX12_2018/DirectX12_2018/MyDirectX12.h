#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <vector>
#include <functional>

struct Vertex{
	DirectX::XMFLOAT3 pos;//座標
	DirectX::XMFLOAT2 uv;
};

struct Cbuffer {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX viewproj;
};

class MyDirectX12
{
private:
	unsigned int bbindex;
	int descriptorSizeRTV;
	float angle;
	DirectX::XMMATRIX* m;

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

	//VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vbView = {};

	//indexBuffer
	D3D12_INDEX_BUFFER_VIEW ibView = {};

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
	void ExecuteCommand();
	void WaitWithFence();

	//サンプラー
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	//ルートパラメーター
	D3D12_ROOT_PARAMETER rootParam[2] = {};

	//テクスチャバッファ
	ID3D12Resource* textureBuffer;

	//定数バッファ 
	ID3D12Resource* constantBuffer;
	//DirectX::XMMATRIX* mat = nullptr;

	//テクスチャリソース
	/*ID3D12DescriptorHeap* srvDescHeap;*/

	//シェーダーリソースビュー
	ID3D12DescriptorHeap* rtvDescHeap;//RTV(レンダーターゲット)デスクリプタヒープ
	ID3D12DescriptorHeap* dsvDescHeap;//DSV(深度)デスクリプタヒープ
	ID3D12DescriptorHeap* rgstDescHeap;//その他(テクスチャ、定数)デスクリプタヒープ

	
public:
	MyDirectX12(HWND _hwnd);
	~MyDirectX12();
	
	void OutLoopDx12();
	void InLoopDx12();

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
	void CreateDescriptorHeap();

	//スワップチェイン
	void CreateSwapChain();

	//レンダーターゲット
	void CreateRenderTarget();

	//ルートシグネチャ
	void CreateRootSignature();

	//フェンス
	void CreateFence();

};

