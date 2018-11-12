#pragma once

#include <vector>
#include <map>

enum class WeightPattern
{
	BDEF1,//: int | 4 | �{�[���̂�
	BDEF2,// : int, int, float | 4 * 3 | �{�[��2�ƁA�{�[��1�̃E�F�C�g�l(PMD����)
	BDEF4,// : int*4, float*4 | 4 * 8 | �{�[��4�ƁA���ꂼ��̃E�F�C�g�l�B�E�F�C�g���v��1.0�ł���ۏ�͂��Ȃ�
	SDEF,// : int, int, float, float3 * 3 | 4 * 12 | BDEF2�ɉ����ASDEF�p��float3(Vector3)��3�B���ۂ̌v�Z�ł͂���ɕ␳�l�̎Z�o���K�v(�ꉞ���̂܂�BDEF2�Ƃ��Ă��g�p�\)
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
	char encodeType;//�G���R�[�h����  | 0:UTF16 1:UTF8
	char addUVNum;//�ǉ�UV�� 	| 0�`4
	char vertexIndexSize;//���_Index�T�C�Y | 1,2,4 �̂����ꂩ
	char textureIndexSize;//�e�N�X�`��Index�T�C�Y | 1,2,4 �̂����ꂩ
	char materialIndexSize;//�ގ�Index�T�C�Y | 1,2,4 �̂����ꂩ
	char boneIndexSize;//�{�[��Index�T�C�Y | 1,2,4 �̂����ꂩ
	char morphIndexSize;//���[�tIndex�T�C�Y | 1,2,4 �̂����ꂩ
	char rigidBodyIndexSize;//����Index�T�C�Y | 1,2,4 �̂����ꂩ
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
	FLOAT3 pos;//�ʒu���
	FLOAT3 normal;//�@�����
	UV uv;//UV���
	std::vector<AdditionalUV> addUV;//�ǉ�UV���
	uint8_t weightType;//�E�F�C�g����
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

