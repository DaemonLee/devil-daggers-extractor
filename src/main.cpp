//============== Copyright lololol, All rights reserved. ==============//
//
// Purpose:		main
//
// $NoKeywords: $
//=====================================================================//

#include <stdio.h>

#include "DDFile.h"

int main(int argc, const char* argv[])
{
	if (argc < 2 || argc > 3)
	{
		printf("Invalid arguments.\n");
		printf("	Usage: ddex.exe <filename>\n");
		printf("	       ddex.exe <filename> <path/to/output/directory/>\n");
		printf("\nExample: ddex.exe audio somefolder/extractedFiles/\n");
		return 0;
	}

	DDFile arc;
	arc.load(argv[1]);
	arc.extract(argc == 3 ? argv[2] : "");

	return 0;
}
