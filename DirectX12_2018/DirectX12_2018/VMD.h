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
	unsigned int motionNum;//VMD�̃��[�V�����f�[�^��
	unsigned int skinNum;//�\��f�[�^��
	unsigned int cameraNum;//�J�����f�[�^��
	unsigned int lightNum;//�Ɩ��f�[�^��
	unsigned int shadowNum;//�Z���t�V���h�E��
	unsigned int duration;//�A�j���[�V����1���̒���

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

	//VMD�t�@�C������f�[�^��ǂݍ���
	void Load(const char* _fileAddress);

	//VMD�̃��[�V������g�ݍ��킹�āA�A�j���[�V�����f�[�^���쐬����
	void InitAnimation();

	//map�^�̃A�j���[�V���������擾����
	const AnimationMap_m GetAnimationMapData() { return animation; };

	//VMD�̃A�j���[�V���������擾����
	const unsigned int GetDuration() { return duration; };
};

