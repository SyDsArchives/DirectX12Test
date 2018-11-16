#include "VMD.h"
#include <stdio.h>
#include <algorithm>

VMD::VMD()
{
}


VMD::~VMD()
{
}

void VMD::Load(const char * _fileAddress)
{
	FILE* fp = fopen(_fileAddress, "rb");

	duration = 0;

	//VMDHeader�ǂݍ���
	fread(&vmdHeader.vmdHeader, sizeof(vmdHeader.vmdHeader), 1, fp);
	fread(&vmdHeader.vmdModelName, sizeof(vmdHeader.vmdModelName), 1, fp);

	{
		//���[�V�������ǂݍ���
		fread(&motionNum, sizeof(motionNum), 1, fp);
		if (motionNum != 0)
		{
			//�������̊m��
			vmdMotion.resize(motionNum);
			//���[�V�������̓ǂݍ���
			for (int i = 0; i < vmdMotion.size(); ++i)
			{
				fread(&vmdMotion[i].boneName, sizeof(vmdMotion[i].boneName), 1, fp);
				fread(&vmdMotion[i].frameNo, sizeof(vmdMotion[i].frameNo), 1, fp);
				fread(&vmdMotion[i].Location, sizeof(vmdMotion[i].Location), 1, fp);
				fread(&vmdMotion[i].Rotation, sizeof(vmdMotion[i].Rotation), 1, fp);
				fread(&vmdMotion[i].interpolation, sizeof(vmdMotion[i].interpolation), 1, fp);

				//�A�j���[�V����1�������擾
				duration = std::max(duration, vmdMotion[i].frameNo);
			}
		}
	}
	
	{
		//�\��f�[�^���̓ǂݍ���
		fread(&skinNum, sizeof(skinNum), 1, fp);

		if (skinNum != 0)
		{
			//�������̊m��
			vmdSkin.resize(skinNum);
			//�\��f�[�^�̓ǂݍ���
			for (int i = 0; i < vmdSkin.size(); ++i)
			{
				fread(&vmdSkin[i].skinName, sizeof(vmdSkin[i].skinName), 1, fp);
				fread(&vmdSkin[i].frameNo, sizeof(vmdSkin[i].frameNo), 1, fp);
				fread(&vmdSkin[i].weight, sizeof(vmdSkin[i].weight), 1, fp);
			}
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

	//{
	//	fread(&shadowNum, sizeof(shadowNum), 1, fp);
	//	if (shadowNum != 0)
	//	{
	//		int a = 0;
	//	}
	//}

	fclose(fp);
	InitAnimation();
}

void VMD::InitAnimation()
{
	for (auto& keyFrame : vmdMotion)
	{
		animation[keyFrame.boneName].emplace_back(keyFrame.frameNo, keyFrame.Rotation);
	}

	for (auto& data : animation)
	{
		std::sort(data.second.begin(), data.second.end(), [](KeyFrame& a, KeyFrame& b) {return a.frameNo < b.frameNo; });
	}
}

