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


	//テクスチャパスの読み込み
	unsigned int textureNum;//テクスチャ数
	std::vector<std::wstring> textureName;//テクスチャ名
	const wchar_t* filepath = L"resource/model/";//ファイルアドレス
	const wchar_t* slash = L"/";

	//テクスチャ数の読み込み
	fread(&textureNum, sizeof(textureNum), 1, fp);

	//テクスチャ数分メモリを確保する
	textureName.resize(textureNum);
	
	//テクスチャパスアドレス文字列の読み込み、組み合わせ
	{
		for (int i = 0; i < textureName.size(); ++i)
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
			textureName[i].append(filepath);
			textureName[i].append(modelname_jp);
			textureName[i].append(slash);
			textureName[i].append(dummy);
		}
	}


	//マテリアル情報の読み込み
	//マテリアル数格納用変数
	unsigned int materialNum;

	//日本語用マテリアル名格納変数
	std::vector<std::wstring> materialName_jp;
	//日本語用マテリアル名格納変数
	std::vector<std::wstring> materialName_en;
	//マテリアル情報格納用変数
	std::vector<PMXMaterial> materials;

	
	//マテリアル数の読み込み
	fread(&materialNum, sizeof(materialNum), 1, fp);

	//マテリアル数分メモリ確保
	materialName_jp.resize(materialNum);
	materialName_en.resize(materialNum);
	materials.resize(materialNum);

	//マテリアル名の読み込み
	{
		for (int i = 0; i < materialNum; ++i)
		{
			//日本語マテリアル名読み込み
			{
				//マテリアル名の長さ格納用
				unsigned int matNameSize = 0;
				//マテリアル名一時格納用
				wchar_t dummy[20] = {};

				//マテリアル名の長さの読み込み
				fread(&matNameSize, sizeof(matNameSize), 1, fp);

				//マテリアル名の読み込み
				fread(&dummy, matNameSize, 1, fp);

				//保存用変数にコピー
				materialName_jp[i] = dummy;
			}

			//英語マテリアル名読み込み
			{
				//マテリアル名の長さ格納用
				unsigned int matNameSize = 0;
				//マテリアル名一時格納用
				wchar_t dummy[20] = {};

				//マテリアル名の長さの読み込み
				fread(&matNameSize, sizeof(matNameSize), 1, fp);

				//マテリアル名の読み込み
				fread(&dummy, matNameSize, 1, fp);

				//保存用変数にコピー
				materialName_en[i] = dummy;
			}

			//マテリアル情報の読み込み
			{
				//diffuse読み込み
				fread(&materials[i].diffuse, sizeof(RGBA), 1, fp);
				//specular読み込み
				fread(&materials[i].specular, sizeof(RGB), 1, fp);
				//specular係数読み込み
				fread(&materials[i].specularCoefficient, sizeof(float), 1, fp);
				//ambient読み込み
				fread(&materials[i].ambient, sizeof(RGB), 1, fp);
				//bitFlag読み込み
				fread(&materials[i].drawFlagType, sizeof(char), 1, fp);
				//edge色読み込み
				fread(&materials[i].edgeColor, sizeof(RGBA), 1, fp);
				//edgeサイズ読み込み
				fread(&materials[i].edgeColor, sizeof(float), 1, fp);
				//textureIndexサイズ読み込み
				fread(&materials[i].textureIndexSize, pmxHeader.textureIndexSize, 1, fp);
				//sphereTextureIndexサイズ読み込み
				fread(&materials[i].sphereTextureIndexSize, pmxHeader.textureIndexSize, 1, fp);
				//sphereモード読み込み
				fread(&materials[i].sphereMode, sizeof(char), 1, fp);
				//共有toonフラグ読み込み
				fread(&materials[i].toonFlag, sizeof(bool), 1, fp);
				
				if (materials[i].toonFlag)
				{
					//toonTextureIndexサイズ読み込み
					fread(&materials[i].toonTextureIndexSize, pmxHeader.textureIndexSize, 1, fp);
				}
				else
				{
					//共有toonTexture読み込み
					fread(&materials[i].toonTextureIndexSize, sizeof(char), 1, fp);
				}

				//テキストメモ読み込み
				unsigned int textSize = 0;
				fread(&materials[i].text, textSize, 1, fp);
				//材質面数読み込み
				fread(&materials[i].materialIndexNum, sizeof(int), 1, fp);
			}
		}
	}
	fclose(fp);
}
