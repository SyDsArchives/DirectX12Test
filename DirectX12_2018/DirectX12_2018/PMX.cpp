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
	DataProp dataArray = {};//size = 8

	//モデル名関連
	unsigned int namelength_jp;
	wchar_t modelname_jp[3];//wchar_t = namelength / 2 = 3, char = namelength = 6
	unsigned int namelength_en;

	//コメント関連
	unsigned int commentlength;
	wchar_t comment[25];//commentlength / 2
	unsigned int commentlength_en;

	//頂点情報関連
	unsigned int vertexNum;
	std::vector<PMXVertex> vertex;

	//データの先を見るための配列
	int checkData[50];

	//pmxモデル情報
	fread(&magic, sizeof(magic), 1, fp);
	fread(&version, sizeof(version), 1, fp);
	fread(&datasize, sizeof(datasize), 1, fp);
	fread(&dataArray, sizeof(DataProp), 1, fp);

	
	//文字列サイズ取得
	fread(&namelength_jp, sizeof(namelength_jp), 1, fp);
	{
		for (int i = 0; i < namelength_jp / 2; ++i)
		{
			//日本語モデル名取得
			fread(&modelname_jp[i], sizeof(modelname_jp[i]), 1, fp);
		}
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

	//頂点数取得
	fread(&vertexNum, sizeof(vertexNum), 1, fp);
	vertex.resize(vertexNum);

	{
		//頂点情報読み込み
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
