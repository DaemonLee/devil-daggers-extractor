//============== Copyright lololol, All rights reserved. ==============//
//
// Purpose:		<DD root>/res/* file extractor
//
// $NoKeywords: $ddf
//=====================================================================//

#include "DDFile.hpp"

#include <fstream>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <cstddef>

#ifdef _MSC_VER
	#include <direct.h>
#endif

// recursive directory creation function (c) Nico Golde @ http://nion.modprobe.de/blog/archives/357-Recursive-directory-creation.html
static void _mkdir_tree(const char *dir)
{
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	if (tmp[len-1] == '/')
		tmp[len-1] = 0;
	for (p=tmp + 1; *p; p++)
		if (*p == '/')
		{
			*p = 0;
			#ifdef _MSC_VER
				_mkdir(tmp);
			#else
				mkdir(tmp, 0644);
			#endif
			*p = '/';
		}
		#ifdef _MSC_VER
			_mkdir(tmp);
		#else
			mkdir(tmp, 0644);
		#endif
}

DDFile::DDFile()
{
	m_header = NULL;
	m_fileBuffer = NULL;
	m_bLoaded = false;
}

DDFile::~DDFile()
{
	clear();
}

void DDFile::clear()
{
	m_bLoaded = false;
	m_header = NULL;

	if (m_fileBuffer != NULL)
	{
		delete[] m_fileBuffer;
		m_fileBuffer = NULL;
	}
}

bool DDFile::load(std::string filename)
{
	if (m_bLoaded)
		clear();

	// check if the file exists
	/*
	FILE *file = NULL;
	fopen_s(&file, filename.c_str(), "rb");
	*/
	FILE *file = fopen(filename.c_str(), "rb");
	if (!file)
	{
		std::cerr << "ERROR: " << filename << " does not exist!\n";
		return false;
	}

	// get file size
	fseek(file, 0, SEEK_END);
	u32 filesize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// filesize error checking
	if (filesize < (sizeof(DDHeader)))
	{
		std::cerr << "ERROR: Invalid filesize (" << filesize << ")\n";
		fclose(file);
		return false;
	}

	// create buffer
	m_fileBuffer = (u32*)(new u8[filesize]);

	// read entire file, then close it
	fread(m_fileBuffer, sizeof(u8), filesize, file);
	fclose(file);

	// now get everything we want
	m_header = (DDHeader*)m_fileBuffer;

	m_header->magicnumber[0] = endianSwapU32(m_header->magicnumber[0]);
	m_header->magicnumber[1] = endianSwapU32(m_header->magicnumber[1]);

	if (m_header->magicnumber[0] != 0x3a68783a || m_header->magicnumber[1] != 0x72673a01)
		std::cout << "WARNING: Header seems to be incorrect ([" << m_header->magicnumber[0] << "] [" << m_header->magicnumber[1] << "])!\n";
	std::cout << "header.dataoffset: " << m_header->dataoffset + sizeof(DDHeader) << "\n";

	// load all sub file headers
	u32 *dataPointer = (u32*)((u8*)m_fileBuffer + sizeof(DDHeader));
	int fileCounter = 0;
	while (true)
	{
		DDSubFileHeader subFileHeader;

		u32 *startFilePointer = (u32*)dataPointer;
		DDSubFileHeaderStart *start = (DDSubFileHeaderStart*)startFilePointer;
		subFileHeader.typeflags = start->typeflags;

		u32 *subfilename = (u32*)((u8*)dataPointer + sizeof(DDSubFileHeaderStart));
		subFileHeader.filename = (unsigned char*)subfilename;

		u32 *endFilePointer = (u32*)((u8*)subfilename + strlen((const char*)subfilename) + 1);
		DDSubFileHeaderEnd *end = (DDSubFileHeaderEnd*)(endFilePointer);
		subFileHeader.offset = end->offset;
		subFileHeader.filesize = end->filesize;
		subFileHeader.unknown2 = end->unknown2;

		// go to next file
		dataPointer = (u32*)((u8*)endFilePointer + sizeof(DDSubFileHeaderEnd));

		// check if we are done before adding it
		if (subFileHeader.typeflags == 0)
		{
			std::cout << "==============================================================================\n"
									 "This seems to be the end of the subfileheaders.\n";
			break;
		}

		// add it
		m_subFileHeaders.push_back(subFileHeader);
		std::cout << "subFile #" << fileCounter
							<< ": name = " << subFileHeader.filename
							<< ", type = " << std::hex << subFileHeader.typeflags << std::dec
							<< ", offset = "<< subFileHeader.offset
							<< ", filesize = " << subFileHeader.filesize/1024.0f/1024.0f
							<< " MB\n";
		fileCounter++;
	}

	m_bLoaded = true;
	return true;
}

bool DDFile::extract(std::string folderToExtractTo)
{
	if (!m_bLoaded)
	{
		std::cerr << "ERROR: load() a file before trying to extract it.\n";
		return false;
	}
	std::cout << "Starting extraction for " << m_subFileHeaders.size() << " subfiles:\n";

	// create folder if it doesn't exist already
	_mkdir_tree(folderToExtractTo.c_str());

	for (u32 i = 0; i<m_subFileHeaders.size(); i++)
	{
		std::string outputFileName = folderToExtractTo;
		outputFileName.append((char*)m_subFileHeaders[i].filename);

		// autodetect filetype
		if (m_subFileHeaders[i].typeflags & 0x20)
			outputFileName.append(".wav");
		std::cout << "  Writing " << outputFileName << " ...\n";
		std::ofstream output(outputFileName.c_str(), std::ios::out | std::ios::binary);
		if (output.is_open())
		{
			output.write(reinterpret_cast<char*>(&((u8*)m_fileBuffer)[m_subFileHeaders[i].offset]), m_subFileHeaders[i].filesize);
		}
		else
			std::cerr << "ERROR: Couldn't write " << outputFileName <<"!\n";
	}

	std::cout << "Done.\n";

	return true;
}
