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
	DirectX::XMFLOAT3 pos;//���W
	DirectX::XMFLOAT2 uv;
};

class MyDirectX12
{
private:
	unsigned int bbindex;
	int descriptorSizeRTV;

	HWND hwnd;
	IDXGIFactory6* dxgiFactory;

	//�f�o�C�X
	IDXGIAdapter4* adapter;
	ID3D12Device* dev;

	//�R�}���h�n
	ID3D12CommandAllocator* cmdAllocator;
	ID3D12CommandQueue* cmdQueue;
	ID3D12GraphicsCommandList3* cmdList;

	//�f�X�N���v�^�[�q�[�v
	ID3D12DescriptorHeap* descriptorHeapRTV;

	//�X���b�v�`�F�C��
	IDXGISwapChain4* swapChain;

	//�f�X�N���v�^�[�q�[�v
	ID3D12DescriptorHeap* descriptorHeap = nullptr;

	//�����_�[�^�[�Q�b�g
	std::vector<ID3D12Resource*> renderTarget;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
	//CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandleRTV(descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());

	//VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vbView = {};

	//indexBuffer
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//���[�g�V�O�l�`��
	ID3D12RootSignature* rootSignature;

	//�p�C�v���C���X�e�[�g
	ID3D12PipelineState* piplineState;

	//�r���[�|�[�g
	D3D12_VIEWPORT viewport;

	//�V�U�[���N�g
	D3D12_RECT scissorRect;

	//�t�F���X
	ID3D12Fence* fence;
	UINT64 fenceValue;
	void ExecuteCommand();
	void WaitWithFence();

	//�T���v���[
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	//���[�g�p�����[�^�[
	D3D12_ROOT_PARAMETER rootParam = {};

	//�e�N�X�`���o�b�t�@
	ID3D12Resource* textureBuffer;

	//�e�N�X�`�����\�[�X
	/*ID3D12DescriptorHeap* srvDescHeap;*/

	//�V�F�[�_�[���\�[�X�r���[
	ID3D12DescriptorHeap* rtvDescHeap;//RTV(�����_�[�^�[�Q�b�g)�f�X�N���v�^�q�[�v
	ID3D12DescriptorHeap* dsvDescHeap;//DSV(�[�x)�f�X�N���v�^�q�[�v
	ID3D12DescriptorHeap* rgstDescHeap;//���̑�(�e�N�X�`���A�萔)�f�X�N���v�^�q�[�v

	
public:
	MyDirectX12(HWND _hwnd);
	~MyDirectX12();
	
	void OutLoopDx12();
	void InLoopDx12();

	//�t�@�N�g���[�̍쐬
	void CreateDXGIFactory();

	//�f�o�C�X�n
	void CreateDevice();
	ID3D12Device* GetDevice();

	//�R�}���h�n
	void CreateCommandAllocator();
	void CreateCommandQueue();
	void CreateCommandList();
	ID3D12CommandAllocator*	GetCommandAllocator();
	ID3D12CommandQueue* GetCommandQueue();
	ID3D12GraphicsCommandList3* GetCommandList();

	//�f�X�N���v�^�[
	void CreateDescriptorHeap();

	//�X���b�v�`�F�C��
	void CreateSwapChain();

	//�����_�[�^�[�Q�b�g
	void CreateRenderTarget();

	//���[�g�V�O�l�`��
	void CreateRootSignature();

	//�t�F���X
	void CreateFence();

};

