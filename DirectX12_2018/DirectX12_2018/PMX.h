#pragma once

#include <vector>

struct DataProp
{
	char encodeType;//�G���R�[�h����  | 0:UTF16 1:UTF8
	char addUVNum;//�ǉ�UV�� 	| 0�`4
	char vertexIndexSize;//���_Index�T�C�Y | 1,2,4 �̂����ꂩ
	char textureIndexSize;//�e�N�X�`��Index�T�C�Y | 1,2,4 �̂����ꂩ
	char materialIndexSize;//�ގ�Index�T�C�Y | 1,2,4 �̂����ꂩ
	char boneIndexSize;//�{�[��Index�T�C�Y | 1,2,4 �̂����ꂩ
	char morphIndexSize;//���[�tIndex�T�C�Y | 1,2,4 �̂����ꂩ
	char rigidBodyIndexSize;//����Index�T�C�Y | 1,2,4 �̂����ꂩ
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

