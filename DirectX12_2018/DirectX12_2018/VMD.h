#pragma once
#include "MyVector_2Dor3D.h"
#include <vector>
#include <map>
#include <string>
#include <DirectXMath.h>

struct VMDHeader {
	char vmdHeader[30];
	char vmdModelName[20];
};

struct VMDMotion {//111byte
	char boneName[15];
	unsigned long frameNo;
	Position3f Location;
	DirectX::XMFLOAT4 Rotation;
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

struct KeyFrame
{
	int frameNo;
	DirectX::XMFLOAT4 quaternion;
};

class VMD
{
private:
	unsigned long motionNum;//VMDのモーションデータ数
	unsigned long skinNum;//表情データ数
	unsigned long cameraNum;//カメラデータ数
	unsigned long lightNum;//照明データ数
	unsigned long shadowNum;//セルフシャドウ数

	VMDHeader vmdHeader = {};
	std::vector<VMDMotion> vmdMotion;
	std::vector<VMDSkin> vmdSkin;
	VMDCamera vmdCamera = {};
	VMDLight vmdLight = {};
	VMDSelfShadow vmdShadow = {};

	std::vector<KeyFrame> keyframes;
	std::map<std::string, std::vector<KeyFrame>> animation;

public:
	VMD();
	~VMD();

	void Load(const char* _fileAddress);
	void InitAnimation();
};

