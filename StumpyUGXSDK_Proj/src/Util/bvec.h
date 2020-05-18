#pragma once
#include "defs.h"
#include <vector>

class BVec
{
public:
	static void ReplaceRange(byte* barr, int32 len, byte* newBArr)
	{
		for (int i = 0; i < len; i++)
		{
			barr[i] = newBArr[i];
		}
	}
	static void AddToVectorFromPtr(std::vector<byte>& bVec, byte* barr, int32 len)
	{
		for (int i = 0; i < len; i++)
		{
			bVec.push_back(barr[i]);
		}
	}
};