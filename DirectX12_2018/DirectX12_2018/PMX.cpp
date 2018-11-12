#include "PMX.h"
#include <stdio.h>




PMX::PMX()
{
}


PMX::~PMX()
{
}

void PMX::Load()
{
	const char* fileAdd = "resource/model/���ԋ�/���ԋ�.pmx";
	FILE* fp = fopen(fileAdd,"rb");

	//pmx���f�����
	char magic[4];
	float version;
	char datasize;
	PMXHeader pmxHeader = {};//size = 8

	//���f�����֘A
	unsigned int namelength_jp;
	wchar_t dummyName[15] = {};
	std::wstring modelname_jp;//wchar_t = namelength / 2 = 3, char = namelength = 6
	unsigned int namelength_en;

	//�R�����g�֘A
	unsigned int commentlength;
	wchar_t comment[25];//commentlength / 2
	unsigned int commentlength_en;



	//pmx���f�����
	fread(&magic, sizeof(magic), 1, fp);
	fread(&version, sizeof(version), 1, fp);
	fread(&datasize, sizeof(datasize), 1, fp);
	fread(&pmxHeader, sizeof(PMXHeader), 1, fp);

	
	//������T�C�Y�擾
	fread(&namelength_jp, sizeof(namelength_jp), 1, fp);
	{
		for (int i = 0; i < namelength_jp / 2; ++i)
		{
			//���{�ꃂ�f�����擾
			fread(&dummyName[i], 2, 1, fp);
		}
		modelname_jp = dummyName;
	}

	//������T�C�Y�擾(�p��)(���͉p�ꖼ�������ĂȂ��̂ŃT�C�Y��0�œǂݍ��ރ��f�����z����Ȃ�)
	fread(&namelength_en, sizeof(namelength_en), 1, fp);
	

	//�R�����g�̕�����T�C�Y�擾
	fread(&commentlength, sizeof(commentlength), 1, fp);
	{
		for (int i = 0; i < commentlength / 2; ++i)
		{
			//�R�����g�̕�����擾
			fread(&comment[i], sizeof(comment[i]), 1, fp);
		}
	}

	//�p��R�����g������擾(����������O�Ɠ���������)
	fread(&commentlength_en, sizeof(commentlength_en), 1, fp);

	//���_���֘A
	unsigned int vertexNum;
	std::vector<PMXVertex> vertex;

	//���_���擾
	fread(&vertexNum, sizeof(vertexNum), 1, fp);
	vertex.resize(vertexNum);

	{
		//���_���ǂݍ���
		for (int i = 0; i < vertex.size(); ++i)
		{
			//�ʒu
			fread(&vertex[i].pos, sizeof(vertex[i].pos), 1, fp);
			//�@��
			fread(&vertex[i].normal, sizeof(vertex[i].normal), 1, fp);
			//UV
			fread(&vertex[i].uv, sizeof(vertex[i].uv), 1, fp);
			//�ǉ�UV
			if (pmxHeader.addUVNum != 0)
			{
				vertex[i].addUV.resize(pmxHeader.addUVNum);
				for (int num = 0; num < pmxHeader.addUVNum; ++num)
				{
					fread(&vertex[i].addUV[num], sizeof(AdditionalUV), 1, fp);
				}
			}
			//�E�F�C�g����
			fread(&vertex[i].weightType, sizeof(vertex[i].weightType), 1, fp);
			//�E�F�C�g���
			if (vertex[i].weightType == 0)
			{
				//BDEF1
				vertex[i].weightprop = {};

				fread(&vertex[i].weightprop.index1, pmxHeader.vertexIndexSize, 1, fp);
			}
			else if (vertex[i].weightType == 1)
			{
				//BDEF2
				vertex[i].weightprop = {};

				fread(&vertex[i].weightprop.index1, pmxHeader.vertexIndexSize, 1, fp);
				fread(&vertex[i].weightprop.index2, pmxHeader.vertexIndexSize, 1, fp);

				fread(&vertex[i].weightprop.weight1, sizeof(float), 1, fp);
				int a = 0;

			}
			else if (vertex[i].weightType == 2)
			{
				//BDEF4
				vertex[i].weightprop = {};

				fread(&vertex[i].weightprop.index1, pmxHeader.vertexIndexSize, 1, fp);
				fread(&vertex[i].weightprop.index2, pmxHeader.vertexIndexSize, 1, fp);
				fread(&vertex[i].weightprop.index3, pmxHeader.vertexIndexSize, 1, fp);
				fread(&vertex[i].weightprop.index4, pmxHeader.vertexIndexSize, 1, fp);

				fread(&vertex[i].weightprop.weight1, sizeof(vertex[i].weightprop.weight1), 1, fp);
				fread(&vertex[i].weightprop.weight2, sizeof(vertex[i].weightprop.weight2), 1, fp);
				fread(&vertex[i].weightprop.weight3, sizeof(vertex[i].weightprop.weight3), 1, fp);
				fread(&vertex[i].weightprop.weight4, sizeof(vertex[i].weightprop.weight4), 1, fp);

			}
			else if (vertex[i].weightType == 3)
			{
				//SDEF
				vertex[i].weightprop = {};
				vertex[i].sdef = {};

				fread(&vertex[i].weightprop.index1, pmxHeader.vertexIndexSize, 1, fp);
				fread(&vertex[i].weightprop.index2, pmxHeader.vertexIndexSize, 1, fp);

				fread(&vertex[i].weightprop.weight1, sizeof(vertex[i].weightprop.weight1), 1, fp);

				fread(&vertex[i].sdef.C, sizeof(vertex[i].sdef.C), 1, fp);
				fread(&vertex[i].sdef.R0, sizeof(vertex[i].sdef.R0), 1, fp);
				fread(&vertex[i].sdef.R1, sizeof(vertex[i].sdef.R1), 1, fp);
			}
			//�G�b�W�{��
			fread(&vertex[i].edgeScale, sizeof(vertex[i].edgeScale), 1, fp);
		}
	}

	//�ʏ��ǂݍ���
	unsigned int indexNum;

	//�C���f�b�N�X���̎擾
	fread(&indexNum, sizeof(indexNum), 1, fp);
	indexNum = indexNum / 3;//�ő�C���f�b�N�X�� / 3���_

	//�C���f�b�N�X���̎擾
	std::vector<PMXIndex> pmxIndex(indexNum);
	{
		for (int i = 0; i < pmxIndex.size(); ++i)
		{
			fread(&pmxIndex[i].index1, pmxHeader.vertexIndexSize, 1, fp);
			fread(&pmxIndex[i].index2, pmxHeader.vertexIndexSize, 1, fp);
			fread(&pmxIndex[i].index3, pmxHeader.vertexIndexSize, 1, fp);
		}
	}


	//�e�N�X�`���p�X�A�h���X�̓ǂݍ���
	unsigned int materialNum;//�}�e���A����
	std::vector<std::wstring> materialName;//�}�e���A����
	const wchar_t* filepath = L"resource/model/";//�t�@�C���A�h���X
	const wchar_t* slash = L"/";

	//�e�N�X�`�����̓ǂݍ���
	fread(&materialNum, sizeof(materialNum), 1, fp);

	//�e�N�X�`���������������m�ۂ���
	materialName.resize(materialNum);
	
	//�e�N�X�`���p�X�A�h���X������̓ǂݍ��݁A�g�ݍ��킹
	for (int i = 0; i < materialName.size(); ++i)
	{
		//�e�N�X�`����������
		unsigned int texNameSize = 0;
		//�e�N�X�`�����p�_�~�[�ϐ�
		wchar_t dummy[20] = {};

		//�e�N�X�`���������񐔓ǂݍ���
		fread(&texNameSize, sizeof(texNameSize), 1, fp);
		//�e�N�X�`�����̓ǂݍ���
		fread(&dummy, texNameSize, 1, fp);

		//�e�N�X�`���A�h���X�̑g�ݍ��킹
		materialName[i].append(filepath);
		materialName[i].append(modelname_jp);
		materialName[i].append(slash);
		materialName[i].append(dummy);
	}
	

	fclose(fp);
}
