#pragma once

#include <vector>
#include <map>

enum class WeightPattern
{
	BDEF1,//: int | 4 | ボーンのみ
	BDEF2,// : int, int, float | 4 * 3 | ボーン2つと、ボーン1のウェイト値(PMD方式)
	BDEF4,// : int*4, float*4 | 4 * 8 | ボーン4つと、それぞれのウェイト値。ウェイト合計が1.0である保障はしない
	SDEF,// : int, int, float, float3 * 3 | 4 * 12 | BDEF2に加え、SDEF用のfloat3(Vector3)が3つ。実際の計算ではさらに補正値の算出が必要(一応そのままBDEF2としても使用可能)
};

struct FLOAT3
{
	float x;
	float y;
	float z;
};

struct UV
{
	float u;
	float v;
};


struct PMXHeader
{
	char encodeType;//エンコード方式  | 0:UTF16 1:UTF8
	char addUVNum;//追加UV数 	| 0～4
	char vertexIndexSize;//頂点Indexサイズ | 1,2,4 のいずれか
	char textureIndexSize;//テクスチャIndexサイズ | 1,2,4 のいずれか
	char materialIndexSize;//材質Indexサイズ | 1,2,4 のいずれか
	char boneIndexSize;//ボーンIndexサイズ | 1,2,4 のいずれか
	char morphIndexSize;//モーフIndexサイズ | 1,2,4 のいずれか
	char rigidBodyIndexSize;//剛体Indexサイズ | 1,2,4 のいずれか
};

struct AdditionalUV {
	float x;
	float y;
	float z;
	float w;
};

struct PMDVertex_Weight
{
	int index1;
	int index2;
	int index3;
	int index4;
	float weight1;
	float weight2;
	float weight3;
	float weight4;
};

struct SDEF_BoneInfo
{
	FLOAT3 C;
	FLOAT3 R0;
	FLOAT3 R1;
};

struct PMXVertex
{
	FLOAT3 pos;//位置情報
	FLOAT3 normal;//法線情報
	UV uv;//UV情報
	std::vector<AdditionalUV> addUV;//追加UV情報
	uint8_t weightType;//ウェイト方式
	PMDVertex_Weight weightprop;
	SDEF_BoneInfo sdef;
	float edgeScale;
};

struct PMXIndex
{
	unsigned int index1;
	unsigned int index2;
	unsigned int index3;
};

class PMX
{
public:
	PMX();
	~PMX();

	void Load();
};

