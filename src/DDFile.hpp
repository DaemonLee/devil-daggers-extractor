//============== Copyright lololol, All rights reserved. ==============//
//
// Purpose:		<DD root>/res/* file extractor
//
// $NoKeywords: $ddf
//=====================================================================//

#ifndef DDFILE_HPP
#define DDFILE_HPP

#include <string>
#include <vector>


//************//
//  Typedefs  //
//************//

typedef unsigned char	u8;
typedef unsigned short  u16;
typedef unsigned int	u32;


//*********************//
//   Helper functions  //
//*********************//

inline int endianSwapU32(int x)
{
	return (x >> 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x << 24);
}


//***********************************//
//	 The main header of the DD file  //
//***********************************//

struct DDHeader
{
	u32 magicnumber[2]; 	// should contain ':hx:rg:.'
	u32 dataoffset;			// where subfile data begins (this is a bit redundant as the offset is also stored for every subfileheader)
};


//*****************************************************************************//
//	Right after the main header, there are multiple instances of file headers  //
//*****************************************************************************//

// due to the unknown filename length, the full subfileheader structure is unusable for parsing
struct DDSubFileHeader
{
	u16 typeflags; // seems to be 0x20 for audio, and 0x10/0x11 and others for textures (dd), and 0x00 for the end of the header lump (for the first invalid fileheader)
	u8 *filename; // unknown length, determined at runtime with null byte at the end; continued after the null byte
	u32 offset;
	u32 filesize;
	u32 unknown2; // some other flags maybe?
};

struct DDSubFileHeaderStart
{
	u16 typeflags;
};
/* filename is between the two parts here */
struct DDSubFileHeaderEnd
{
	u32 offset;
	u32 filesize;
	u32 unknown2;
};


class DDFile
{
public:
	DDFile();
	~DDFile();

	bool load(std::string filename);
	bool extract(std::string folderToExtractTo);

	inline int getNumFiles() const { return m_subFileHeaders.size(); }

private:
	void clear();

	bool m_bLoaded;
	u32 *m_fileBuffer;

	DDHeader *m_header;
	std::vector<DDSubFileHeader> m_subFileHeaders;
};

#endif
