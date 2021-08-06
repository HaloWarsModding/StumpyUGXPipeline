#include "Util/crc/checksum.h"
#include "UGXFile.h"
#include "Util/tinyxml2.h"
#include "Util/bitconverter.h"
#include "Util/util.h"
#include "Util/defs.h"
#include "Util/bvec.h"
#include "Util/xmlutils.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <filesystem>
#include <map>
using namespace tinyxml2;

const string sYes = "Yes";
const string sNo = "No";

struct Ori { 
	Ori() {}
	Ori(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	float x, y, z;
	~Ori() {}
};
std::map<string, Ori> oriTranslate
{
	{"X+", Ori(1,0,0)},
	{"X-", Ori(-1,0,0)},
	{"Y+", Ori(0,1,0)},
	{"Y-", Ori(0,-1,0)},
	{"Z+", Ori(0,0,1)},
	{"Z-", Ori(0,0,-1)},
};
granny_file_info* LoadAndPreprocessGR2(granny_file*& out_gf, string path, string front, string right, string up)
{
	granny_file* gf = GrannyReadEntireFile(path.c_str());
	if (gf == NULL) { std::cout << "granny_file was null.\n"; return NULL; };
	out_gf = gf;
	granny_file_info* gfi = GrannyGetFileInfo(gf);
	if (gfi == NULL) { std::cout << "granny_file_info was null.\n"; return NULL; };
	gfi->FromFileName = "gr2ugx";


	if (gfi->SkeletonCount <= 0) {
		std::cout << "No skeleton.";
		return NULL;
	}


	granny_real32 DesiredUnitsPerMeter = 1.0f;
	granny_real32 DesiredOrigin[] = { 0, 0, 0 };

	Ori oriR = oriTranslate.at(right);
	Ori oriF = oriTranslate.at(front);
	Ori oriU = oriTranslate.at(up);
	granny_real32 DesiredRight[] = { oriR.x, oriR.y, oriR.z };
	granny_real32 DesiredBack[] = { -oriF.x, -oriF.y, -oriF.z };
	granny_real32 DesiredUp[] = { oriU.x, oriU.y, oriU.z };

	std::cout << DesiredRight[0] << " " << DesiredRight[1] << " " << DesiredRight[2] << "\n" <<
		DesiredUp[0] << " " << DesiredUp[1] << " " << DesiredUp[2] << "\n" <<
		DesiredBack[0] << " " << DesiredBack[1] << " " << DesiredBack[2] << "\n";

	granny_real32 Affine3[3];
	granny_real32 Linear3x3[9];
	granny_real32 InverseLinear3x3[9];

	GrannyComputeBasisConversion(
		gfi,
		DesiredUnitsPerMeter,
		DesiredOrigin,
		DesiredRight,
		DesiredUp,
		DesiredBack,
		Affine3,
		Linear3x3,
		InverseLinear3x3);

	GrannyTransformFile(
		gfi,
		Affine3,
		Linear3x3,
		InverseLinear3x3,
		1e-5f, 1e-5f,
		GrannyRenormalizeNormals | GrannyReorderTriangleIndices);

	return gfi;
}


int UGXFile::Open(string s)
{
	FILE* f = fopen(s.c_str(), "rb");
	if (f == NULL) return -11;
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

int CreateUAX(granny_file_info* gfi, string outPath)
{

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

	string p = string(std::filesystem::temp_directory_path().string() + "tmpuax").c_str();

	GrannyConvertFileInfoToRaw(gfi, p.c_str());
	FILE* tmpgrx = fopen(p.c_str(), "rb");
	fseek(tmpgrx, 0, SEEK_END);
	int size = ftell(tmpgrx);
	fseek(tmpgrx, 0, SEEK_SET);


	byte* buffer = new byte[size];
	fread(buffer, size, 1, tmpgrx);
	fclose(tmpgrx);
	uax.resize(size);
	for (int i = 0; i < size; i++) { uax[i] = buffer[i]; }
	remove(p.c_str());

	BVec::ReplaceRange(&header[44], 4, BitConverter::GetBytesI32(size, BitConverter::BigE));
	BVec::ReplaceRange(&header[12], 4, BitConverter::GetBytesI32(64 + uax.size(), BitConverter::BigE));
	BVec::ReplaceRange(&header[8], 4, BitConverter::GetBytesI32(Util::CalcAdler32(&header[12], 20), BitConverter::BigE));
	

	FILE* f = fopen((outPath).c_str(), "wb");
	fwrite(header, 64, 1, f);
	fwrite(&uax[0], uax.size(), 1, f);
	fclose(f);
	delete buffer;

	return 1;
}
int CreateUGX(granny_file_info* gfi, string materialInfoPath, string outPath)
{
	bool verbose = true;
	UGXFile f;

	//verbose output variables
	std::vector<string> meshNames;
	std::vector<int32> meshVertexCount;
	std::vector<int32> meshIndexCount;
	std::vector<string> meshIsRigid;
	std::vector<string> boneNames;

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

	#pragma region cached data header
	//magic
	byte magic[] = { 0x04, 0x00, 0x34, 0xC2 };
	BVec::AddToVectorFromPtr(newCachedData, magic, 4);

	//rigid bone index
	byte* rigidBoneIndex = BitConverter::GetBytesI32(0);
	BVec::AddToVectorFromPtr(newCachedData, rigidBoneIndex, 4);

	//bounding sphere center
	float32 boundingSphereCenter[] = { 0.f, 2.f, 0.f };
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundingSphereCenter[0]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundingSphereCenter[1]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundingSphereCenter[2]), 4);

	//bounding sphere radius
	float32 boundingSphereRadius = 10.f;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundingSphereRadius), 4);

	//bounding box
	float32 boundsA[] = { 10, 10.f, 10.f }; //lower
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsA[0]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsA[1]), 4);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(boundsA[2]), 4);
	float32 boundsB[] = { -10.f, -10.f, -10.f };    //upper
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

	int* isRigid = new int[gfi->MeshCount];

	for (int i = 0; i < gfi->MeshCount; i++)
	{
		isRigid[i] = false;
	}

	//flags (0 or 1)
	byte allSectionsRigid = 1;
	for (int i = 0; i < gfi->MeshCount; i++)
	{
		if (!isRigid[i]) { allSectionsRigid = 0; break; }
	}
	BVec::AddToVectorFromPtr(newCachedData, &allSectionsRigid, 1);

	byte globalBones = 1;
	BVec::AddToVectorFromPtr(newCachedData, &globalBones, 1);

	byte allSectionSkinned = 1;
	for (int i = 0; i < gfi->MeshCount; i++)
	{
		if (isRigid[i]) { allSectionSkinned = 0; break; }
	}
	BVec::AddToVectorFromPtr(newCachedData, &allSectionSkinned, 1);

	byte rigidOnly = allSectionsRigid;
	BVec::AddToVectorFromPtr(newCachedData, &rigidOnly, 1);

	//unknown (padding?)
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI64(0), 6);

	//subdata headers
	int32 subDataOffsetBase = 160;
	//meshes
	uint64 numMeshes = (uint64)gfi->MeshCount;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numMeshes), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase), 8);
	//bones
	if (gfi->Skeletons == NULL) { f.status = "No Skeleton"; return -3; }
	uint64 numBones = (uint64)gfi->Skeletons[0]->BoneCount;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numBones), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes)), 8);
	//accessories
	uint64 numAccessories = (uint64)gfi->MeshCount;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numAccessories), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes) + (numBones * 112)), 8);
	//valid accessories (?)
	uint64 numValidAccessories = (uint64)gfi->MeshCount;
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numValidAccessories), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes) + (numBones * 112) + (numAccessories * 28)), 8);
	//bone bounds low
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numBones), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes) + (numBones * 112) + (numAccessories * 28) + (numValidAccessories * 4)), 8);
	//bone bounds high
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(numBones), 8);
	BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesUI64(subDataOffsetBase + (160 * numMeshes) + (numBones * 112) + (numAccessories * 28) + (numValidAccessories * 4) + (numBones * 12)), 8);


	#pragma endregion
	#pragma region meshes

	//for material indices
	std::map<string, int32> matIndices;

	string matIndexPath = materialInfoPath + ".matindex";

	std::ifstream fileF(matIndexPath.c_str());
	std::vector<char> fileB(
		(std::istreambuf_iterator<char>(fileF)),
		(std::istreambuf_iterator<char>()));
	fileF.close();

	string num(&fileB.data()[0]);
	int numObjs = std::stoi(num);
	int cur = num.length() + 1; //pass the null char

	for (int i = 0; i < numObjs; i++)
	{
		string name(&fileB.data()[cur]);
		cur += name.length() + 1;
		string index(&fileB.data()[cur]);
		cur += index.length() + 1;
		matIndices.insert(std::make_pair(name, std::stoi(index)));
	}

	//for OBB calculation
	struct Pos
	{
		granny_real32 position[3];
	};
	granny_data_type_definition P_def[]
	{
		{ GrannyReal32Member, GrannyVertexPositionName, 0, 3 },
		{ GrannyEndMember }
	};


	for (int i = 0; i < numMeshes; i++)
	{
		#pragma region calculate OBBs
		//boneBinding OBBs need to be calculated and set manually. why? who knows.

		int vData_Size = sizeof(Pos) * GrannyGetMeshVertexCount(gfi->Meshes[i]);
		void* vData_ = malloc(vData_Size);
		GrannyCopyMeshVertices(gfi->Meshes[i], P_def, vData_);

		granny_real32 minx = 0, miny = 0, minz = 0, maxx = 0, maxy = 0, maxz = 0;
		for (int q = 0; q < GrannyGetMeshVertexCount(gfi->Meshes[i]); q++)
		{
			Pos* p = reinterpret_cast<Pos*>(vData_);

			if (p[q].position[0] < minx) minx = p[q].position[0];
			if (p[q].position[1] < miny) miny = p[q].position[1];
			if (p[q].position[2] < minz) minz = p[q].position[2];

			if (p[q].position[0] > maxx) maxx = p[q].position[0];
			if (p[q].position[1] > maxy) maxy = p[q].position[1];
			if (p[q].position[2] > maxz) maxz = p[q].position[2];
		}

		for (int j = 0; j < gfi->Meshes[i]->BoneBindingCount; j++)
		{
			gfi->Meshes[i]->BoneBindings[j].OBBMin[0] = minx;
			gfi->Meshes[i]->BoneBindings[j].OBBMin[1] = miny;
			gfi->Meshes[i]->BoneBindings[j].OBBMin[2] = minz;

			gfi->Meshes[i]->BoneBindings[j].OBBMax[0] = maxx;
			gfi->Meshes[i]->BoneBindings[j].OBBMax[1] = maxy;
			gfi->Meshes[i]->BoneBindings[j].OBBMax[2] = maxz;
		}

		delete vData_;
		#pragma endregion


		//material id
		int32 materialID = matIndices[gfi->Meshes[i]->Name];
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(materialID), 4);



		//accessory index
		int32 accessoryIndex = i;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(accessoryIndex), 4);

		//max bones (?)
		int32 maxBones = 1;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(maxBones), 4);

		//rigid bone index
		granny_mesh_binding* gmb = GrannyNewMeshBinding(gfi->Meshes[i], gfi->Skeletons[0], gfi->Skeletons[0]);
		const granny_int32* mb = GrannyGetMeshBindingToBoneIndices(gmb);
		int32 rigidBoneIndex = 0; //rigid meshes dont seem to follow bones correctly.
		if (!isRigid[i]) rigidBoneIndex = 0xFFFFFF7F;
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
		if (isRigid[i]) vertexSize = 24;
		if (!isRigid[i]) vertexSize = 32;

		//vertex buffer offset (in bytes)
		int32 vertexBufferOffset = 0;
		for (int j = 0; j < i; j++)
		{
			int currentIterationVertexSize = 24;
			if (!isRigid[j]) currentIterationVertexSize = 32;
			vertexBufferOffset += gfi->Meshes[j]->PrimaryVertexData->VertexCount * currentIterationVertexSize;
		}
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(vertexBufferOffset), 4);

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

		if (verbose) {
			std::string name(gfi->Meshes[i]->Name);
			meshNames.push_back(name);
			if (isRigid[i]) meshIsRigid.push_back(sYes);
			else meshIsRigid.push_back(sNo);
			meshVertexCount.push_back(GrannyGetMeshVertexCount(gfi->Meshes[i]));
			meshIndexCount.push_back(GrannyGetMeshIndexCount(gfi->Meshes[i]));
		}
	}
	//packing order
	for (int i = 0; i < numMeshes; i++)
	{
		if (isRigid[i])
		{
			byte* packOrder = new byte[8]{ 0x50, 0x4E, 0x54, 0x30, 0x00, 0x00, 0x00, 0x00 }; //PNT0
			BVec::AddToVectorFromPtr(newCachedData, packOrder, 8);
			delete packOrder;
		}
		else
		{
			byte* packOrder = new byte[8]{ 0x50, 0x4E, 0x53, 0x54, 0x30, 0x00, 0x00, 0x00 }; //PNST0
			BVec::AddToVectorFromPtr(newCachedData, packOrder, 8);
			delete packOrder;
		}
	}


	#pragma endregion
	#pragma region bones

	int currentSize = newCachedData.size();
	for (int i = 0; i < numBones; i++)
	{
		//name offset
		int64 nameOffset = currentSize + (gfi->Skeletons[0]->BoneCount * 80) + (32 * i);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI64(nameOffset), 8);

		//world matrix
		//std::cout << '\n' << currentSize << '\n' << newCachedData.size() << '\n';
		BVec::AddToVectorFromPtr(newCachedData, (byte*)gfi->Skeletons[0]->Bones[i].InverseWorld4x4, 64);

		//parent index
		//std::cout << gfi->Skeletons[0]->Bones[i].ParentIndex << '\n' << newCachedData.size();
		int64 parentIndex = gfi->Skeletons[0]->Bones[i].ParentIndex;
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI64(parentIndex), 8);
	}

	//write bone names (TODO: NEEDS REWORK)
	for (int i = 0; i < gfi->Skeletons[0]->BoneCount; i++)
	{
		int32 nameLen = strlen(gfi->Skeletons[0]->Bones[i].Name);
		BVec::AddToVectorFromPtr(newCachedData, (byte*)gfi->Skeletons[0]->Bones[i].Name, nameLen);
		for (int j = nameLen; j < 32; j++)
		{
			int16 k = 0;
			BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI16(k), 1);
		}
		boneNames.push_back(gfi->Skeletons[0]->Bones[i].Name);
	}

	#pragma endregion
	#pragma region accessories
	//accessories
	int32 currentOffset = newCachedData.size();
	for (int i = 0; i < numAccessories; i++)
	{
		int32 firstBone = i;
		int32 accessoryBoneCount = 1;
		if (!isRigid[i]) numBones = gfi->Skeletons[0]->BoneCount;
		int32 sectionIndicesCount = 1;
		int32 sectionIndicesOffset = currentOffset + (gfi->MeshCount * 24) + (i * 4); //what?

		//first bone
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(firstBone), 4);
		//number of bones
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(accessoryBoneCount), 4);
		//section indices count
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(sectionIndicesCount), 4);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(NULL), 4);
		//section indices offset
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(sectionIndicesOffset), 4);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(NULL), 4);
	}
	for (int i = 0; i < numAccessories; i++)
	{
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(i), 4);
	}

	//valid accessories
	for (int i = 0; i < numAccessories; i++)
	{
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesI32(i), 4);
	}
	#pragma endregion
	#pragma region bonebounds
	//low
	for (int i = 0; i < numBones; i++)
	{
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(-100.f), 4);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(-100.f), 4);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(-100.f), 4);
	}
	//high
	for (int i = 0; i < numBones; i++)
	{
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(100.f), 4);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(100.f), 4);
		BVec::AddToVectorFromPtr(newCachedData, BitConverter::GetBytesF32(100.f), 4);
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

	int vdOffset = 0;
	for (int i = 0; i < gfi->MeshCount; i++)
	{
		if (isRigid[i]) //PNT0
		{
			int vDataSize = sizeof(PNT0) * GrannyGetMeshVertexCount(gfi->Meshes[i]);
			void* vData = malloc(vDataSize);
			GrannyCopyMeshVertices(gfi->Meshes[i], PNT0_def, vData);

			BVec::AddToVectorFromPtr(newVertexData, (byte*)vData, vDataSize);
			vdOffset += vDataSize;
		}
		else //PNST0
		{
			granny_mesh_binding* gmb = GrannyNewMeshBinding(gfi->Meshes[i], gfi->Skeletons[0], gfi->Skeletons[0]);
			const granny_int32* mb = GrannyGetMeshBindingToBoneIndices(gmb);

			int vDataSize = sizeof(PNST0) * GrannyGetMeshVertexCount(gfi->Meshes[i]);
			void* vData = malloc(vDataSize);
			GrannyCopyMeshVertices(gfi->Meshes[i], PNST0_def, vData);

			BVec::AddToVectorFromPtr(newVertexData, (byte*)vData, vDataSize);

			for (int j = 0; j < GrannyGetMeshVertexCount(gfi->Meshes[i]); j++)
			{
				for (int k = 0; k < 4; k++)
				{
					int xx = 0;
					if (mb != NULL) xx = mb[(int)newVertexData[vdOffset + (j * 32) + 20 + k]] + 1;
					newVertexData[vdOffset + (j * 32) + 20 + k] = xx;
				}
			}
			vdOffset += vDataSize;
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

	string matPath = materialInfoPath + ".matdata";

	std::vector<Node> nodes;
	std::vector<NameValue> names;
	std::vector<byte> data;

	string s = ParseXML(matPath, nodes, names, data);
	if (s != "OK") {
		f.status = "error creating material chunk from xml: " + s;
		return -2;
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

	string p = string(std::filesystem::temp_directory_path().string() + "tmpgrx").c_str();

	GrannyConvertFileInfoToRaw(gfi, p.c_str());
	FILE* tmpgrx = fopen(p.c_str(), "rb");
	if (tmpgrx == NULL) { f.status = "tmpgrx file was null."; return -1; }
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
		delete buffer;
		remove(p.c_str());
	}

	#pragma endregion

	#pragma region verbose output
	if (verbose) {
		std::printf("\n____________________________________________________________________________________________\n");
		std::printf("| Mesh Name                                        | Mat Index | Vertex Count | Index Count |\n");
		for (int i = 0; i < numMeshes; i++)
		{
			std::string s;
			if (meshNames[i].size() > 48) s = meshNames[i].substr(0, 45) + "..."; else s = meshNames[i];
			std::printf("| %-*s ", 48, s.c_str());
			std::printf("| %-*s ", 9, std::to_string(matIndices[meshNames[i]]).c_str());
			std::printf("| %-*i ", 12, meshVertexCount[i]);
			std::printf("| %-*i |", 11, meshIndexCount[i]);
			std::printf("\n");
		}

		std::printf("\n____________________________________________________________________________________________\n");
		std::printf("| Bone Name                                   | Bone Name                                  |\n");
		bool _first = true;
		for (int i = 0; i < numBones; i++)
		{
			if (_first) {
				std::string s;
				if (boneNames[i].size() > 43) s = boneNames[i].substr(0, 40) + "..."; else s = boneNames[i];
				std::printf("| %-*s |", 43, s.c_str());
			}

			else {
				std::string s;
				if (boneNames[i].size() > 42) s = boneNames[i].substr(0, 39) + "..."; else s = boneNames[i];
				std::printf(" %-*s |\n", 42, s.c_str());
			}

			_first = !_first;
		}
		if (!_first) std::printf("                                            |\n");

		std::printf("\n");
	}
	#pragma endregion

	delete isRigid;
	f.status = "OK";

	f.Save(outPath);
}
