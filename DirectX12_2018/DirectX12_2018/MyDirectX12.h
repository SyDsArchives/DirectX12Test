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
	DirectX::XMFLOAT3 pos;//���W
	DirectX::XMFLOAT2 uv;
};

//PMD
struct PMDData {
	float version;
	char modelName[20];
	char comment[256];
	unsigned int vertexNum;//���_��
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

#pragma pack(1)
struct PMDMaterials {
	Vector3f diffuse;
	float alpha;
	float specularity;
	Vector3f specularityColor;
	Vector3f mirror;
	unsigned char toonIndex;
	unsigned char edgeFlag;
	unsigned int faceVertCount;
	char textureFileName[20];
};
#pragma pack()

struct MaterialColorRGBA
{
	MaterialColorRGBA() {}
	MaterialColorRGBA(float r, float g, float b, float a) :
	red(r),green(g),blue(b),alpha(a) {}
	float red;
	float green;
	float blue;
	float alpha;
};

//�}�e���A�������V�F�[�_�[�ɓn���p�̍\����
struct SendMaterialforShader
{
	SendMaterialforShader() {}
	SendMaterialforShader(MaterialColorRGBA& dif, MaterialColorRGBA& spe, MaterialColorRGBA& amb):
		diffuse(dif) ,specular(spe), ambient(amb) {}
	MaterialColorRGBA diffuse;
	MaterialColorRGBA specular;
	MaterialColorRGBA ambient;
};

struct Cbuffer {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX viewproj;
};

class MyDirectX12
{
private:
	unsigned int bbindex;
	unsigned int descriptorSizeRTV;

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

	//�����_�[�^�[�Q�b�g
	std::vector<ID3D12Resource*> renderTarget;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;

	//���_�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ID3D12Resource* vertexBuffer;

	//�C���f�b�N�X�o�b�t�@
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	//�[�x�o�b�t�@
	D3D12_DEPTH_STENCIL_VIEW_DESC dbView = {};
	ID3D12Resource* depthBuffer;
	ID3D12DescriptorHeap* descriptorHeapDSB;

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
	void ExecuteCommand(unsigned int cmdlistnum);
	void WaitWithFence();

	//�T���v���[
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	//���[�g�p�����[�^�[
	D3D12_ROOT_PARAMETER rootParam[3] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	D3D12_DESCRIPTOR_RANGE materialRange[1] = {};
	D3D12_DESCRIPTOR_RANGE whiteTextureRange[1] = {};

	//�e�N�X�`���o�b�t�@
	ID3D12Resource* textureBuffer;

	//�萔�o�b�t�@ 
	ID3D12Resource* constantBuffer;
	Cbuffer wvp = {};
	Cbuffer* cbuff;

	//�e��f�X�N�q�[�v
	ID3D12DescriptorHeap* rtvDescHeap;//RTV(�����_�[�^�[�Q�b�g)�f�X�N���v�^�q�[�v
	ID3D12DescriptorHeap* dsvDescHeap;//DSV(�[�x)�f�X�N���v�^�q�[�v
	ID3D12DescriptorHeap* rgstDescHeap;//���̑�(�e�N�X�`���A�萔)�f�X�N���v�^�q�[�v
	
	//PMD�֘A
	char magic[3];
	PMDData pmddata = {};
	std::vector<PMDVertex> pmdvertices;
	std::vector<unsigned short> pmdindices;
	std::vector<PMDMaterials> pmdmaterials;
	unsigned int materialNum;

	//�}�e���A���o�b�t�@
	ID3D12DescriptorHeap* materialDescHeap;
	std::vector<ID3D12Resource*> materialBuffer;
	std::vector<SendMaterialforShader*> material;

	//���e�N�X�`��
	ID3D12DescriptorHeap* whiteTextureDescHeap;
	ID3D12Resource* whiteTextureBuffer;

public:
	MyDirectX12(HWND _hwnd);
	~MyDirectX12();
	
	void OutLoopDx12();
	void InLoopDx12(float angle);

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
	void CreateDescriptorHeapRTV();

	//�X���b�v�`�F�C��
	void CreateSwapChain();

	//�����_�[�^�[�Q�b�g
	void CreateRenderTarget();

	//���[�g�V�O�l�`��
	void CreateRootSignature();

	//�t�F���X
	void CreateFence();

	//�r���[�|�[�g
	void SetViewPort();

	//�V�U�[���N�g
	void SetScissorRect();
	
	//���_�o�b�t�@
	void CreateVertexBuffer();

	//�C���f�b�N�X�o�b�t�@
	void CreateIndexBuffer();

	//�[�x�o�b�t�@
	void CreateDepthBuffer();
	
	//�V�F�[�_�[
	void CreateShader();

	//���W�X�^�n�f�X�N���v�^�[
	void CreateDescriptorHeapRegister();

	//�e�N�X�`���o�b�t�@
	void CreateTextureBuffer();

	//���e�N�X�`���o�b�t�@
	void CreateWhiteTextureBuffer();

	//�萔�o�b�t�@
	void CreateConstantBuffer();

	//���[�g�p�����[�^�[
	void CreateRootParameter();

	//�p�C�v���C���X�e�[�g
	void CreatePiplineState();

	//���f���֘A
	void LoadPMDModelData();

	//�}�e���A���o�b�t�@
	void CreateMaterialBuffer();
	void CreateDescriptorHeapforMaterial();
	void PMDMaterialUpdate(std::vector<PMDMaterials>& materials);

	//���e�N�X�`��
	void CreateDescriptorHeapforWhiteTexture();
};

