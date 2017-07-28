//============== Copyright lololol, All rights reserved. ==============//
//
// Purpose:		main
//
// $NoKeywords: $
//=====================================================================//

#include <iostream>

#include "DDFile.hpp"

int main(int argc, const char* argv[])
{
	if (argc < 2 || argc > 3)
	{
		std::cout <<
	         "Invalid arguments.\n"
		       "	Usage: " << argv[0] << " <filename>\n"
		       "	       " << argv[0] << " <filename> <path/to/output/directory/>\n"
		       "\nExample: " << argv[0] << " audio somefolder/extractedFiles/\n";
		return 0;
	}

	DDFile arc;
	arc.load(argv[1]);
	arc.extract(argc == 3 ? argv[2] : "");

	return 0;
}
