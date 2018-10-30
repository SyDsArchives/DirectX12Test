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
	//フォルダ名文字列の定義
	const char* fileFolder = "resource/img/";

	//返り値用変数の定義、フォルダ名文字列の追加
	std::string fileName = fileFolder;

	//ファイル名文字列の追加
	fileName.append(_filename);

	//const char* ret = fileName.c_str();

	//ファイルのアドレスを返す
	return fileName;
}

ImageFileData LoadImageFile::Load(const char* _filename)
{
	ImageFileData ret;

	std::string filename = _filename;
	std::string	extension = filename.substr(filename.rfind(".",filename.size()));

	//bmpファイルの読み込み
	if (extension == ".bmp")
	{
		//bmpheader変数の定義
		BITMAPFILEHEADER bmpFileHeader = {};
		BITMAPINFOHEADER bmpInfoHeader = {};

		//
		std::vector<char> data;
		
		FILE* fp;
		fp = fopen(_filename, "rb");

		//header各種へのbmpデータ読み込み
		fread(&bmpFileHeader, sizeof(bmpFileHeader), 1, fp);
		fread(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, fp);

		//サイズ確保
		data.resize(bmpInfoHeader.biWidth * bmpInfoHeader.biHeight * 4);
		ret.data.resize(data.size());

		//反転読み込み防止のため一つずつ読み込む
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

		//返り値用変数への代入
		ret.imageSize = bmpInfoHeader.biSizeImage;
		ret.width = bmpInfoHeader.biWidth;
		ret.height = bmpInfoHeader.biHeight;
		std::copy(data.begin(), data.end(), ret.data.begin());
	}

	return ret;
}

