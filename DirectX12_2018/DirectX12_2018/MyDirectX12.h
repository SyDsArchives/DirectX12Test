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
#include <map>

#include "PMX.h"

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

#pragma pack(1)
struct BoneProperty {
	char boneName[20];//ボーン名
	unsigned short parentBoneIndex;//親ボーン番号
	unsigned short tailPosBoneIndex;//tail位置のボーン番号
	unsigned char boneType;//ボーンの種類 //0:回転 1:回転と移動 2:IK 3:不明 4:IK影響下 5:回転影響下 6:IK接続先 7:非表示
	unsigned short parentBoneIndex_IK;//IKボーン番号
	Vector3f boneHeadPos;//xyzのボーンのヘッドの位置
};
#pragma pack()

struct PMDBones
{
	unsigned short boneNum;//ボーン数
	std::vector<BoneProperty> boneProp;//ボーンデータ
};

struct BoneNode {
	int boneIdx;
	Position3f startPos;
	Position3f endPos;
	std::vector<BoneNode*> children;
};

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

//マテリアル情報をシェーダーに渡す用の構造体
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

struct KeyFrame;
typedef std::map<std::string, std::vector<KeyFrame>> AnimationMap_m;
struct DirectX::XMMATRIX;
class VMD;
class MyDirectX12
{
private:
	PMX* pmx;

	unsigned int bbindex;
	unsigned int descriptorSizeRTV;

	HWND hwnd;
	IDXGIFactory6* dxgiFactory;

	//ファクトリー
	 //関数
	 void CreateDXGIFactory();

	//デバイス
	 //関数
	 void CreateDevice();
	 //変数
	 IDXGIAdapter4* adapter;
	 ID3D12Device* dev;

	//コマンド系
	 //関数
	 void CreateCommandAllocator();
	 void CreateCommandQueue();
	 void CreateCommandList();
	 ID3D12CommandAllocator*	GetCommandAllocator();
	 ID3D12CommandQueue* GetCommandQueue();
	 ID3D12GraphicsCommandList3* GetCommandList();
	 //変数
	 ID3D12CommandAllocator* cmdAllocator;
	 ID3D12CommandQueue* cmdQueue;
	 ID3D12GraphicsCommandList3* cmdList;

	//レンダーターゲット系
	 //関数
	 void CreateDescriptorHeapRTV();
	 void CreateRenderTarget();
	 //変数
	 ID3D12DescriptorHeap* descriptorHeapRTV;
	 std::vector<ID3D12Resource*> renderTarget;
	 D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;

	//スワップチェイン
	 //関数
	 void CreateSwapChain();
	 //変数
	 IDXGISwapChain4* swapChain;
	
	//モデル頂点バッファ
	 //関数
	 void CreateVertexBuffer();
	 //変数
	 D3D12_VERTEX_BUFFER_VIEW vbView = {};
	 ID3D12Resource* vertexBuffer;

	//モデルインデックスバッファ
	 //関数
	 void CreateIndexBuffer();
	 //変数
	 D3D12_INDEX_BUFFER_VIEW ibView = {};
	 ID3DBlob* vertexShader;
	 ID3DBlob* pixelShader;

	//深度バッファ
	 //関数
	 void CreateDepthBuffer();
	 //変数
	 ID3D12Resource* depthBuffer;
	 ID3D12DescriptorHeap* descriptorHeapDSB;

	//ルートシグネチャ
	 //関数
	 void CreateRootSignature();
	 //変数
	 ID3D12RootSignature* rootSignature;

	//パイプラインステート
	 //関数
	 void CreatePiplineState();
	 //変数
	 ID3D12PipelineState* piplineState;

	//ビューポート
	 //関数
	 void SetViewPort();
	 //変数
	 D3D12_VIEWPORT viewport;

	//シザーレクト
	 //関数
	 void SetScissorRect();
	 //変数
	 D3D12_RECT scissorRect;

	//フェンス
	 //関数
	 void CreateFence();
	 void ExecuteCommand(unsigned int cmdlistnum);
	 void WaitWithFence();
	 //変数
	 ID3D12Fence* fence;
	 UINT64 fenceValue;

	//サンプラー
	 //関数
	 void CreateSamplerState();
	 //変数
	 D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	//ルートパラメーター
	 //関数
	 void CreateRootParameter();
	 //変数
	 D3D12_ROOT_PARAMETER rootParam[3] = {};
	 D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	 D3D12_DESCRIPTOR_RANGE materialRange[2] = {};
	 D3D12_DESCRIPTOR_RANGE boneRange[1] = {};

	//テクスチャバッファ
	 //関数
	 void CreateTextureBuffer();
	 //変数
	 ID3D12Resource* textureBuffer;

	//定数バッファ 
	 //関数
	 void CreateConstantBuffer();
	 //変数
	 ID3D12Resource* constantBuffer;
	 Cbuffer wvp = {};
	 Cbuffer* cbuff;

	//レジスタデスクヒープ
	 //関数
	 void CreateDescriptorHeapRegister();
	 //変数
	 ID3D12DescriptorHeap* rgstDescHeap;//その他(テクスチャ、定数)デスクリプタヒープ
	
	//PMD関連
	 //関数
	 void LoadPMDModelData(const char * _modelFilename);
	 //変数
	 char magic[3];
	 PMDData pmddata = {};
	 std::vector<PMDVertex> pmdvertices;
	 std::vector<unsigned short> pmdindices;
	 std::vector<PMDMaterials> pmdmaterials;
	 unsigned int materialNum;

	//マテリアルバッファ
	 //関数
	 void CreateMaterialBuffer();
	 void CreateDescriptorHeapforMaterial();
	 //変数
	 ID3D12DescriptorHeap* materialDescHeap;
	 std::vector<ID3D12Resource*> materialBuffer;
	 std::vector<SendMaterialforShader*> material;

	//白テクスチャ
	 //関数
	 void CreateWhiteTextureBuffer();
	 //変数
	 ID3D12Resource* whiteTextureBuffer;

	//ボーン関連
	 //関数
	 void CreateBoneBuffer();
	 //変数
	 PMDBones pmdbones;
	 std::vector<DirectX::XMMATRIX> boneMatrices;
	 std::map<std::string, BoneNode> boneMap;
	 ID3D12Resource* boneBuffer;//ボーン用バッファ
	 DirectX::XMMATRIX* bBuff;//ボーン情報更新用(buffer->map用)

	//VMD
	 //関数
	 void RecursiveMatrixMultiply(BoneNode& node, DirectX::XMMATRIX& inMat);
	 void RotateBone(const char* bonename, const DirectX::XMFLOAT4& q, const DirectX::XMFLOAT4& q2 = DirectX::XMFLOAT4(), float t = 0.0f);
	 void MotionUpdate(int _frameNo);
	 //変数
	 VMD* vmd;
	 AnimationMap_m animationData;
	 unsigned int lastTime;


////////////////////////////////
//			マルチパス
////////////////////////////////

	 ID3D12Resource* firstpassBuffer;
	 //std::vector<ID3D12Resource*> firstpassBuffer;
	 //レンダーターゲット
	  //関数
	  void CreateDescriptorHeapRTVforFirstPass();
	  void CreateRenderTargetforFirstPass();
	  //変数
	  ID3D12DescriptorHeap* descriptorHeapRTV_FP;
	  ID3D12Resource* multipassRTV;

	 //シェーダーリソース
	  //関数
	  void CreateDescriptorHeapSRVforFirstPass();
	  void CreateShaderResourceforFirstPass();
	  //変数
	  ID3D12DescriptorHeap* descriptorHeapSRV_FP;
	  ID3D12Resource* multipassSRV;

	  //リソース
	  void CreateFirstPassResource(ID3D12Resource * _resource, bool rtvflg);

	  //頂点バッファ
	  //関数
	  void CreateVertexBufferforFirstPass();
	  //変数
	  D3D12_VERTEX_BUFFER_VIEW vbViewFP = {};
	  ID3D12Resource* vertexBufferFP;

	  //サンプラー
	  //関数
	  void CreateSamplerStateforFirstPass();
	  //変数
	  D3D12_STATIC_SAMPLER_DESC samplerDescFP = {};

	  //ルートパラメーター
	  //関数
	  void CreateRootParameterforFirstPass();
	  //変数
	  D3D12_ROOT_PARAMETER rootParamFP[1] = {};
	  D3D12_DESCRIPTOR_RANGE descriptorRangeFP[1] = {};

	  //デスクリプターテーブル
	  //関数
	  void CreateDescriptorHeapforFirstPass();
	  //変数
	  ID3D12DescriptorHeap* firstPassDescHeap;//その他(テクスチャ、定数)デスクリプタヒープ


	  //ルートシグネチャ
	  //関数
	  void CreateRootSignatureforFirstPass();
	  //変数
	  ID3D12RootSignature* rootSignatureFP;

	  //パイプラインステート
	  //関数
	  void CreatePiplineStateforFirstPass();
	  //変数
	  ID3D12PipelineState* piplineStateFP;

	  //シャドウデプスバッファ
	  //関数
	  void CreateDepthBufferforShadow();
	  //変数
	  ID3D12Resource* depthBufferFP;
	  ID3D12DescriptorHeap* descriptorHeapDSBFP;


public:
	MyDirectX12(HWND _hwnd);
	~MyDirectX12();
	
	void OutLoopDx12();
	void Update(float angle);

	void testUpdate();

	//デバイス
	ID3D12Device* GetDevice();

};

