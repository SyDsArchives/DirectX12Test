#include "PMD.h"
#include <Windows.h>
#include <stdio.h>



PMD::PMD()
{
}


PMD::~PMD()
{
}

PMDModelData PMD::Load(const char* _fileAddress)
{
	PMDModelData ret;

	FILE* pmd_model = fopen(_fileAddress, "rb");

	fread(&pmdBasicData, sizeof(PMDBasicData), 1, pmd_model);

	//頂点読み込み
	//頂点数の読み込み
	unsigned int vertexNum = 0;
	fread(&vertexNum, sizeof(unsigned int), 1, pmd_model);

	pmdModelData.pmdVertices.resize(vertexNum);

	{
		for (int loadCnt = 0; loadCnt < pmdModelData.pmdVertices.size(); ++loadCnt)
		{
			fread(&pmdModelData.pmdVertices[loadCnt], sizeof(PMDVertexProperty), 1, pmd_model);
		}
	}

	//インデックス読み込み
	//インデックス数の読み込み
	unsigned int indexNum = 0;
	fread(&indexNum, sizeof(unsigned int), 1, pmd_model);

	pmdModelData.pmdIndices.resize(indexNum);

	//各頂点毎のインデックス情報を読み込む
	{
		for (int loadCnt = 0; loadCnt < indexNum; ++loadCnt)
		{
			fread(&pmdModelData.pmdIndices[loadCnt], sizeof(unsigned short), 1, pmd_model);
		}
	}

	unsigned int materialNum = 0;
	fread(&materialNum, sizeof(unsigned int), 1, pmd_model);

	pmdModelData.pmdMaterials.resize(materialNum);

	{
		for (int loadCnt = 0; loadCnt < materialNum; ++loadCnt)
		{
			fread(&pmdModelData.pmdMaterials[loadCnt], sizeof(PMDMaterialProperty), 1, pmd_model);
		}
	}

	//ファイルを閉じる
	fclose(pmd_model);

	ret = pmdModelData;

	return ret;
}
