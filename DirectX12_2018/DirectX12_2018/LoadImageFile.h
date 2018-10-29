#pragma once
#include <string>
#include <vector>

struct ImageFileData
{
	unsigned int imageSize;
	unsigned int width;
	unsigned int height;
	std::vector<char> data;
};

class LoadImageFile
{
public:
	LoadImageFile();
	~LoadImageFile();

	const char* SearchImageFile(const char * _filename);

	ImageFileData Load(const char* _filename);
};

