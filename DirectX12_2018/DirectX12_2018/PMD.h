#pragma once

#include <vector>
#include "MyVector_2Dor3D.h"

#pragma pack(1)
//PMD���f���̊�{�I�ȏ����i�[����\����
struct PMDBasicData
{
	char magic[3];//�t�@�C���f�[�^�t�H�[�}�b�g�i�[�ϐ�
	float version;
	char modelName[20];
	char comment[256];
};

//PMD�̒��_�����i�[����\����
struct PMDVertexProperty
{
	Vector3f pos;
	Vector3f nomalVector;
	Vector2f uv;
	unsigned short boneNum[2];
	unsigned char boneWeight;
	unsigned char edgeFlag;
};

//PMD�̃}�e���A�������i�[����\����
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
	//���_���z��
	std::vector<PMDVertexProperty> pmdVertices;
	//�C���f�b�N�X���z��
	std::vector<unsigned short> pmdIndices;
	//�}�e���A�����z��
	std::vector<PMDMaterialProperty> pmdMaterials;
};


class PMD
{
private:

	//PMD���f���̊�{�������ϐ�
	PMDBasicData  pmdBasicData = {};
	//���f���f�[�^�Ǘ��ϐ�
	PMDModelData pmdModelData = {};
	////���_���z��
	//std::vector<PMDVertexProperty> pmdVertices;
	////�C���f�b�N�X���z��
	//std::vector<unsigned short> pmdIndices;
	////�}�e���A�����z��
	//std::vector<PMDMaterialProperty> pmdMaterials;

public:
	PMD();
	~PMD();

	PMDModelData Load(const char* _fileAddress);
};

