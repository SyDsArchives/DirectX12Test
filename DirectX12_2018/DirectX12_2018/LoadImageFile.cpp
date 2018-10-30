#include "LoadImageFile.h"
#include <Windows.h>
#include <stdio.h>
#include <vector>


LoadImageFile::LoadImageFile()
{
}


LoadImageFile::~LoadImageFile()
{
}

std::string LoadImageFile::SearchImageFile(const char* _filename)
{
	//�t�H���_��������̒�`
	const char* fileFolder = "resource/img/";

	//�Ԃ�l�p�ϐ��̒�`�A�t�H���_��������̒ǉ�
	std::string fileName = fileFolder;

	//�t�@�C����������̒ǉ�
	fileName.append(_filename);

	//const char* ret = fileName.c_str();

	//�t�@�C���̃A�h���X��Ԃ�
	return fileName;
}

ImageFileData LoadImageFile::Load(const char* _filename)
{
	ImageFileData ret;

	std::string filename = _filename;
	std::string	extension = filename.substr(filename.rfind(".",filename.size()));

	//bmp�t�@�C���̓ǂݍ���
	if (extension == ".bmp")
	{
		//bmpheader�ϐ��̒�`
		BITMAPFILEHEADER bmpFileHeader = {};
		BITMAPINFOHEADER bmpInfoHeader = {};

		//
		std::vector<char> data;
		
		FILE* fp;
		fp = fopen(_filename, "rb");

		//header�e��ւ�bmp�f�[�^�ǂݍ���
		fread(&bmpFileHeader, sizeof(bmpFileHeader), 1, fp);
		fread(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, fp);

		//�T�C�Y�m��
		data.resize(bmpInfoHeader.biWidth * bmpInfoHeader.biHeight * 4);
		ret.data.resize(data.size());

		//���]�ǂݍ��ݖh�~�̂��߈���ǂݍ���
		for (int line = bmpInfoHeader.biHeight - 1; line >= 0; --line)
		{
			for (int count = 0; count < bmpInfoHeader.biWidth * 4; count += 4)
			{
				unsigned int address = line * bmpInfoHeader.biWidth * 4;
				data[address + count] = 0;
				fread(&data[address + count + 1], sizeof(unsigned char), 3, fp);
			}
		}
		fclose(fp);

		//�Ԃ�l�p�ϐ��ւ̑��
		ret.imageSize = bmpInfoHeader.biSizeImage;
		ret.width = bmpInfoHeader.biWidth;
		ret.height = bmpInfoHeader.biHeight;
		std::copy(data.begin(), data.end(), ret.data.begin());
	}

	return ret;
}

