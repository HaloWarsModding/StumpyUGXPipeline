#pragma once
#include <vector>
#include <map>
#include <iostream>
#include "defs.h"
#include <Windows.h>
#include "tinyxml2.h"
using namespace tinyxml2;
template<class T> using vector = std::vector<T>;

class Node
{
public:
	uint16 childNodeIndex;
	byte childNodeCount;
	uint16 nameValueIndex;
	byte nameValueCount;
	uint16 parentIndex;
};
class NameValue
{
public:
	uint32 value;
	uint16 nameOffset;
	uint16 flags;
};

float* GetUVWValues(char* uvw)
{
	float* f = new float[3];
	char** c = new char*[3];
	string s(uvw);
	
	int index1 = s.find(',');
	uvw[index1] = 0x00;
	c[0] = &uvw[0];
	int index2 = s.find(',', index1 + 1);
	uvw[index2] = 0x00;
	c[1] = &uvw[index1 + 1];
	int index3 = s.find(',', index2 + 1);
	uvw[index3] = 0x00;
	c[2] = &uvw[index2 + 1];

	f[0] = std::stof(c[0]);
	f[1] = std::stof(c[1]);
	f[2] = std::stof(c[2]);

	return f;
}

std::vector<XMLElement*> GetChildren(XMLElement* root)
{
	std::vector<XMLElement*> v;
	XMLElement* firstE = root->FirstChildElement();
	if (firstE == NULL) { throw("error at GetChildren(XMLElement*)."); return v; }

	XMLElement* previousE = firstE;
	v.push_back(previousE);
	while (true)
	{
		XMLElement* sibling = v[v.size() - 1]->NextSiblingElement();
		if (sibling == NULL) break;
		v.push_back(sibling);
	}
	return v;
}

std::map<std::string, int32> strLoc;
void InitStrLoc()
{
	const char nameData[298] = {
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
	
	int16 strIndex = 0;
	for (int16 i = 0; i < 298; i++)
	{
		if (nameData[i] == '\0' && i < 297)
		{
			strLoc.insert(std::pair<std::string, int32>((&nameData[strIndex]), strIndex));
			strIndex = i + 1;
		}
	}
}

void WriteMapParent(XMLElement* e, std::string name, int parentIndex, int& outIndex, std::vector<byte>& data, std::vector<Node>& nodes, std::vector<NameValue>& names);
void WriteMap(XMLElement* e, int namesSize, int mapIndex, std::vector<byte>& data, std::vector<Node>& nodes, std::vector<NameValue>& names);
void WriteAttrib(XMLElement* e, std::string name, std::vector<Node>& nodes, std::vector<NameValue>& names, int dataType, int parentIndex);

#define TBD 0x3030
string ParseXML(string path, vector<Node>& nodes, vector<NameValue>& names, vector<byte>& data)
{
	try {
		InitStrLoc();
		tinyxml2::XMLDocument doc;
		doc.LoadFile(path.c_str());

		vector<XMLElement*> materials = GetChildren(doc.FirstChildElement("Materials"));
		if (materials.size() == 0) return "materials.size() returned null";
		data.push_back('d');
		data.push_back('e');
		data.push_back('f');
		data.push_back('a');
		data.push_back('u');
		data.push_back('l');
		data.push_back('t');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');
		data.push_back('\0');

		Node rootNode;
		rootNode.childNodeIndex = 1;
		rootNode.childNodeCount = materials.size();
		rootNode.nameValueIndex = 0;
		rootNode.nameValueCount = 1;
		rootNode.parentIndex = 65535;
		NameValue rootName;
		rootName.value = 0;
		rootName.nameOffset = strLoc["Materials"];
		rootName.flags = 256;
		nodes.push_back(rootNode);
		names.push_back(rootName);

		//material roots
		for (int i = 0; i < materials.size(); i++)
		{
			Node n;
			n.childNodeCount = 2;
			n.nameValueIndex = names.size();
			n.nameValueCount = 3;
			n.parentIndex = 0;
			nodes.push_back(n);

			NameValue mnvTitle;
			mnvTitle.value = 0;
			mnvTitle.nameOffset = strLoc["Material"];
			mnvTitle.flags = 0;
			NameValue mnvName;
			mnvName.value = 0;
			mnvName.nameOffset = strLoc["Name"];
			mnvName.flags = 4113;
			NameValue mnvVer;
			mnvVer.value = 4;
			mnvVer.nameOffset = strLoc["Ver"];
			mnvVer.flags = 2379;
			names.push_back(mnvTitle);
			names.push_back(mnvName);
			names.push_back(mnvVer);
		}

		for (int i = 0; i < materials.size(); i++)
		{
			nodes[i + 1].childNodeIndex = nodes.size();

			//NameValues
			XMLElement* attribValues = materials[i]->FirstChildElement("NameValues");
			if (!attribValues) return "attribValues was null.";
			Node attribValuesNode;
			attribValuesNode.childNodeIndex = TBD;
			attribValuesNode.childNodeCount = 12;
			attribValuesNode.nameValueIndex = names.size();
			attribValuesNode.nameValueCount = 1;
			attribValuesNode.parentIndex = i + 1;

			int attribValuesIndex = nodes.size();
			nodes.push_back(attribValuesNode);

			NameValue attribName;
			attribName.value = 0;
			attribName.nameOffset = strLoc["NameValues"];
			attribName.flags = 256;

			names.push_back(attribName);

			//Maps
			XMLElement* mapsValues = materials[i]->FirstChildElement("Maps");
			if (!mapsValues) return "mapsValues was null.";
			Node mapValuesNode;
			mapValuesNode.childNodeIndex = nodes.size() + 1;
			mapValuesNode.childNodeCount = 13;
			mapValuesNode.nameValueIndex = names.size();
			mapValuesNode.nameValueCount = 1;
			mapValuesNode.parentIndex = i + 1;

			int mapsValuesIndex = nodes.size();
			nodes.push_back(mapValuesNode);

			NameValue mapsName;
			mapsName.value = 0;
			mapsName.nameOffset = strLoc["Maps"];
			mapsName.flags = 256;

			names.push_back(mapsName);

#pragma region maps

			int diffuseIndex;
			WriteMapParent(mapsValues, "diffuse", mapsValuesIndex, diffuseIndex, data, nodes, names);
			int normalIndex;
			WriteMapParent(mapsValues, "normal", mapsValuesIndex, normalIndex, data, nodes, names);
			int glossIndex;
			WriteMapParent(mapsValues, "gloss", mapsValuesIndex, glossIndex, data, nodes, names);
			int opacityIndex;
			WriteMapParent(mapsValues, "opacity", mapsValuesIndex, opacityIndex, data, nodes, names);
			int xformIndex;
			WriteMapParent(mapsValues, "xform", mapsValuesIndex, xformIndex, data, nodes, names);
			int emissiveIndex;
			WriteMapParent(mapsValues, "emissive", mapsValuesIndex, emissiveIndex, data, nodes, names);
			int aoIndex;
			WriteMapParent(mapsValues, "ao", mapsValuesIndex, aoIndex, data, nodes, names);
			int envIndex;
			WriteMapParent(mapsValues, "env", mapsValuesIndex, envIndex, data, nodes, names);
			int envmaskIndex;
			WriteMapParent(mapsValues, "envmask", mapsValuesIndex, envmaskIndex, data, nodes, names);
			int emxformIndex;
			WriteMapParent(mapsValues, "emxform", mapsValuesIndex, emxformIndex, data, nodes, names);
			int distortionIndex;
			WriteMapParent(mapsValues, "distortion", mapsValuesIndex, distortionIndex, data, nodes, names);
			int highlightIndex;
			WriteMapParent(mapsValues, "highlight", mapsValuesIndex, highlightIndex, data, nodes, names);
			int modulateIndex;
			WriteMapParent(mapsValues, "modulate", mapsValuesIndex, modulateIndex, data, nodes, names);


			XMLElement* e;
			e = mapsValues->FirstChildElement("diffuse")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), diffuseIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("normal")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), normalIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("gloss")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), glossIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("opacity")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), opacityIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("xform")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), xformIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("emissive")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), emissiveIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("ao")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), aoIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("env")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), envIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("envmask")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), envmaskIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("emxform")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), emxformIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("distortion")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), distortionIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("highlight")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), highlightIndex, data, nodes, names);
			e = mapsValues->FirstChildElement("modulate")->FirstChildElement("Map");
			if (e) WriteMap(e, names.size(), modulateIndex, data, nodes, names);
#pragma endregion
#pragma region attribs

			nodes[attribValuesIndex].childNodeIndex = nodes.size();

			WriteAttrib(attribValues->FirstChildElement("SpecPower"), "SpecPower", nodes, names, 1, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("SpecColorR"), "SpecColorR", nodes, names, 1, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("SpecColorG"), "SpecColorG", nodes, names, 1, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("SpecColorB"), "SpecColorB", nodes, names, 1, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("EnvReflectivity"), "EnvReflectivity", nodes, names, 1, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("EnvSharpness"), "EnvSharpness", nodes, names, 1, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("EnvFresnel"), "EnvFresnel", nodes, names, 1, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("EnvFresnelPower"), "EnvFresnelPower", nodes, names, 1, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("AccessoryIndex"), "AccessoryIndex", nodes, names, 2, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("Flags"), "Flags", nodes, names, 2, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("BlendType"), "BlendType", nodes, names, 3, attribValuesIndex);
			WriteAttrib(attribValues->FirstChildElement("Opacity"), "Opacity", nodes, names, 3, attribValuesIndex);

#pragma endregion
		}

	}
	catch (const char* c)
	{
		return std::string(c);
	}
	return "OK";
}

void WriteMapParent(XMLElement* e, std::string name, int parentIndex, int& outIndex, std::vector<byte>& data, std::vector<Node>& nodes, std::vector<NameValue>& names)
{
	//diffuse
	XMLElement* diffuseElement = e->FirstChildElement(name.c_str());
	if (!diffuseElement) throw("element in WriteMapParent() was null.");

	NameValue diffuseUVWName;
	diffuseUVWName.value = TBD;
	diffuseUVWName.nameOffset = strLoc["UVWVel"];
	diffuseUVWName.flags = 6476;

	Node diffuseNode;
	diffuseNode.childNodeIndex = 65535;
	if (diffuseElement->NoChildren()) {
		diffuseNode.childNodeCount = 0;
		diffuseUVWName.value = 8;
	}
	else {
		diffuseNode.childNodeCount = 1;
		float* f = GetUVWValues((char*)diffuseElement->FindAttribute("UVWVel")->Value());
		if (!f) throw("floats from GetUVWValues() was null.");
		diffuseUVWName.value = data.size();
		BVec::AddToVectorFromPtr(data, BitConverter::GetBytesF32(f[0]), 4);
		BVec::AddToVectorFromPtr(data, BitConverter::GetBytesF32(f[1]), 4);
		BVec::AddToVectorFromPtr(data, BitConverter::GetBytesF32(f[2]), 4);
	}
	diffuseNode.nameValueIndex = names.size();
	diffuseNode.nameValueCount = 2;
	diffuseNode.parentIndex = parentIndex;

	NameValue diffuseName;
	diffuseName.value = 0;
	diffuseName.nameOffset = strLoc[name];
	diffuseName.flags = 0;

	outIndex = nodes.size();

	nodes.push_back(diffuseNode);
	names.push_back(diffuseName);
	names.push_back(diffuseUVWName);
#pragma endregion
}
void WriteMap(XMLElement* e, int namesSize, int mapIndex, std::vector<byte>& data, std::vector<Node>& nodes, std::vector<NameValue>& names)
{
	Node mapNode;
	mapNode.childNodeIndex = 65535;
	mapNode.childNodeCount = 0;
	mapNode.nameValueIndex = namesSize;
	mapNode.nameValueCount = 4;

	NameValue mapName;
	mapName.value = 0;
	mapName.nameOffset = strLoc["Map"];
	mapName.flags = 0;

	NameValue mapNameName;
	mapNameName.nameOffset = strLoc["Name"];
	mapNameName.flags = 32785;

	NameValue mapChannel;
	mapChannel.nameOffset = strLoc["Channel"];
	mapChannel.flags = 1066;

	NameValue mapFlags;
	mapFlags.nameOffset = strLoc["Flags"];
	mapFlags.flags = 1323;

	mapNode.parentIndex = mapIndex;
	nodes[mapIndex].childNodeIndex = nodes.size();

	if (!e->FindAttribute("Name")->Value()) { throw("attribute \'Name\' was null in WriteMap()."); return; }

	mapNameName.value = data.size();
	int sLen = std::strlen(e->FindAttribute("Name")->Value());
	for (int z = 0; z < sLen; z++) data.push_back(e->FindAttribute("Name")->Value()[z]);
	data.push_back(0x00);


	if (!e->FindAttribute("Channel")->Value()) { throw("attribute \'Channel\' was null in WriteMap()."); return; };
	mapChannel.value = e->FindAttribute("Channel")->IntValue();

	if (!e->FindAttribute("Flags")->Value()) { throw("attribute \'Flags\' was null in WriteMap()."); return; };
	mapFlags.value = e->FindAttribute("Flags")->IntValue();

	nodes.push_back(mapNode);
	names.push_back(mapName);
	names.push_back(mapNameName);
	names.push_back(mapChannel);
	names.push_back(mapFlags);
}
void WriteAttrib(XMLElement* e, std::string name, std::vector<Node>& nodes, std::vector<NameValue>& names, int dataType, int parentIndex)
	{
		Node attribNode;
		attribNode.childNodeIndex = 65535;
		attribNode.childNodeCount = 0;
		attribNode.nameValueIndex = names.size();
		attribNode.nameValueCount = 1;
		attribNode.parentIndex = parentIndex;

		NameValue attribName;

		if (dataType == 1) {
			attribName.value = BitConverter::ToInt32(BitConverter::GetBytesF32(e->FloatText()), BitConverter::BigE);
			attribName.flags = 2382;
		}
		if (dataType == 2) {
			attribName.value = e->IntText();
			attribName.flags = 2379;
		}
		if (dataType == 3) {
			attribName.value = e->Value()[0];
			attribName.flags = 779;
		}

		if (name != "Opacity") attribName.nameOffset = strLoc[name];
		if (name == "Opacity") attribName.nameOffset = 290;

		nodes.push_back(attribNode);
		names.push_back(attribName);
}