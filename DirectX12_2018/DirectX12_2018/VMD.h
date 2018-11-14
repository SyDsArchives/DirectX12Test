#pragma once
#include "MyVector_2Dor3D.h"
#include <vector>

struct VMDHeader {
	char vmdHeader[30];
	char vmdModelName[20];
};

struct VMDMotion {//111byte
	char boneName[15];
	unsigned long frameNo;
	Position3f Location;
	float Rotation[4];
	unsigned char interpolation[64];
};

struct VMDSkin
{
	char skinName[15];
	unsigned long frameNo;
	float weight;
};

struct VMDCamera
{
	unsigned long frameNo;
	float length;
	Position3f location;
	Position3f Ratation;
	unsigned char interpolation[24];
	unsigned long viewingAngle;
	unsigned char Perspective;
};

struct VMDLight
{
	unsigned long frameNo;
	float RGB[3];
	Position3f location;
};

struct VMDSelfShadow {
	unsigned long frameNo;
	unsigned char mode;
	float distance;
};

class VMD
{
private:
	unsigned long motionNum;//VMD�̃��[�V�����f�[�^��
	unsigned long skinNum;//�\��f�[�^��
	unsigned long cameraNum;//�J�����f�[�^��
	unsigned long lightNum;//�Ɩ��f�[�^��
	unsigned long shadowNum;//�Z���t�V���h�E��

	VMDHeader vmdHeader = {};
	std::vector<VMDMotion> vmdMotion;
	std::vector<VMDSkin> vmdSkin;
	/*VMDCamera vmdCamera = {};
	VMDLight vmdLight = {};
	VMDSelfShadow vmdShadow = {};*/

public:
	VMD();
	~VMD();

	void Load(const char* _fileAddress);
};

