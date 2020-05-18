#pragma once
#include "defs.h"
#include <iostream>

class BitConverter {
private: 
	static byte* FlipBytes(byte* b, int len)
	{
		byte* newArr = new byte[len];
		for (int i = 0; i < len; i++)
		{
			newArr[i] = b[len - i - 1];
		}
		return newArr;
	}

public:

	enum Endianness { LittleE, BigE };

	static byte* GetBytesI64(int64 i, Endianness e = LittleE)
	{
		byte* b = new byte[8];
		b[0] = (i >> 0);
		b[1] = (i >> 8);
		b[2] = (i >> 16);
		b[3] = (i >> 24);
		b[4] = (i >> 32);
		b[5] = (i >> 40);
		b[6] = (i >> 48);
		b[7] = (i >> 56);
		if (e == BigE) { b = FlipBytes(b, 8); }
		else return b;
	}
	static byte* GetBytesUI64(uint64 i, Endianness e = LittleE)
	{
		byte* b = new byte[8];
		b[0] = (i >> 0);
		b[1] = (i >> 8);
		b[2] = (i >> 16);
		b[3] = (i >> 24);
		b[4] = (i >> 32);
		b[5] = (i >> 40);
		b[6] = (i >> 48);
		b[7] = (i >> 56);
		if (e == BigE) { b = FlipBytes(b, 8); }
		else return b;
	}

	static byte* GetBytesI32(int32 i, Endianness e = LittleE)
	{
		byte* b = new byte[4];
		b[0] = (i >> 0);
		b[1] = (i >> 8);
		b[2] = (i >> 16);
		b[3] = (i >> 24);
		if (e == BigE) { b = FlipBytes(b, 4); }
		else return b;
	}	
	static byte* GetBytesUI32(uint32 i, Endianness e = LittleE)
	{
		byte* b = new byte[4];
		b[0] = (i >> 0);
		b[1] = (i >> 8);
		b[2] = (i >> 16);
		b[3] = (i >> 24);
		if (e == BigE) { b = FlipBytes(b, 4); }
		else return b;
	}

	static byte* GetBytesI16(int16 i, Endianness e = LittleE)
	{
		byte* b = new byte[2];
		b[0] = (i >> 0);
		b[1] = (i >> 8);
		if (e == BigE) { b = FlipBytes(b, 2); }
		else return b;
	}	
	static byte* GetBytesUI16(uint16 i, Endianness e = LittleE)
	{
		byte* b = new byte[2];
		b[0] = (i >> 0);
		b[1] = (i >> 8);
		if (e == BigE) { b = FlipBytes(b, 2); }
		else return b;
	}

	static byte* GetBytesF32(float32 f, Endianness e = LittleE)
	{
		byte* b = new byte[4];
		int32* i = reinterpret_cast<int32*>(&f);
		b[0] = (*i >> 0);
		b[1] = (*i >> 8);
		b[2] = (*i >> 16);
		b[3] = (*i >> 24);
		if (e == BigE) { b = FlipBytes(b, 4); }
		return b;
	}
	static byte* GetBytesF16(float16 f, Endianness e = LittleE)
	{
		byte* b = new byte[2];
		int16* i = reinterpret_cast<int16*>(&f);
		b[0] = (*i >> 0);
		b[1] = (*i >> 8);
		if (e == BigE) { b = FlipBytes(b, 2); }
		return b;
	}

	static int ToInt32(byte* b, Endianness e = LittleE)
	{
		if (e == BigE) { b = FlipBytes(b, 4); }
		int i = 0;
		i = (i << 8) + b[0];
		i = (i << 8) + b[1];
		i = (i << 8) + b[2];
		i = (i << 8) + b[3];
		return i;
	}
};