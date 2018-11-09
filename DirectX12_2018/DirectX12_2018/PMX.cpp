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
	DataProp dataArray = {};//size = 8

	//���f�����֘A
	unsigned int namelength_jp;
	wchar_t modelname_jp[3];//wchar_t = namelength / 2 = 3, char = namelength = 6
	unsigned int namelength_en;

	//�R�����g�֘A
	unsigned int commentlength;
	wchar_t comment[25];//commentlength / 2
	unsigned int commentlength_en;

	//���_���֘A
	unsigned int vertexNum;
	std::vector<PMXVertex> vertex;

	//�f�[�^�̐�����邽�߂̔z��
	int checkData[50];

	//pmx���f�����
	fread(&magic, sizeof(magic), 1, fp);
	fread(&version, sizeof(version), 1, fp);
	fread(&datasize, sizeof(datasize), 1, fp);
	fread(&dataArray, sizeof(DataProp), 1, fp);

	
	//������T�C�Y�擾
	fread(&namelength_jp, sizeof(namelength_jp), 1, fp);
	{
		for (int i = 0; i < namelength_jp / 2; ++i)
		{
			//���{�ꃂ�f�����擾
			fread(&modelname_jp[i], sizeof(modelname_jp[i]), 1, fp);
		}
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

	//���_���擾
	fread(&vertexNum, sizeof(vertexNum), 1, fp);
	vertex.resize(vertexNum);

	{
		//���_���ǂݍ���
		for (int i = 0; i < vertex.size(); ++i)
		{
			fread(&vertex[i], sizeof(PMXVertex), 1, fp);
		}
	}

	{
		for (int i = 0; i < 50; ++i)
		{
			fread(&checkData[i], sizeof(checkData[i]), 1, fp);
		}
	}

	
	fclose(fp);
}
