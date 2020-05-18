#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include "granny.h"
#include "ECF/UGX/UGXFile.h"

int main(int argc, char** argv)
{
	if (argc < 2 || argc > 3 || std::string(argv[1]) == "-h")
	{
		std::cout <<
		 "\n===============================================================================\n" <<
			"This program requires two flags to function.\n" <<
			"Valid flags for flag 0: \n" <<
			"-ugx => Convert this gr2 into a ugx (Halo Wars mesh)\n" <<
			"-uax => Convert this gr2 into a uax (Halo Wars animation)\n" <<
			"Valid flags for flag 1: \n" <<
			"-path => Specifies a path to a .gr2 (Granny Runtime) file for conversion to the previously selected format.\n\n" <<
			"Valid inputs would look like: \n" <<
			"stumpygr2ugx -ugx -C:\\folder1\\mesh.gr2\n" <<
			"stumpygr2ugx -uax -C:\\folder2\\anim.gr2\n\n" <<
			"Output file will be placed in the exact path as the\ninput with the correct extension for the selected output.\n"
			"===============================================================================\n";
		return -1;
	}

	if (std::string(argv[1]) == "-ugx")
	{
		UGXFile f = UGXFile::FromGR2(argv[2]);
		if (f.status == "OK")
		{
			string s(argv[2]);
			s = s.substr(0, s.find(".gr2"));

			f.Save(s + ".ugx");
			std::cout << "UGX Saved to " + s + ".ugx" << std::endl;
		}
		else
		{
			std::cout << "Error: " << f.status << "\nUGX could not be saved." << std::endl;
		}
	}

	if (std::string(argv[1]) == "-uax")
	{
		CreateUAXs(argv[2]);
	}
}