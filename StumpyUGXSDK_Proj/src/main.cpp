#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include "granny.h"
#include "ECF/UGX/UGXFile.h"

int main(int a, char** b)
{
	if (a < 2) goto START; //no params -- bypass debug code.
	if (string(b[1]) == "-debug") //debug override.
	{
		std::cout << b[2];
		UGXFile f = UGXFile::FromGR2(b[2], true);
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

START:

	std::string path;
	std::string mode;
	std::string materialPath;


	std::cout << "Please enter a path to a .gr2 file.\n>>";
	std::getline(std::cin, path);
	std::cout << "Please select what you want to export this file as: (a = animation | m = mesh)\n>>";
	std::getline(std::cin, mode);


	if (std::string(mode) == "-h")
	{
		//for (int i = 0; i < 12; i++)std::cout << helpString[i];
	}

	if (std::string(mode) == "m")
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

	if (std::string(mode) == "a")
	{
		CreateUAXs(path);
	}

	std::cout << '\n';
	goto START;
}