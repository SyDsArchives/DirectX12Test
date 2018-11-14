#include "VMD.h"
#include <stdio.h>

VMD::VMD()
{
}


VMD::~VMD()
{
}

void VMD::Load(const char * _fileAddress)
{
	FILE* fp = fopen(_fileAddress, "rb");

	//VMDHeader読み込み
	fread(&vmdHeader.vmdHeader, sizeof(vmdHeader.vmdHeader), 1, fp);
	fread(&vmdHeader.vmdModelName, sizeof(vmdHeader.vmdModelName), 1, fp);

	{
		//モーション数読み込み
		fread(&motionNum, sizeof(motionNum), 1, fp);
		//メモリの確保
		vmdMotion.resize(motionNum);
		//モーション情報の読み込み
		for (int i = 0; i < vmdMotion.size(); ++i)
		{
			fread(&vmdMotion[i].boneName, sizeof(vmdMotion[i].boneName), 1, fp);
			fread(&vmdMotion[i].frameNo, sizeof(vmdMotion[i].frameNo), 1, fp);
			fread(&vmdMotion[i].Location, sizeof(vmdMotion[i].Location), 1, fp);
			fread(&vmdMotion[i].Rotation, sizeof(vmdMotion[i].Rotation), 1, fp);
			fread(&vmdMotion[i].interpolation, sizeof(vmdMotion[i].interpolation), 1, fp);
		}
	}
	
	{
		//表情データ数の読み込み
		fread(&skinNum, sizeof(skinNum), 1, fp);
		//メモリの確保
		vmdSkin.resize(skinNum);
		//表情データの読み込み
		for (int i = 0; i < vmdSkin.size(); ++i)
		{
			fread(&vmdSkin[i].skinName, sizeof(vmdSkin[i].skinName), 1, fp);
			fread(&vmdSkin[i].frameNo, sizeof(vmdSkin[i].frameNo), 1, fp);
			fread(&vmdSkin[i].weight, sizeof(vmdSkin[i].weight), 1, fp);
		}
	}

	{
		fread(&cameraNum, sizeof(cameraNum), 1, fp);
		if (cameraNum != 0)
		{
			int a = 0;
		}
	}

	{
		fread(&lightNum, sizeof(lightNum), 1, fp);
		if (lightNum != 0)
		{
			int a = 0;
		}
	}

	{
		fread(&shadowNum, sizeof(shadowNum), 1, fp);
		if (shadowNum != 0)
		{
			int a = 0;
		}
	}

	fclose(fp);
}

void VMD::InitAnimation()
{
	for (auto& keyFrame : vmdMotion)
	{
		//animation[keyFrame.boneName].emplace_back(KeyFrame((int)(keyFrame.frameNo), keyFrame.Rotation));
	}
}
