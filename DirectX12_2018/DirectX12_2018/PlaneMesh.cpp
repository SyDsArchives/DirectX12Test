#include "PlaneMesh.h"
#include <array>
#include "d3dx12.h"
#include "DirectXTex.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DirectXTex.lib")

PlaneMesh::PlaneMesh(ID3D12Device* _dev, const DirectX::XMFLOAT3 _pos, float _width, float _depth):
	dev(_dev),pos(_pos),width(_width),depth(_depth)
{
	PlaneMesh::PlaneVertexBuffer();
}

PlaneMesh::~PlaneMesh()
{
}

void PlaneMesh::Update()
{
}

void PlaneMesh::PlaneVertexBuffer()
{
	std::array<PrimVertex, 4> vertices{ {
	{ DirectX::XMFLOAT3(pos.x - width / 2,pos.y,pos.z - depth / 2),DirectX::XMFLOAT3(0,1,0),DirectX::XMFLOAT2(0,0) },
	{ DirectX::XMFLOAT3(pos.x - width / 2,pos.y,pos.z + depth / 2),DirectX::XMFLOAT3(0,1,0),DirectX::XMFLOAT2(0,1) },
	{ DirectX::XMFLOAT3(pos.x + width / 2,pos.y,pos.z - depth / 2),DirectX::XMFLOAT3(0,1,0),DirectX::XMFLOAT2(1,0) },
	{ DirectX::XMFLOAT3(pos.x + width / 2,pos.y,pos.z + depth / 2),DirectX::XMFLOAT3(0,1,0),DirectX::XMFLOAT2(1,1) }
	} };


	HRESULT result = S_OK;

	auto size = sizeof(vertices);
	auto stride = sizeof(PrimVertex);

	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//CPUからGPUへ転送する
		D3D12_HEAP_FLAG_NONE,//指定なし
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,//???
		nullptr,//nullptrで良い
		IID_PPV_ARGS(&vertexBuffer));


	PrimVertex* vert = nullptr;
	//mapで頂点情報をGPUに送る
	result = vertexBuffer->Map(0, nullptr, (void**)(&vert));


	std::copy(vertices.begin(), vertices.end(), vert);
	vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビュー
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = static_cast<unsigned int>(size);
	vbView.StrideInBytes = stride;
}

void PlaneMesh::PlaneDescriptorHeap()
{

}

void PlaneMesh::CreateRootParam()
{

}

void PlaneMesh::CreateSampler()
{
}

void PlaneMesh::CreateRootSignature()
{
}

void PlaneMesh::CreatePiplineState()
{
}

