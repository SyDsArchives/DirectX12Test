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
};

class MyDirectX12
{
private:
	unsigned int bbindex;
	int descriptorSizeRTV;

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

	//レンダーターゲット
	std::vector<ID3D12Resource*> renderTarget;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
	//CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandleRTV(descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());

	//VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vbView = {};

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

