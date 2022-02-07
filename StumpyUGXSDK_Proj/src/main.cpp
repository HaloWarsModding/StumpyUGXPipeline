#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include "granny.h"
#include "ECF/UGX/UGXFile.h"

int main(int a, char** b)
{
	string mode(b[1]);			// (-auto | -manual)

	if (mode == "-auto")
	{
		if (a < 8) { std::cout << "not enough args.\n"; return -1; } //not enough args.

		string outputType(b[2]);	// (-mesh | -anim)
		string inputGr2Path(b[3]);  // (path)
		string outputPath(b[4]);	// (path)
		string front(b[5]);			// (ori)
		string right(b[6]);			// (ori)
		string up(b[7]);			// (ori)

		granny_file* forFree;
		granny_file_info* gfi = LoadAndPreprocessGR2(forFree, inputGr2Path, front, right, up);






		if (gfi == NULL) {
			std::cout << "GR2 preprocess error.\n";
			return -111;
		}

		if (outputType == "-mesh") 
		{
			int32 fileExtIndex = inputGr2Path.rfind('.');
			string matDatPath = inputGr2Path.substr(0, fileExtIndex);
			CreateUGX(gfi, matDatPath, outputPath) << '\n';
		}

		if (outputType == "-anim")
		{
			CreateUAX(gfi, outputPath);
		}

		GrannyFreeFile(forFree);
		return 20;
	}

	//debug
	else
	{
		string inputGr2Path = "C:\\Users\\jaken\\AppData\\Local\\Temp\\tmpseju60f6\\tmp.gr2";
		string outputPath = "C:\\Users\\jaken\\AppData\\Local\\Temp\\tmpseju60f6\\tmp.ugx";

		granny_file* forFree;
		granny_file_info* gfi = LoadAndPreprocessGR2(forFree, inputGr2Path, "Z-", "X-", "Y+");
		int32 fileExtIndex = inputGr2Path.rfind('.');
		string matDatPath = inputGr2Path.substr(0, fileExtIndex);
		std::cout << CreateUGX(gfi, matDatPath, outputPath) << '\n';
		GrannyFreeFile(forFree);

	}

	/*else if (mode == "-manual") {

	START_MODE_MANUAL:

		std::string path;
		std::string mode__;
		std::string materialPath;


		std::cout << "Please enter a path to a .gr2 file.\n>>";
		std::getline(std::cin, path);
		std::cout << "Please select what you want to export this file as: (a = animation | m = mesh)\n>>";
		std::getline(std::cin, mode);


		if (std::string(mode__) == "-h")
		{
			//for (int i = 0; i < 12; i++)std::cout << helpString[i];
		}

		if (std::string(mode__) == "m")
		{
			UGXFile f = UGXFile::FromGR2(path, true);
			if (f.status == "OK")
			{
				string s(path);
				s = s.substr(0, s.find(".gr2"));

				f.Save(s + ".ugx");
				std::cout << "UGX Saved to " + s + ".ugx" << std::endl;
			}
			else
			{
				std::cout << "Error: " << f.status << "\nUGX could not be saved." << std::endl;
			}
		}

		if (std::string(mode__) == "a")
		{
			CreateUAXs(path);
		}

		std::cout << '\n';
		goto START_MODE_MANUAL;
	}*/

	//else std::cout << "bad args.\n";
}