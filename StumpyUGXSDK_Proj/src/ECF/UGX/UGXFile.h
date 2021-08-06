#pragma once
#include <vector>
#include <string>
#include "Util/defs.h"
#include "granny.h"

class granny_file;
class UGXFile
{
public:
	string filePath;
	string fileName;

	std::vector<byte> headerData;
	std::vector<byte> grannyData;
	std::vector<byte> cachedData;
	std::vector<byte> vertexData;
	std::vector<byte> indexData;
	std::vector<byte> materialData;

	int Open(string path);
	static UGXFile FromGR2(bool verbose, granny_file_info* gf);
	int Save(string path);

	string status;
};

//TODO: move to new file.
granny_file_info* LoadAndPreprocessGR2(granny_file*& out_gf, string path, string front, string right, string up);
int CreateUAX(granny_file_info* gfi, string outPath);
int CreateUGX(granny_file_info* gfi, string materialInfoPath, string outPath);