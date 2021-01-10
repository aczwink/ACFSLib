/*
 * Copyright (c) 2019,2021 Amir Czwink (amir130@hotmail.de)
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
#include <StdXX.hpp>
using namespace StdXX;

enum G3_PAK_CompressionMethod
{
	G3_PAK_COMPRESSION_METHOD_NONE = 0,
	G3_PAK_COMPRESSION_METHOD_ZLIB = 2,
};

enum G3_PAK_Flags
{
	G3_PAK_FLAGS_DIRECTORY = 0x10,
};

#define G3_PAK_MAGIC 0x100067F8
#define G3_PAK_MAGIC2 0x323C3F
#define G3_PAK_MAGIC3 0x226418
#define G3_PAK_MAGIC4 0x2A6418

class G3_PAK_FileSystem : public ContainerFileSystem
{
public:
	//Constructor
	G3_PAK_FileSystem(const Format *format, const Path &fileSystemPath) : ContainerFileSystem(fileSystemPath)
	{
		if(!this->containerInputStream.IsNull())
			this->ReadFileHeaders();
	}

	//Methods
	void Flush() override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}

private:
	//Methods
	void ReadFileEntry(InputStream& inputStream)
	{
		DataReader dataReader(false, inputStream);

		dataReader.Skip(3 * 8);
		uint64 unknown4 = dataReader.ReadUInt64();
		ASSERT(unknown4 == 0, u8"REPORT THIS PLEASE!");

		uint32 flags = dataReader.ReadUInt32(); //these are windows file attributes
		const uint32 knownMask = 0x40000 //???
				| 0x20000 //???
				| 0x2000 //???
				| 0x800 //winapi: FILE_ATTRIBUTE_COMPRESSED
				| 0x20 //winapi: FILE_ATTRIBUTE_ARCHIVE
		        | G3_PAK_FLAGS_DIRECTORY
				| 0x4 //winapi: FILE_ATTRIBUTE_SYSTEM
				| 0x2 //winapi: FILE_ATTRIBUTE_HIDDEN
				| 0x1 //winapi: FILE_ATTRIBUTE_READONLY
		;
		ASSERT((flags & (~knownMask)) == 0, u8"UNKNOWN FLAGS");
		if(flags & G3_PAK_FLAGS_DIRECTORY)
		{
			Path dirPath = this->ReadFilePath(inputStream);
			uint32 nSubDirs = dataReader.ReadUInt32();
			for(uint32 i = 0; i < nSubDirs; i++)
				this->ReadFileEntry(inputStream);

			uint32 nFiles = dataReader.ReadUInt32();
			for(uint32 i = 0; i < nFiles; i++)
				this->ReadFileEntry(inputStream);
		}
		else
		{
			ContainerFileHeader fileHeader;
			fileHeader.offset = dataReader.ReadUInt64();
			fileHeader.compressedSize = dataReader.ReadUInt64();
			fileHeader.uncompressedSize = dataReader.ReadUInt64();

			uint32 unknown = dataReader.ReadUInt32();
			ASSERT(unknown == 0, u8"REPORT THIS PLEASE!");

			uint32 compressionMethod = dataReader.ReadUInt32();
			switch(compressionMethod)
			{
				case G3_PAK_COMPRESSION_METHOD_NONE:
					break;
				case G3_PAK_COMPRESSION_METHOD_ZLIB:
					fileHeader.compression = CompressionStreamFormatType::zlib;
					break;
				default:
					NOT_IMPLEMENTED_ERROR; //TODO: implement me
			}

			Path filePath = this->ReadFilePath(inputStream);
			this->ReadFilePath(inputStream);
			//this->AddSourceFile(..., fileHeader);

			this->AddSourceFile(String(u8"/") + filePath.String(), fileHeader);
		}
	}

	void ReadFileHeaders()
	{
		DataReader dataReader(false, *this->containerInputStream);
		dataReader.Skip(8);

		uint32 version = dataReader.ReadUInt32();
		ASSERT(version == 0, u8"REPORT THIS PLEASE!");

		//unknown
		uint32 unknown = dataReader.ReadUInt32();
		ASSERT(unknown == 0, u8"REPORT THIS PLEASE!");

		//compression
		uint32 compressionMethod = dataReader.ReadUInt32();
		ASSERT(compressionMethod == 1, u8"REPORT THIS PLEASE!");

		//? could be some kind of size ?
		uint32 unknown2 = dataReader.ReadUInt32();
		ASSERT((unknown2 == G3_PAK_MAGIC) or (unknown2 == G3_PAK_MAGIC2) or (unknown2 == G3_PAK_MAGIC3) or (unknown2 == G3_PAK_MAGIC4), u8"REPORT THIS PLEASE!");

		//offsets
		uint64 fileHeadersOffset = dataReader.ReadUInt64();
		uint64 fileHeadersOffsetAgain = dataReader.ReadUInt64(); //???
		ASSERT(fileHeadersOffset == fileHeadersOffsetAgain, u8"REPORT THIS PLEASE!");
		uint64 unknown3 = dataReader.ReadUInt64(); //points to end -4 bytes

		//check unknown3
		this->containerInputStream->SeekTo(unknown3);
		ASSERT(this->containerInputStream->QueryRemainingBytes() == 4, u8"REPORT THIS PLEASE!");
		ASSERT(dataReader.ReadUInt32() == 0, u8"REPORT THIS PLEASE!");

		//read file headers
		this->containerInputStream->SeekTo(fileHeadersOffset);
		LimitedInputStream limitedInputStream(*this->containerInputStream, this->containerInputStream->QueryRemainingBytes() - 4);
		BufferedInputStream bufferedInputStream(limitedInputStream);
		while(!bufferedInputStream.IsAtEnd())
		{
			this->ReadFileEntry(bufferedInputStream);
		}
	}

	Path ReadFilePath(InputStream& inputStream)
	{
		DataReader dataReader(false, inputStream);

		uint32 length = dataReader.ReadUInt32();
		if(length > 0)
		{
			TextReader textReader(inputStream, TextCodecType::Latin1);
			String string = textReader.ReadString(length);

			uint8 zero = dataReader.ReadByte();
			ASSERT(zero == 0, u8"REPORT THIS PLEASE!");

			return string;
		}
		return {};
	}
};