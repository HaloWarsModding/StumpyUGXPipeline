#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include "granny.h"
#include "ECF/UGX/UGXFile.h"

int main(int a, char** b)
{
	if (string(b[1]) == "-debug")
	{
		std::cout << b[2];
		UGXFile f = UGXFile::FromGR2(b[2]);
		if (f.status == "OK")
		{
			string s(b[2]);
			s = s.substr(0, s.find(".gr2"));

			f.Save(s + ".ugx");
			std::cout << "UGX Saved to " + s + ".ugx" << std::endl;
		}
		else
		{
			std::cout << "Error: " << f.status << "\nUGX could not be saved." << std::endl;
		}
		system("pause");
		exit(117);
	}

	std::string helpString[12] = {
		"\n=======================OLD=====================================================\n",
		"This program requires two flags to function.\n",
		"Valid flags for flag 0: \n",
		"-ugx => Convert this gr2 into a ugx (Halo Wars mesh)\n",
		"-uax => Convert this gr2 into a uax (Halo Wars animation)\n",
		"Valid flags for flag 1: \n",
		"-path => Specifies a path to a .gr2 (Granny Runtime) file for conversion to the previously selected format.\n\n",
		"Valid inputs would look like: \n",
		"stumpygr2ugx -ugx -C:\\folder1\\mesh.gr2\n",
		"stumpygr2ugx -uax -C:\\folder2\\anim.gr2\n\n",
		"Output file will be placed in the exact path as the\ninput with the correct extension for the selected output.\n",
		"===============================================================================\n"
	};

START:

	std::string path;
	std::string type;
	std::cout << "Please enter a path to a .gr2 file.\n>>";
	std::getline(std::cin, path);
	std::cout << "Please select what you want to export this file as: (a = animation | m = mesh)\n>>";
	std::getline(std::cin, type);


	if (std::string(type) == "-h")
	{
		//for (int i = 0; i < 12; i++)std::cout << helpString[i];
	}

	if (std::string(type) == "m")
	{
		UGXFile f = UGXFile::FromGR2(path);
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

	if (std::string(type) == "a")
	{
		CreateUAXs(path);
	}

	std::cout << '\n';
	goto START;
}