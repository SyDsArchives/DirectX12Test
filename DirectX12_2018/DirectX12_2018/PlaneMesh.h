#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "MyVector_2Dor3D.h"
#include <vector>

struct PrimVertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;

		PrimVertex() {
		pos = DirectX::XMFLOAT3(0, 0, 0);
		normal = DirectX::XMFLOAT3(0, 0, 0);
		uv = DirectX::XMFLOAT2(0, 0);
	}
	PrimVertex(DirectX::XMFLOAT3 p, DirectX::XMFLOAT3 norm, DirectX::XMFLOAT2 coord) {
		pos = p;
		normal = norm;
		uv = coord;
	}
	PrimVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) {
		pos.x = x;
		pos.y = y;
		pos.z = z;
		normal.x = nx;
		normal.y = ny;
		normal.z = nz;
		uv.x = u;
		uv.y = v;
	}
};

class PlaneMesh
{
private:
	void PlaneVertexBuffer();
	void PlaneDescriptorHeap();

	DirectX::XMFLOAT3 pos;
	float width;
	float depth;

	ID3D12Device* dev;

	ID3D12Resource* vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView;

	void CreateRootParam();
	void CreateSampler();
	void CreateRootSignature();
	void CreatePiplineState();

	D3D12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
	D3D12_ROOT_PARAMETER rootParam[1] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	ID3D12RootSignature* rootSignature;
	ID3D12PipelineState* piplineState;
public:
	PlaneMesh(ID3D12Device* _dev, const DirectX::XMFLOAT3 _pos,float _width, float _depth);
	~PlaneMesh();

	void Update();
};

