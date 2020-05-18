#pragma once
#include "defs.h"
#include <vector>

class Util
{
public:
	static uint32 CalcAdler32(byte* barr, int32 len)
	{
		const int mod = 65521;
		uint32 a = 1, b = 0;
		for (int i = 0; i < len; i++)
		{
			byte c = barr[i];
			a = (a + c) % mod;
			b = (b + a) % mod;
		}
		return (b << 16) | a;
	}
};