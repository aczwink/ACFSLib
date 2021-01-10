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
//Local
#include "EDMW_SSA_CompressedFile.hpp"

class EDMW_SSA_FileSystem : public ContainerFileSystem
{
public:
	//Constructor
	inline EDMW_SSA_FileSystem(const Format *format, const Path &fileSystemPath) : ContainerFileSystem(fileSystemPath)
	{
		if (!this->containerInputStream.IsNull())
			this->ReadFileHeaders();
	}

	void Flush() override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}

private:
	//Methods
	void ReadFileHeaders()
	{
		BufferedInputStream bufferedInputStream(*this->containerInputStream);
		DataReader dataReader(false, bufferedInputStream);
		TextReader textReader(bufferedInputStream, TextCodecType::ASCII);

		bufferedInputStream.Skip(12); //skip signature and version
		uint32 fileHeadersSize = dataReader.ReadUInt32();

		for(uint32 size = 0; size < fileHeadersSize;)
		{
			uint32 pathLength = dataReader.ReadUInt32(); //apparently includes two zero bytes at the end
			ASSERT(pathLength >= 2, u8"REPORT THIS PLEASE!");

			String filePath = textReader.ReadString(pathLength - 2);
			ASSERT(dataReader.ReadByte() == 0, u8"REPORT THIS PLEASE!");
			ASSERT(dataReader.ReadByte() == 0, u8"REPORT THIS PLEASE!");

			filePath = filePath.Replace(u8"\\", u8"/"); //convert from windows line endings
			filePath = u8"/" + filePath;

			ContainerFileHeader fileHeader;

			fileHeader.offset = dataReader.ReadUInt32();
			uint32 lastByteOffset = dataReader.ReadUInt32();
			fileHeader.compressedSize = dataReader.ReadUInt32();

			ASSERT(lastByteOffset == fileHeader.offset + fileHeader.compressedSize - 1, u8"REPORT THIS PLEASE!");
			if(TestForCompression(fileHeader))
				this->AddSourceFile(filePath, new EDMW_SSA_CompressedFile(fileHeader, this));
			else
				this->AddSourceFile(filePath, fileHeader);

			size += sizeof(uint32) + pathLength + 3 * sizeof(uint32);
		}
	}

	bool TestForCompression(ContainerFileHeader& fileHeader)
	{
		const uint32 c_pk01_signature = FOURCC(u8"PK01");

		uint64 currentOffset = this->containerInputStream->GetCurrentOffset();
		this->containerInputStream->SeekTo(fileHeader.offset);

		BufferedInputStream bufferedInputStream(*this->containerInputStream);
		DataReader dataReader(false, bufferedInputStream);

		bool compressed;
		if(dataReader.ReadUInt32() == c_pk01_signature)
		{
			fileHeader.uncompressedSize = dataReader.ReadUInt32();
			compressed = true;
		}
		else
		{
			fileHeader.uncompressedSize = fileHeader.compressedSize;
			compressed = false;
		}

		this->containerInputStream->SeekTo(currentOffset);
		return compressed;
	}
};