/*
 * Copyright (c) 2018 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of ACFSLib.
 *
 * ACFSLib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ACFSLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ACFSLib.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <Std++.hpp>

/*
Descriptions:
	CompressedInt:
		byte, bit 7 indicates if negative (1) or positive (0), bit 6 if a byte follows, rest is data
		if byte follows, bit 7 indicates if a byte follows,
		shift and or to result



File Layout:
-file data
-struct FileHeaders:
	CompressedInt nFiles;
	struct FileHeader[...]:
		CompressedInt length; //length of the string to follow + nullbyte
		char filename[length]; //includes the nullbyte
		uint32 dataOffset; //offset to file data
		uint32 fileSize;
		uint32 zeroPad; //seems to be padding cause always 0
-struct Footer: //Always at the end of the file
	uint32 unknown; //=0xA3 C5 E3 9F, Engine doesn't matter about the value
	uint32 fileHeadersOffset;
	uint32 fileSize;
	uint32 unknown; //=1, Engine doesn't matter about the value
	uint32 unknown; //=0x2D 5C 86 D2, Engine doesn't matter about the value
*/

//Definitions
#define UMD_FOOTER_SIGNATURE 0x9FE3C5A3
#define UMD_FOOTER_UNKNOWN1 1
#define UMD_FOOTER_UNKNOWN2 0xD2865C2D

struct UMDFileHeader
{
	StdXX::String fileName;
	uint32 dataOffset;
	uint32 fileSize;
};

struct UMDFooter
{
	uint32 signature;
	uint32 fileHeadersOffset;
	uint32 fileSize;
	uint32 unknown2;
	uint32 unknown3;
};