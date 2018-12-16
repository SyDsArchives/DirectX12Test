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
#include <array>
#include <memory>

#include "PMX.h"

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

#pragma pack(1)
struct BoneProperty {
	char boneName[20];//�{�[����
	unsigned short parentBoneIndex;//�e�{�[���ԍ�
	unsigned short tailPosBoneIndex;//tail�ʒu�̃{�[���ԍ�
	unsigned char boneType;//�{�[���̎�� //0:��] 1:��]�ƈړ� 2:IK 3:�s�� 4:IK�e���� 5:��]�e���� 6:IK�ڑ��� 7:��\��
	unsigned short parentBoneIndex_IK;//IK�{�[���ԍ�
	Vector3f boneHeadPos;//xyz�̃{�[���̃w�b�h�̈ʒu
};
#pragma pack()

struct PMDBones
{
	unsigned short boneNum;//�{�[����
	std::vector<BoneProperty> boneProp;//�{�[���f�[�^
};

struct BoneNode {
	int boneIdx;
	Position3f startPos;
	Position3f endPos;
	std::vector<BoneNode*> children;
};

#pragma pack(1)
struct PMDIK {
	unsigned short ikIndex;
	unsigned short targetBoneIndex;
	unsigned char ikLength;
	unsigned short iterations;
	float control_weight;
	std::vector<unsigned short> ikChildrenIndex;//iklength;
};
#pragma pack()

struct PMDSkinVertData
{
	unsigned short skin_Vert_Index;
	float skin_Vert_Pos[3];
};

struct PMDSkinVertDataNonBase
{
	unsigned short skin_Vert_Index;
	float skin_Vert_Pos[3];
};

struct PMDSkinData
{
	char skinName[20];
	unsigned short skinCount;
	unsigned char skinType;
	std::vector<PMDSkinVertData> pmdSkinVertData;
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

//�}�e���A�������V�F�[�_�[�ɓn���p�̍\����
struct SendMaterialforShader
{
	SendMaterialforShader() {}
	SendMaterialforShader(MaterialColorRGBA& dif, MaterialColorRGBA& spe, MaterialColorRGBA& amb,float _spec):
		diffuse(dif) ,specular(spe), ambient(amb), spec(_spec){}
	MaterialColorRGBA diffuse;
	MaterialColorRGBA specular;
	MaterialColorRGBA ambient;
	float spec;
};

struct Cbuffer {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX viewproj;
	DirectX::XMMATRIX lvp;
};

class PlaneMesh;
struct KeyFrame;
typedef std::map<std::string, std::vector<KeyFrame>> AnimationMap_m;
struct DirectX::XMMATRIX;
class VMD;
class MyDirectX12
{
private:
	PMX* pmx;
	int testflag;
	unsigned int bbindex;
	unsigned int descriptorSizeRTV;

	HWND hwnd;
	IDXGIFactory6* dxgiFactory;

	//�t�@�N�g���[
	 //�֐�
	 void CreateDXGIFactory();

	//�f�o�C�X
	 //�֐�
	 void CreateDevice();
	 //�ϐ�
	 IDXGIAdapter4* adapter;
	 ID3D12Device* dev;

	//�R�}���h�n
	 //�֐�
	 void CreateCommandAllocator();
	 void CreateCommandQueue();
	 void CreateCommandList();
	 ID3D12CommandAllocator*	GetCommandAllocator();
	 ID3D12CommandQueue* GetCommandQueue();
	 ID3D12GraphicsCommandList3* GetCommandList();
	 //�ϐ�
	 ID3D12CommandAllocator* cmdAllocator;
	 ID3D12CommandQueue* cmdQueue;
	 ID3D12GraphicsCommandList3* cmdList;

	//�����_�[�^�[�Q�b�g�n
	 //�֐�
	 void CreateDescriptorHeapRTV();
	 void CreateRenderTarget();
	 //�ϐ�
	 ID3D12DescriptorHeap* descriptorHeapRTV;
	 std::vector<ID3D12Resource*> renderTarget;
	 D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;

	//�X���b�v�`�F�C��
	 //�֐�
	 void CreateSwapChain();
	 //�ϐ�
	 IDXGISwapChain4* swapChain;
	
	//���f�����_�o�b�t�@
	 //�֐�
	 void CreateVertexBuffer();
	 //�ϐ�
	 D3D12_VERTEX_BUFFER_VIEW vbView = {};
	 ID3D12Resource* vertexBuffer;

	//���f���C���f�b�N�X�o�b�t�@
	 //�֐�
	 void CreateIndexBuffer();
	 //�ϐ�
	 D3D12_INDEX_BUFFER_VIEW ibView = {};
	 ID3DBlob* vertexShader;
	 ID3DBlob* pixelShader;

	//�[�x�o�b�t�@
	 //�֐�
	 void CreateDepthBuffer();
	 //�ϐ�
	 ID3D12Resource* depthBuffer;
	 ID3D12DescriptorHeap* descriptorHeapDSB;

	//���[�g�V�O�l�`��
	 //�֐�
	 void CreateRootSignature();
	 //�ϐ�
	 ID3D12RootSignature* rootSignature;

	//�p�C�v���C���X�e�[�g
	 //�֐�
	 void CreatePiplineState();
	 //�ϐ�
	 ID3D12PipelineState* piplineState;

	//�r���[�|�[�g
	 //�֐�
	 void SetViewPort();
	 //�ϐ�
	 D3D12_VIEWPORT viewport;
	 D3D12_VIEWPORT viewport2;

	//�V�U�[���N�g
	 //�֐�
	 void SetScissorRect();
	 //�ϐ�
	 D3D12_RECT scissorRect;

	//�t�F���X
	 //�֐�
	 void CreateFence();
	 void ExecuteCommand(unsigned int cmdlistnum);
	 void WaitWithFence();
	 //�ϐ�
	 ID3D12Fence* fence;
	 UINT64 fenceValue;

	//�T���v���[
	 //�֐�
	 void CreateSamplerState();
	 //�ϐ�
	 D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	//���[�g�p�����[�^�[
	 //�֐�
	 void CreateRootParameter();
	 //�ϐ�
	 D3D12_ROOT_PARAMETER rootParam[6] = {};
	 D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	 D3D12_DESCRIPTOR_RANGE materialRange[2] = {};
	 D3D12_DESCRIPTOR_RANGE boneRange[1] = {};
	 D3D12_DESCRIPTOR_RANGE spRange[1] = {};
	 D3D12_DESCRIPTOR_RANGE toonRange[1] = {};
	 D3D12_DESCRIPTOR_RANGE shadowMapRange[1] = {};

	//�e�N�X�`���o�b�t�@
	 //�֐�
	 void CreateTextureBuffer();
	 //�ϐ�
	 ID3D12Resource* textureBuffer;

	//�萔�o�b�t�@ 
	 //�֐�
	 void CreateConstantBuffer();
	 //�ϐ�
	 ID3D12Resource* constantBuffer;
	 Cbuffer wvp = {};
	 Cbuffer* cbuff;

	//���W�X�^�f�X�N�q�[�v
	 //�֐�
	 void CreateDescriptorHeapRegister();
	 //�ϐ�
	 ID3D12DescriptorHeap* rgstDescHeap;//���̑�(�e�N�X�`���A�萔)�f�X�N���v�^�q�[�v
	
	//PMD�֘A
	 //�֐�
	 void LoadPMDModelData(const char * _modelFilename);
	 std::string GetToonPathFromIdx(int idx);
	 //�ϐ�
	 char magic[3];
	 PMDData pmddata = {};
	 std::vector<PMDVertex> pmdvertices;
	 std::vector<unsigned short> pmdindices;
	 std::vector<PMDMaterials> pmdmaterials;
	 unsigned int materialNum;
	 std::vector<PMDIK> pmdIK;
	 

	 //���f���g�D�[��
	 //�֐�
	 void CreateDescriptorHeapSRVforToon();
	 void CreateToonTextureBuffer();
	 ID3D12Resource * InitBuffer(UINT64 _width, UINT64 _height);
	 //�ϐ�
	 std::array<char[100], 10> toonTexNames;
	 ID3D12DescriptorHeap* toonDescriptorHeap;
	 ID3D12Resource* toonBuffer;

	//�}�e���A���o�b�t�@
	 //�֐�
	 void CreateMaterialBuffer();
	 void CreateDescriptorHeapforMaterial();
	 //�ϐ�
	 ID3D12DescriptorHeap* materialDescHeap;
	 std::vector<ID3D12Resource*> materialBuffer;
	 std::vector<SendMaterialforShader*> material;

	//���e�N�X�`��
	 //�֐�
	 void CreateWhiteTextureBuffer();
	 //�ϐ�
	 ID3D12Resource* whiteTextureBuffer;

	//�{�[���֘A
	 //�֐�
	 void CreateBoneBuffer();
	 void CreateDescriptorHeapforBone();
	 //�ϐ�
	 PMDBones pmdbones;
	 std::vector<DirectX::XMMATRIX> boneMatrices;
	 std::map<std::string, BoneNode> boneMap;
	 ID3D12DescriptorHeap* boneDescriptorHeap;
	 ID3D12Resource* boneBuffer;//�{�[���p�o�b�t�@
	 DirectX::XMMATRIX* bBuff;//�{�[�����X�V�p(buffer->map�p)

	//VMD
	 //�֐�
	 void RecursiveMatrixMultiply(BoneNode& node, DirectX::XMMATRIX& inMat);
	 void RotateBone(const char* bonename, const DirectX::XMFLOAT4& q, const DirectX::XMFLOAT4& q2 = DirectX::XMFLOAT4(), float t = 0.0f);
	 void MotionUpdate(int _frameNo);
	 
	 VMD* vmd;
	 AnimationMap_m animationData;
	 unsigned int lastTime;

	//�V���h�E�}�b�v
	 //�֐�
	 void CreateShadowmap();
	 void CreateShadowmapRootSignature();
	 void CreateShadowmapPiplineState();
	 void DrawLightView();
	 size_t RoundupPowerOf2(size_t size);

	 //�ϐ�
	 ID3D12DescriptorHeap* shadowSRVDescriptorHeap;//�V���h�E�}�b�vSRV�p�f�X�N�q�[�v
	 ID3D12DescriptorHeap* shadowDSVDescriptorHeap;//�V���h�E�}�b�vDSV�p�f�X�N�q�[�v
	 ID3D12Resource* shadowBuffer;
	 ID3D12RootSignature* shadowRootSignature;
	 ID3D12PipelineState* shadowPiplineState;
	 
////////////////////////////////
//			�}���`�p�X
////////////////////////////////

//�t�@�[�X�g�p�X
	 ID3D12Resource* firstpassBuffer;

	 //�����_�[�^�[�Q�b�g
	 //�֐�
	 void CreateDescriptorHeapRTVforFirstPass();
	 void CreateRenderTargetforFirstPass();
	 //�ϐ�
	 ID3D12DescriptorHeap* descriptorHeapRTV_FP;

	 //�V�F�[�_�[���\�[�X
	 //�֐�
	 void CreateDescriptorHeapSRVforFirstPass();
	 void CreateShaderResourceforFirstPass();
	 //�ϐ�
	 ID3D12DescriptorHeap* descriptorHeapSRV_FP;

	 //@param bool rtvflag �쐬�����o�b�t�@�[�������_�[�^�[�Q�b�g�Ƃ��Ďg�p���邩�ǂ���
	 //���\�[�X
	 void CreateMultiPassResource(ID3D12Resource * _resource, bool rtvflg);

//�Z�J���h�p�X
	 ID3D12Resource* secondpassBuffer;
	//�����_�[�^�[�Q�b�g
	//�֐�
	 void CreateDescriptorHeapRTVforSecondPass();
	 void CreateRenderTargetforSecondPass();
	 void CreateDescriptorHeapSRVforSecondPass();
	 void CreateShaderResourceforSecondPass();
	 void CreateDescriptorHeapforFirstpassTexture();
	 void CreateFirstpassTexture();
	 //�ϐ�
	 ID3D12DescriptorHeap* firstpassTextureDescHeap;
	 ID3D12DescriptorHeap* descriptorHeapRTV_SP;
	 ID3D12DescriptorHeap* descriptorHeapSRV_SP;

//�T�[�h�p�X
	  //���_�o�b�t�@
	  //�֐�
	  void CreateVertexBufferforPeraPolygon();
	  //�ϐ�
	  D3D12_VERTEX_BUFFER_VIEW vbView_pera = {};
	  ID3D12Resource* vertexBuffer_pera;

	  //�e�N�X�`��
	  //�֐�
	  void CreateDescriptorHeapforPeraTexture();
	  void CreatePeraPolygonTexture();
	  //�ϐ�
	  ID3D12DescriptorHeap* peraTextureDescriptorHeap;

	  //�T���v���[
	  //�֐�
	  void CreateSamplerStateforPeraPolygon();
	  //�ϐ�
	  D3D12_STATIC_SAMPLER_DESC samplerDesc_pera = {};

	  //���[�g�p�����[�^�[
	  //�֐�
	  void CreateRootParameterforPeraPolygon();
	  //�ϐ�
	  D3D12_ROOT_PARAMETER rootParam_pera[2] = {};
	  D3D12_DESCRIPTOR_RANGE descriptorRange_pera[2] = {};

	  //���[�g�V�O�l�`��
	  //�֐�
	  void CreateRootSignatureforPeraPolygon();
	  //�ϐ�
	  ID3D12RootSignature* rootSignature_pera;

	  //�p�C�v���C���X�e�[�g
	  //�֐�
	  void CreatePiplineStateforPeraPolygon();
	  //�ϐ�
	  ID3D12PipelineState* piplineState_pera;


	  /////////////���ʃI�u�W�F
	  std::shared_ptr<PlaneMesh> plane;

	  //�p�C�v���C���X�e�[�g
	  //�֐�
	  void CreatePiplineStateforPlane();
	  //�ϐ�
	  ID3D12PipelineState* piplineState_Plane;

public:
	MyDirectX12(HWND _hwnd);
	~MyDirectX12();

	void Update(float angle);

	void testUpdate();

	//�f�o�C�X
	ID3D12Device* GetDevice();

};

