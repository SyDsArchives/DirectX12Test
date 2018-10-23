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

void LoadImageFile::Load(std::string& _filename)
{
	//bmp
	//ÉeÉNÉXÉ`ÉÉÇÃì«Ç›çûÇ›
	BITMAPFILEHEADER bmpFileHeader = {};
	BITMAPINFOHEADER bmpInfoHeader = {};

	//FILE* tiles;
	//tiles = fopen("resource/img/tiles.bmp", "rb");
	std::vector<char> imgdata;
	
	FILE* fp;
	//fp = fopen(_filename.c_str, "rb");

	fread(&bmpFileHeader, sizeof(bmpFileHeader), 1, fp);
	fread(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, fp);
	imgdata.resize(bmpInfoHeader.biWidth * bmpInfoHeader.biHeight * 4);
	//îΩì]ì«Ç›çûÇ›ñhé~ÇÃÇΩÇﬂàÍÇ¬Ç∏Ç¬ì«Ç›çûÇﬁ
	for (int line = bmpInfoHeader.biHeight - 1; line >= 0; --line)
	{
		for (int count = 0; count < bmpInfoHeader.biWidth * 4; count += 4)
		{
			unsigned int address = line * bmpInfoHeader.biWidth * 4;
			imgdata[address + count] = 0;
			fread(&imgdata[address + count + 1], sizeof(unsigned char), 3, fp);
		}
	}
	fclose(fp);
}

