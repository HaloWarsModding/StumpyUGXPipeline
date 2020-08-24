#pragma once
#include <vector>
#include <string>
#include "Util/defs.h"

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
	static UGXFile FromGR2(string s, bool verbose);
	int Save(string path);

	string status;
};

int CreateUAXs(string s);