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
	const char* fileAdd = "resource/model/立花響/立花響.pmx";
	FILE* fp = fopen(fileAdd,"rb");

	//pmxモデル情報
	char magic[4];
	float version;
	char datasize;
	PMXHeader pmxHeader = {};//size = 8

	//モデル名関連
	unsigned int namelength_jp;
	wchar_t dummyName[15] = {};
	std::wstring modelname_jp;//wchar_t = namelength / 2 = 3, char = namelength = 6
	unsigned int namelength_en;

	//コメント関連
	unsigned int commentlength;
	wchar_t comment[25];//commentlength / 2
	unsigned int commentlength_en;



	//pmxモデル情報
	fread(&magic, sizeof(magic), 1, fp);
	fread(&version, sizeof(version), 1, fp);
	fread(&datasize, sizeof(datasize), 1, fp);
	fread(&pmxHeader, sizeof(PMXHeader), 1, fp);

	
	//文字列サイズ取得
	fread(&namelength_jp, sizeof(namelength_jp), 1, fp);
	{
		for (int i = 0; i < namelength_jp / 2; ++i)
		{
			//日本語モデル名取得
			fread(&dummyName[i], 2, 1, fp);
		}
		modelname_jp = dummyName;
	}

	//文字列サイズ取得(英語)(響は英語名が入ってないのでサイズが0で読み込むモデル名配列もない)
	fread(&namelength_en, sizeof(namelength_en), 1, fp);
	

	//コメントの文字列サイズ取得
	fread(&commentlength, sizeof(commentlength), 1, fp);
	{
		for (int i = 0; i < commentlength / 2; ++i)
		{
			//コメントの文字列取得
			fread(&comment[i], sizeof(comment[i]), 1, fp);
		}
	}

	//英語コメント文字列取得(こちらも名前と同じく無い)
	fread(&commentlength_en, sizeof(commentlength_en), 1, fp);

	//頂点情報関連
	unsigned int vertexNum;
	std::vector<PMXVertex> vertex;

	//頂点数取得
	fread(&vertexNum, sizeof(vertexNum), 1, fp);
	vertex.resize(vertexNum);

	{
		//頂点情報読み込み
		for (int i = 0; i < vertex.size(); ++i)
		{
			//位置
			fread(&vertex[i].pos, sizeof(vertex[i].pos), 1, fp);
			//法線
			fread(&vertex[i].normal, sizeof(vertex[i].normal), 1, fp);
			//UV
			fread(&vertex[i].uv, sizeof(vertex[i].uv), 1, fp);
			//追加UV
			if (pmxHeader.addUVNum != 0)
			{
				vertex[i].addUV.resize(pmxHeader.addUVNum);
				for (int num = 0; num < pmxHeader.addUVNum; ++num)
				{
					fread(&vertex[i].addUV[num], sizeof(AdditionalUV), 1, fp);
				}
			}
			//ウェイト方式
			fread(&vertex[i].weightType, sizeof(vertex[i].weightType), 1, fp);
			//ウェイト情報
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
			//エッジ倍率
			fread(&vertex[i].edgeScale, sizeof(vertex[i].edgeScale), 1, fp);
		}
	}

	//面情報読み込み
	unsigned int indexNum;

	//インデックス数の取得
	fread(&indexNum, sizeof(indexNum), 1, fp);
	indexNum = indexNum / 3;//最大インデックス数 / 3頂点

	//インデックス情報の取得
	std::vector<PMXIndex> pmxIndex(indexNum);
	{
		for (int i = 0; i < pmxIndex.size(); ++i)
		{
			fread(&pmxIndex[i].index1, pmxHeader.vertexIndexSize, 1, fp);
			fread(&pmxIndex[i].index2, pmxHeader.vertexIndexSize, 1, fp);
			fread(&pmxIndex[i].index3, pmxHeader.vertexIndexSize, 1, fp);
		}
	}


	//テクスチャパスアドレスの読み込み
	unsigned int materialNum;//マテリアル数
	std::vector<std::wstring> materialName;//マテリアル名
	const wchar_t* filepath = L"resource/model/";//ファイルアドレス
	const wchar_t* slash = L"/";

	//テクスチャ数の読み込み
	fread(&materialNum, sizeof(materialNum), 1, fp);

	//テクスチャ数分メモリを確保する
	materialName.resize(materialNum);
	
	//テクスチャパスアドレス文字列の読み込み、組み合わせ
	for (int i = 0; i < materialName.size(); ++i)
	{
		//テクスチャ名文字列数
		unsigned int texNameSize = 0;
		//テクスチャ名用ダミー変数
		wchar_t dummy[20] = {};

		//テクスチャ名文字列数読み込み
		fread(&texNameSize, sizeof(texNameSize), 1, fp);
		//テクスチャ名の読み込み
		fread(&dummy, texNameSize, 1, fp);

		//テクスチャアドレスの組み合わせ
		materialName[i].append(filepath);
		materialName[i].append(modelname_jp);
		materialName[i].append(slash);
		materialName[i].append(dummy);
	}
	

	fclose(fp);
}
