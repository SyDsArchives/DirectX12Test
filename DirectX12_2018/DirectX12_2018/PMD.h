#pragma once

#include <vector>
#include "MyVector_2Dor3D.h"

#pragma pack(1)
//PMDモデルの基本的な情報を格納する構造体
struct PMDBasicData
{
	char magic[3];//ファイルデータフォーマット格納変数
	float version;
	char modelName[20];
	char comment[256];
};

//PMDの頂点情報を格納する構造体
struct PMDVertexProperty
{
	Vector3f pos;
	Vector3f nomalVector;
	Vector2f uv;
	unsigned short boneNum[2];
	unsigned char boneWeight;
	unsigned char edgeFlag;
};

//PMDのマテリアル情報を格納する構造体
struct PMDMaterialProperty
{
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

struct PMDModelData
{
	//頂点情報配列
	std::vector<PMDVertexProperty> pmdVertices;
	//インデックス情報配列
	std::vector<unsigned short> pmdIndices;
	//マテリアル情報配列
	std::vector<PMDMaterialProperty> pmdMaterials;
};


class PMD
{
private:

	//PMDモデルの基本情報を持つ変数
	PMDBasicData  pmdBasicData = {};
	//モデルデータ管理変数
	PMDModelData pmdModelData = {};
	////頂点情報配列
	//std::vector<PMDVertexProperty> pmdVertices;
	////インデックス情報配列
	//std::vector<unsigned short> pmdIndices;
	////マテリアル情報配列
	//std::vector<PMDMaterialProperty> pmdMaterials;

public:
	PMD();
	~PMD();

	PMDModelData Load(const char* _fileAddress);
};

