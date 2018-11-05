#pragma once
#include <vector>
#include <string>

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

	std::string SearchImageFile(const char* _filename);

	ImageFileData Load(const char* _filename);
};

