#pragma once

#include <vector>

struct DataProp
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

struct UV_EX {
	float x;
	float y;
	float z;
	float w;
};

struct PMXVertex
{
	float pos[3];
	float normal[3];
	float uv[2];
	std::vector<UV_EX> uv_ex;
	char wait;
	float edge;

};

class PMX
{
public:
	PMX();
	~PMX();

	void Load();
};

