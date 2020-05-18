#include "Util/crc/checksum.h"
#include "UGXFile.h"
#include "Util/tinyxml2.h"
#include "Util/bitconverter.h"
#include "Util/util.h"
#include "Util/defs.h"
#include "Util/bvec.h"
#include "Util/xmlutils.h"
#include "granny.h"
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace tinyxml2;

int UGXFile::Open(string s)
{
	FILE* f = fopen(s.c_str(), "rb");
	if (f == NULL) return -1;
	else 
	{
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, 0, SEEK_SET);

		byte* buffer = new byte[size];
		fread(buffer, size, 1, f);
		fclose(f);

		if (*reinterpret_cast<int32_t*>(&buffer[0]) != 930593498) return -2;

		//header
		for (int i = 0; i < 152; i++) headerData.push_back(buffer[i]);
		int grannyOffset = BitConverter::ToInt32(&headerData[40]),
			grannySize = BitConverter::ToInt32(&headerData[44]),
			cachedOffset = BitConverter::ToInt32(&headerData[64]),
			cachedSize = BitConverter::ToInt32(&headerData[68]),
			vertexOffset = BitConverter::ToInt32(&headerData[88]),
			vertexSize = BitConverter::ToInt32(&headerData[92]),
			indexOffset = BitConverter::ToInt32(&headerData[112]),
			indexSize = BitConverter::ToInt32(&headerData[116]),
			materialOffset = BitConverter::ToInt32(&headerData[136]),
			materialSize = BitConverter::ToInt32(&headerData[140]);

		for (int i = 0; i < grannySize; i++)   grannyData.push_back(buffer[grannyOffset + i]);
		for (int i = 0; i < cachedSize; i++)   cachedData.push_back(buffer[cachedOffset + i]);
		for (int i = 0; i < vertexSize; i++)   vertexData.push_back(buffer[vertexOffset + i]);
		for (int i = 0; i < indexSize;  i++)   indexData.push_back(buffer[indexOffset + i]);
		for (int i = 0; i < materialSize; i++) materialData.push_back(buffer[materialOffset + i]);

		return 1;
	}
}
int UGXFile::Save(string s)
{
	FILE* f = fopen(s.c_str(), "wb");

	BVec::ReplaceRange(&headerData[40], 4, BitConverter::GetBytesI32(152, BitConverter::BigE));  //grannyOffset
	BVec::ReplaceRange(&headerData[64], 4, BitConverter::GetBytesI32(152 + grannyData.size(), BitConverter::BigE));  //cachedOffset
	BVec::ReplaceRange(&headerData[88], 4, BitConverter::GetBytesI32(152 + grannyData.size() + cachedData.size(), BitConverter::BigE));  //vertexOffset
	BVec::ReplaceRange(&headerData[112], 4, BitConverter::GetBytesI32(152 + grannyData.size() + cachedData.size() + vertexData.size(), BitConverter::BigE));  //indexOffset
	BVec::ReplaceRange(&headerData[136], 4, BitConverter::GetBytesI32(152 + grannyData.size() + cachedData.size() + vertexData.size() + indexData.size(), BitConverter::BigE));  //materialOffset

	BVec::ReplaceRange(&headerData[44], 4, BitConverter::GetBytesI32(grannyData.size(),    BitConverter::BigE));  //grannySize
	BVec::ReplaceRange(&headerData[68], 4, BitConverter::GetBytesI32(cachedData.size(),    BitConverter::BigE));  //cachedSize
	BVec::ReplaceRange(&headerData[92], 4, BitConverter::GetBytesI32(vertexData.size(),    BitConverter::BigE));  //vertexSize
	BVec::ReplaceRange(&headerData[116], 4, BitConverter::GetBytesI32(indexData.size() ,   BitConverter::BigE));  //indexSize
	BVec::ReplaceRange(&headerData[140], 4, BitConverter::GetBytesI32(materialData.size(), BitConverter::BigE));  //materialSize
	
	std::cout << (int)BitConverter::GetBytesI32(grannyData.size())[0] << '\n';
	std::cout << (int)BitConverter::GetBytesI32(grannyData.size())[1] << '\n';
	std::cout << (int)BitConverter::GetBytesI32(grannyData.size())[2] << '\n';
	std::cout << (int)BitConverter::GetBytesI32(grannyData.size())[3] << '\n';

	BVec::ReplaceRange(&headerData[12], 4, BitConverter::GetBytesI32(152 + grannyData.size() + cachedData.size() + vertexData.size() + indexData.size() + materialData.size(), BitConverter::BigE));
	BVec::ReplaceRange(&headerData[8], 4, BitConverter::GetBytesI32(Util::CalcAdler32(&headerData[12], 20), BitConverter::BigE));

	fwrite(headerData.data(), headerData.size(), 1, f);
	fwrite(grannyData.data(), grannyData.size(), 1, f);
	fwrite(cachedData.data(), cachedData.size(), 1, f);
	fwrite(vertexData.data(), vertexData.size(), 1, f);
	fwrite(indexData.data(),  indexData.size(),  1, f);
	fwrite(materialData.data(), materialData.size(), 1, f);
	fclose(f);

	return 1;
}
UGXFile UGXFile::FromGR2(string gr2Path)
{
	UGXFile f;
	//prepare granny_file_info
	granny_file* gf = GrannyReadEntireFile(gr2Path.c_str());
	if (gf == NULL) { f.status = "granny_file was null."; return f; }
	granny_file_info* gfi = GrannyGetFileInfo(gf);
	if (gfi == NULL) { f.status = "granny_file_info was null."; return f; }
	gfi->FromFileName = "gr2ugx";


#pragma region insert ecf header
	std::vector<byte> newHeaderData;

	byte ecfHeader[] = { 0xDA, 0xBA, 0x77, 0x37, 0x00, 0x00, 0x00, 0x20, 0x2A, 0x07, 0x03, 0x26, 0x00, 0x00, 0x41, 0xEF
						, 0x00, 0x05, 0x00, 0x00, 0xAA, 0xC9, 0x37, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						, 0xAC, 0x4D, 0x0A, 0xEE, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00
						, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x6D, 0x4E, 0x04, 0x00, 0x04, 0x00, 0x00
						, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						, 0x58, 0x2C, 0xC9, 0xC6, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x01
						, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x06, 0x0D, 0x78, 0x00, 0x05, 0x00, 0x01
						, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						, 0x7C, 0x79, 0xE9, 0x44, 0x00, 0x02, 0x00, 0x00 };

	BVec::AddToVectorFromPtr(newHeaderData, ecfHeader, 152);
	f.headerData = newHeaderData;
#pragma endregion
#pragma region create cached data
	std::vector<byte> newCachedData;
	//magic
	byte magic[] = { 0x04, 0x00, 0x34, 0xC2 };
	BVec::AddToVectorFromPtr(newCachedData, magic, 4);

#pragma region header

	//rigid bone index
	byte* rigidBoneIndex = BitConverter::GetBytesI32(0);
	BVec::AddToVectorFromPtr(newCachedData, rigidBoneIndex, 4);

	//bounding sphere center
	float32 boundingSphereCenter[] = { 1.f, 1.f, 1.f };
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundingSphereCenter[0]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundingSphereCenter[1]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundingSphereCenter[2]), 4);

	//bounding sphere radius
	float32 boundingSphereRadius = 1.f;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundingSphereRadius), 4);

	//bounding box
	float32 boundsA[] = { -1.f, -1.f, -1.f }; //lower
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsA[0]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsA[1]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsA[2]), 4);
	float32 boundsB[] = { 1.f, 1.f, 1.f };    //upper
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsB[0]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsB[1]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsB[2]), 4);

	//instances (?)
	int16 maxInstances = 4;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI16(maxInstances), 2);
	int16 instanceIndexMultiplier = 256;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI16(instanceIndexMultiplier), 2);

	//large geom bone index (?)
	int16 largeGeomBoneIndex = 32767;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI16(largeGeomBoneIndex), 2);

	//flags (0 or 1)
	byte allSectionsRigid = 1;
	for (int i = 0; i < gfi->MeshCount; i++)
	{
		if (!GrannyMeshIsRigid(gfi->Meshes[i])) { allSectionsRigid = 0; break; }
	}
	BVec::AddToVectorFromPtr(newCachedData, &allSectionsRigid, 1);

	byte globalBones = 1;
	BVec::AddToVectorFromPtr(newCachedData, &globalBones, 1);

	byte allSectionSkinned = 1;
	for (int i = 0; i < gfi->MeshCount; i++)
	{
		if (GrannyMeshIsRigid(gfi->Meshes[i])) { allSectionSkinned = 0; break; }
	}
	BVec::AddToVectorFromPtr(newCachedData, &allSectionSkinned, 1);

	byte rigidOnly = allSectionsRigid;
	BVec::AddToVectorFromPtr(newCachedData, &rigidOnly, 1);

	//unknown (padding?)
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI64(0), 6);

	//sbudata headers
	int32 subDataOffsetBase = 160;
	//meshes
	uint64 numMeshes = (uint64)gfi->MeshCount;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numMeshes), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase), 8);
	//bones
	uint64 numBones = (uint64)gfi->Skeletons[0]->BoneCount;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numBones), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes)), 8);
	//accessories (?)
	uint64 numAccessories = 0;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numAccessories), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes) + (numBones * 80)), 8);
	//valid accessories (?)
	uint64 numValidAccessories = 0;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numValidAccessories), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes) + (numBones * 80) + (numAccessories * 16)), 8);
	//bone bounds low
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numBones), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes) + (numBones * 80) + (numAccessories * 16) + (numValidAccessories * 4)), 8);
	//bone bounds high
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numBones), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes) + (numBones * 80) + (numAccessories * 16) + (numValidAccessories * 4) + (numBones * 12)), 8);


#pragma endregion
#pragma region meshes

	for (int i = 0; i < gfi->MeshCount; i++)
	{
		bool isRigid = GrannyMeshIsRigid(gfi->Meshes[i]);

		//material id
		int32 materialID = 0;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(materialID), 4);

		//accessory index
		int32 accessoryIndex = 0;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(accessoryIndex), 4);

		//max bones (?)
		int32 maxBones = 1;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(maxBones), 4);

		//rigid bone index
		int32 rigidBoneIndex = 0;
		if (!isRigid) rigidBoneIndex = 0xFFFFFF7F;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(rigidBoneIndex, BitConverter::BigE), 4);

		//index buffer offset (in shorts)
		int32 indexBufferOffset = 0;
		for (int j = 0; j < i; j++)
		{
			indexBufferOffset += GrannyGetMeshIndexCount(gfi->Meshes[j]);
		}
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(indexBufferOffset), 4);

		//number of triangles
		int32 numberOfTris = GrannyGetMeshTriangleCount(gfi->Meshes[i]);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(numberOfTris), 4);

		//vertex size declaration (needed for buffer offset & buffer size)
		int32 vertexSize = 0;
		if (isRigid) vertexSize = 24;
		if (!isRigid) vertexSize = 32;

		//vertex buffer offset (in bytes)
		int32 vertexBufferOffset = 0;
		for (int j = 0; j < i; j++)
		{
			vertexBufferOffset += gfi->Meshes[j]->PrimaryVertexData->VertexCount;
		}
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(vertexBufferOffset * vertexSize), 4);

		//vertex buffer size (in bytes)
		int32 vertexBufferSize = gfi->Meshes[i]->PrimaryVertexData->VertexCount * vertexSize;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(vertexBufferSize), 4);

		//vertex size write
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(vertexSize), 4);

		//number of vertices
		int32 numberOfVerts = gfi->Meshes[i]->PrimaryVertexData->VertexCount;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(numberOfVerts), 4);

		//misc
		byte padding[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 };
		BVec::AddToVectorFromPtr(newCachedData, padding, 16);

		int64 packOrderOffset = 160 + (152 * gfi->MeshCount) + (8 * i);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI64(packOrderOffset), 8);

		byte baseVertPacker[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
									0x10, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
									0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
									0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
									0x09, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		BVec::AddToVectorFromPtr(newCachedData, baseVertPacker, 88);
	}

	//packing order
	for (int i = 0; i < gfi->MeshCount; i++)
	{
		if (GrannyMeshIsRigid(gfi->Meshes[i]))
		{
			byte* packOrder = new byte[8]{ 0x50, 0x4E, 0x54, 0x30, 0x00, 0x00, 0x00, 0x00 }; //PNT0
			BVec::AddToVectorFromPtr(newCachedData, packOrder, 8);
		}
		else
		{
			byte* packOrder = new byte[8]{ 0x50, 0x4E, 0x53, 0x54, 0x30, 0x00, 0x00, 0x00 }; //PNST0
			BVec::AddToVectorFromPtr(newCachedData, packOrder, 8);
		}
	}

#pragma endregion
#pragma region bones

	for (int i = 0; i < gfi->Skeletons[0]->BoneCount; i++)
	{
		//name offset
		int64 nameOffset = (gfi->Skeletons[0]->BoneCount * 80) + (32 * i);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI64(nameOffset), 8);

		//world matrix
		//for (int i = 0; i < 64; i++) { newCachedData.push_back(0x00); }
		BVec::AddToVectorFromPtr(newCachedData, (byte*)gfi->Skeletons[0]->Bones[i].InverseWorld4x4, 64);

		//parent index
		int64 parentIndex = gfi->Skeletons[0]->Bones[i].ParentIndex;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI64(parentIndex), 8);
	}

	//write bone names
	for (int i = 0; i < gfi->Skeletons[0]->BoneCount; i++)
	{
		int32 nameLen = strlen(gfi->Skeletons[0]->Bones[i].Name);
		BVec::AddToVectorFromPtr(newCachedData, (byte*)gfi->Skeletons[0]->Bones[i].Name, nameLen);
		for (int j = nameLen; j < 32; j++)
		{
			int16 k = 0;
			BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI16(k), 1);
		}
	}

#pragma endregion

	f.cachedData = newCachedData;
#pragma endregion
#pragma region create vertex data
	std::vector<byte> newVertexData;
#pragma region vertex structures

	struct PNT0
	{
		granny_real16 position[4];
		granny_real32 normal[3];
		granny_real16 uv[2];
	};
	granny_data_type_definition PNT0_def[]
	{
		{ GrannyReal16Member, GrannyVertexPositionName, 0, 4 },
		{ GrannyReal32Member, GrannyVertexNormalName, 0, 3 },
		{ GrannyReal16Member, "TextureCoordinates0", 0, 2 },
		{ GrannyEndMember }
	};

	struct PNST0
	{
		granny_real16 position[4];
		granny_real32 normal[3];
		granny_uint8 bone[4];
		granny_uint8 weight[4];
		granny_real16 uv[2];
	};
	granny_data_type_definition PNST0_def[]
	{
		{ GrannyReal16Member, GrannyVertexPositionName, 0, 4 },
		{ GrannyReal32Member, GrannyVertexNormalName, 0, 3 },
		{ GrannyUInt8Member, GrannyVertexBoneIndicesName, 0, 4},
		{ GrannyUInt8Member, GrannyVertexBoneWeightsName, 0, 4},
		{ GrannyReal16Member, "TextureCoordinates0", 0, 2 },
		{ GrannyEndMember }
	};
#pragma endregion

	for (int i = 0; i < gfi->MeshCount; i++)
	{
		if (GrannyMeshIsRigid(gfi->Meshes[i])) //PNT0
		{
			void* vData = malloc(sizeof(PNT0) * GrannyGetMeshVertexCount(gfi->Meshes[i]));
			GrannyCopyMeshVertices(gfi->Meshes[i], PNT0_def, vData);

			BVec::AddToVectorFromPtr(newVertexData, (byte*)vData, sizeof(PNT0) * GrannyGetMeshVertexCount(gfi->Meshes[i]));
		}
		else
		{
			granny_mesh_binding* gmb = GrannyNewMeshBinding(gfi->Meshes[0], gfi->Skeletons[0], gfi->Skeletons[0]);
			const granny_int32* mb = GrannyGetMeshBindingToBoneIndices(gmb);

			void* vData = malloc(sizeof(PNST0) * GrannyGetMeshVertexCount(gfi->Meshes[i]));
			GrannyCopyMeshVertices(gfi->Meshes[i], PNST0_def, vData);

			BVec::AddToVectorFromPtr(newVertexData, (byte*)vData, sizeof(PNST0) * GrannyGetMeshVertexCount(gfi->Meshes[i]));

			for (int j = 0; j < GrannyGetMeshVertexCount(gfi->Meshes[i]); j++)
			{
				for (int k = 0; k < 4; k++)
				{
					newVertexData[(j * 32) + 20 + k] = (mb[(int)newVertexData[(j * 32) + 20 + k]] + 1);
				}
			}
		}
	}
	f.vertexData = newVertexData;
#pragma endregion
#pragma region create index data
	std::vector<byte> newIndexData;
for (int i = 0; i < gfi->MeshCount; i++)
{
	void* iData = malloc(2 * GrannyGetMeshIndexCount(gfi->Meshes[i]));
	GrannyCopyMeshIndices(gfi->Meshes[i], 2, iData);
	BVec::AddToVectorFromPtr(newIndexData, (byte*)iData, 2 * GrannyGetMeshIndexCount(gfi->Meshes[i]));
}
f.indexData = newIndexData;
#pragma endregion
#pragma region create material data

	int32 fileExtIndex = gr2Path.rfind('.');
	string matPath = gr2Path.substr(0, fileExtIndex) + ".xml";

	std::vector<Node> nodes;
	std::vector<NameValue> names;
	std::vector<byte> data;

	string s = ParseXML(matPath, nodes, names, data);
	if (s != "OK") {
		f.status = "error creating material chunk from xml: " + s;
		return f;
	}

	//write
	std::vector<byte> matHeader = {
		0x3E, 0x07, 0x5C, 0x00, 0xC9, 0x3C, 0x37, 0xBB, 0x03, 0x04, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x2A, 0x01, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00 };
	std::vector<byte> nodesData;
	std::vector<byte> nameValueData;
	std::vector<byte> nameData = {
		0x4D, 0x61, 0x74, 0x65, 0x72, 0x69, 0x61, 0x6C, 0x73, 0x00, 0x4D, 0x61, 0x74, 0x65, 0x72, 0x69,
		0x61, 0x6C, 0x00, 0x4E, 0x61, 0x6D, 0x65, 0x00, 0x56, 0x65, 0x72, 0x00, 0x4E, 0x61, 0x6D, 0x65,
		0x56, 0x61, 0x6C, 0x75, 0x65, 0x73, 0x00, 0x4D, 0x61, 0x70, 0x73, 0x00, 0x64, 0x69, 0x66, 0x66,
		0x75, 0x73, 0x65, 0x00, 0x55, 0x56, 0x57, 0x56, 0x65, 0x6C, 0x00, 0x6E, 0x6F, 0x72, 0x6D, 0x61,
		0x6C, 0x00, 0x67, 0x6C, 0x6F, 0x73, 0x73, 0x00, 0x6F, 0x70, 0x61, 0x63, 0x69, 0x74, 0x79, 0x00,
		0x78, 0x66, 0x6F, 0x72, 0x6D, 0x00, 0x65, 0x6D, 0x69, 0x73, 0x73, 0x69, 0x76, 0x65, 0x00, 0x61,
		0x6F, 0x00, 0x65, 0x6E, 0x76, 0x00, 0x65, 0x6E, 0x76, 0x6D, 0x61, 0x73, 0x6B, 0x00, 0x65, 0x6D,
		0x78, 0x66, 0x6F, 0x72, 0x6D, 0x00, 0x64, 0x69, 0x73, 0x74, 0x6F, 0x72, 0x74, 0x69, 0x6F, 0x6E,
		0x00, 0x68, 0x69, 0x67, 0x68, 0x6C, 0x69, 0x67, 0x68, 0x74, 0x00, 0x6D, 0x6F, 0x64, 0x75, 0x6C,
		0x61, 0x74, 0x65, 0x00, 0x4D, 0x61, 0x70, 0x00, 0x43, 0x68, 0x61, 0x6E, 0x6E, 0x65, 0x6C, 0x00,
		0x46, 0x6C, 0x61, 0x67, 0x73, 0x00, 0x53, 0x70, 0x65, 0x63, 0x50, 0x6F, 0x77, 0x65, 0x72, 0x00,
		0x53, 0x70, 0x65, 0x63, 0x43, 0x6F, 0x6C, 0x6F, 0x72, 0x52, 0x00, 0x53, 0x70, 0x65, 0x63, 0x43,
		0x6F, 0x6C, 0x6F, 0x72, 0x47, 0x00, 0x53, 0x70, 0x65, 0x63, 0x43, 0x6F, 0x6C, 0x6F, 0x72, 0x42,
		0x00, 0x45, 0x6E, 0x76, 0x52, 0x65, 0x66, 0x6C, 0x65, 0x63, 0x74, 0x69, 0x76, 0x69, 0x74, 0x79,
		0x00, 0x45, 0x6E, 0x76, 0x53, 0x68, 0x61, 0x72, 0x70, 0x6E, 0x65, 0x73, 0x73, 0x00, 0x45, 0x6E,
		0x76, 0x46, 0x72, 0x65, 0x73, 0x6E, 0x65, 0x6C, 0x00, 0x45, 0x6E, 0x76, 0x46, 0x72, 0x65, 0x73,
		0x6E, 0x65, 0x6C, 0x50, 0x6F, 0x77, 0x65, 0x72, 0x00, 0x41, 0x63, 0x63, 0x65, 0x73, 0x73, 0x6F,
		0x72, 0x79, 0x49, 0x6E, 0x64, 0x65, 0x78, 0x00, 0x42, 0x6C, 0x65, 0x6E, 0x64, 0x54, 0x79, 0x70,
		0x65, 0x00, 0x4F, 0x70, 0x61, 0x63, 0x69, 0x74, 0x79, 0x00
	};
	std::vector<byte> valueData = data;
	for (int i = 0; i < nodes.size(); i++)
	{
		nodesData.push_back(BitConverter::GetBytesUI16(nodes[i].parentIndex)[0]); nodesData.push_back(BitConverter::GetBytesUI16(nodes[i].parentIndex)[1]);
		nodesData.push_back(BitConverter::GetBytesUI16(nodes[i].childNodeIndex)[0]); nodesData.push_back(BitConverter::GetBytesUI16(nodes[i].childNodeIndex)[1]);
		nodesData.push_back(BitConverter::GetBytesUI16(nodes[i].nameValueIndex)[0]); nodesData.push_back(BitConverter::GetBytesUI16(nodes[i].nameValueIndex)[1]);
		nodesData.push_back(*BitConverter::GetBytesUI16(nodes[i].nameValueCount));
		nodesData.push_back(*BitConverter::GetBytesUI16(nodes[i].childNodeCount));
	}
	for (int i = 0; i < names.size(); i++)
	{
		nameValueData.push_back(BitConverter::GetBytesUI32(names[i].value)[0]); nameValueData.push_back(BitConverter::GetBytesUI32(names[i].value)[1]);
		nameValueData.push_back(BitConverter::GetBytesUI32(names[i].value)[2]); nameValueData.push_back(BitConverter::GetBytesUI32(names[i].value)[3]);
		nameValueData.push_back(BitConverter::GetBytesUI16(names[i].nameOffset)[0]); nameValueData.push_back(BitConverter::GetBytesUI16(names[i].nameOffset)[1]);
		nameValueData.push_back(BitConverter::GetBytesUI16(names[i].flags)[0]); nameValueData.push_back(BitConverter::GetBytesUI16(names[i].flags)[1]);
	}

	BVec::ReplaceRange(&matHeader[12], 4, BitConverter::GetBytesUI32(nodesData.size()));	  //replace nodeData size
	BVec::ReplaceRange(&matHeader[16], 4, BitConverter::GetBytesUI32(nameValueData.size()));  //replace nameValueData size
	BVec::ReplaceRange(&matHeader[20], 4, BitConverter::GetBytesUI32(nameData.size()));		  //replace nameData size (always 298)
	BVec::ReplaceRange(&matHeader[24], 4, BitConverter::GetBytesUI32(valueData.size()));	  //replace valueData size

	int namePaddingSize = 16 - ((matHeader.size() + nameData.size() + nodesData.size() + nameValueData.size()) % 16);

	vector<byte> tempData;
	for (int i = 0; i < nodesData.size(); i++) {
		tempData.push_back(nodesData[i]);
	}
	for (int i = 0; i < nameValueData.size(); i++) {
		tempData.push_back(nameValueData[i]);
	}
	for (int i = 0; i < nameData.size(); i++) {
		tempData.push_back(nameData[i]);
	}
	for (int i = 0; i < namePaddingSize; i++) {
		tempData.push_back(0x00);
	}
	for (int i = 0; i < valueData.size(); i++) {
		tempData.push_back(valueData[i]);
	}

	BVec::ReplaceRange(&matHeader[4], 4, BitConverter::GetBytesUI32(crc_32(tempData.data(), tempData.size()))); //get crc32
	BVec::ReplaceRange(&matHeader[8], 4, BitConverter::GetBytesUI32(nameData.size() + nodesData.size() + nameValueData.size() + valueData.size() + namePaddingSize)); //write size

	matHeader[2] = 0x00;
	matHeader[2] = BitConverter::GetBytesUI16(crc_ccitt_ffff(&matHeader[0], 28))[0]; //crc16 ccitt xor'd with 0xffff ("GENIBus")

	for (int i = 0; i < 28; i++) {
		f.materialData.push_back(matHeader[i]);
	}
	for (int i = 0; i < tempData.size(); i++) {
		f.materialData.push_back(tempData[i]);
	}


#pragma endregion
#pragma region write granny_file_info

	gfi->VertexDataCount = 0;
	gfi->VertexDatas = NULL;
	gfi->TriTopologyCount = 0;
	gfi->TriTopologies = NULL;
	gfi->Meshes[0]->PrimaryVertexData = NULL;
	gfi->Meshes[0]->PrimaryTopology = NULL;
	GrannyRebasePointers(GrannyFileInfoType, gfi, 0, false);
	GrannyConvertFileInfoToRaw(gfi, "tmpgrx");
	FILE* tmpgrx = fopen("tmpgrx", "rb");
	if (tmpgrx == NULL) { f.status = "tmpgrx file was null."; return f; }
	else
	{
		fseek(tmpgrx, 0, SEEK_END);
		int size = ftell(tmpgrx);
		fseek(tmpgrx, 0, SEEK_SET);

		byte* buffer = new byte[size];
		fread(buffer, size, 1, tmpgrx);
		fclose(tmpgrx);
		f.grannyData.clear();
		f.grannyData.resize(size);
		for (int i = 0; i < size; i++) { f.grannyData[i] = buffer[i]; }
		remove("tmpgrx");
	}

#pragma endregion

	f.status = "OK";
	return f;
}

int CreateUAXs(string s)
{
	granny_file* gf = GrannyReadEntireFile(s.c_str());
	if (gf == NULL) { std::cout << "granny_file was null.\n"; return -64; };
	granny_file_info* gfi = GrannyGetFileInfo(gf);
	if (gfi == NULL) { std::cout << "granny_file_info was null.\n"; return -65; };
	gfi->FromFileName = "gr2ugx";

	unsigned char header[64] = {
		0xDA, 0xBA, 0x77, 0x37, 0x00, 0x00, 0x00, 0x20, 0x27, 0x7B, 0x02, 0xFF, 0x00, 0x00, 0x4C, 0xC0,
		0x00, 0x01, 0x00, 0x00, 0xAA, 0xC9, 0x37, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x4C, 0x80,
		0xDA, 0x15, 0x99, 0xBF, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	std::vector<byte> uax;

	//GrannyMakeIdentity(&gfi->Skeletons[0]->Bones[0].LocalTransform);
	//GrannyMakeIdentity(&gfi->Animations[0]->TrackGroups[0]->InitialPlacement);
	for (int i = 0; i < gfi->Animations[0]->TrackGroupCount; i++)
	{
		//GrannyBuildInverse(&gfi->Animations[0]->TrackGroups[i]->InitialPlacement, &gfi->Animations[0]->TrackGroups[i]->InitialPlacement);
		//transformtrack
	}

	gfi->VertexDataCount = 0;
	gfi->VertexDatas = NULL;
	gfi->TextureCount = 0;
	gfi->Textures = NULL;
	gfi->MeshCount = 0;
	gfi->Meshes = NULL;
	gfi->ModelCount = 0;
	gfi->Models = NULL;
	gfi->SkeletonCount = 0;
	gfi->Skeletons = NULL;
	gfi->TriTopologyCount = 0;
	gfi->TriTopologies = NULL;
	GrannyRebasePointers(GrannyFileInfoType, gfi, 0, false);
	GrannyConvertFileInfoToRaw(gfi, "tmpuax");
	FILE* tmpgrx = fopen("tmpuax", "rb");
	fseek(tmpgrx, 0, SEEK_END);
	int size = ftell(tmpgrx);
	fseek(tmpgrx, 0, SEEK_SET);


	byte* buffer = new byte[size];
	fread(buffer, size, 1, tmpgrx);
	fclose(tmpgrx);
	uax.resize(size);
	for (int i = 0; i < size; i++) { uax[i] = buffer[i]; }
	remove("tmpuax");

	BVec::ReplaceRange(&header[44], 4, BitConverter::GetBytesI32(size, BitConverter::BigE));
	BVec::ReplaceRange(&header[12], 4, BitConverter::GetBytesI32(64 + uax.size(), BitConverter::BigE));
	BVec::ReplaceRange(&header[8], 4, BitConverter::GetBytesI32(Util::CalcAdler32(&header[12], 20), BitConverter::BigE));

	s = s.substr(0, s.find(".gr2"));

	FILE* f = fopen((s + ".uax").c_str(), "wb");
	fwrite(header, 64, 1, f);
	fwrite(&uax[0], uax.size(), 1, f);
	fclose(f);

	std::cout << "UAX saved to " + s + ".uax\n";

	return 1;
}