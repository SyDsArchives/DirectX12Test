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
	unsigned int frameNo;
	Position3f Location;
	DirectX::XMFLOAT4 Rotation;
	unsigned char interpolation[64];
};

struct VMDSkin
{
	char skinName[15];
	unsigned int frameNo;
	float weight;
};

struct VMDCamera
{
	unsigned int frameNo;
	float length;
	Position3f location;
	Position3f Ratation;
	unsigned char interpolation[24];
	unsigned int viewingAngle;
	unsigned char Perspective;
};

struct VMDLight
{
	unsigned int frameNo;
	float RGB[3];
	Position3f location;
};

struct VMDSelfShadow {
	unsigned int frameNo;
	unsigned char mode;
	float distance;
};

struct KeyFrame
{
	KeyFrame(unsigned int _frameNo, DirectX::XMFLOAT4 _quaternion) 
		:frameNo(_frameNo), quaternion(_quaternion){};
	unsigned int frameNo;
	DirectX::XMFLOAT4 quaternion;
};

typedef std::vector<KeyFrame> KeyFrame_v;
typedef std::map<std::string, std::vector<KeyFrame>> AnimationMap_m;

class VMD
{
private:
	unsigned int motionNum;//VMDのモーションデータ数
	unsigned int skinNum;//表情データ数
	unsigned int cameraNum;//カメラデータ数
	unsigned int lightNum;//照明データ数
	unsigned int shadowNum;//セルフシャドウ数
	unsigned int duration;//アニメーション1周の長さ

	VMDHeader vmdHeader = {};
	std::vector<VMDMotion> vmdMotion;
	std::vector<VMDSkin> vmdSkin;
	VMDCamera vmdCamera = {};
	VMDLight vmdLight = {};
	VMDSelfShadow vmdShadow = {};

	KeyFrame_v keyframes;
	AnimationMap_m animation;

public:
	VMD();
	~VMD();

	//VMDファイルからデータを読み込む
	void Load(const char* _fileAddress);

	//VMDのモーションを組み合わせて、アニメーションデータを作成する
	void InitAnimation();

	//map型のアニメーション情報を取得する
	const AnimationMap_m GetAnimationMapData() { return animation; };

	//VMDのアニメーション長を取得する
	const unsigned int GetDuration() { return duration; };
};

