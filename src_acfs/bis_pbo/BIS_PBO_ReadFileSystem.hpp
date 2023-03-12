/*
 * Copyright (c) 2019-2023 Amir Czwink (amir130@hotmail.de)
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
#include "BIS_PBO_Decompressor.hpp"
#include "PBO.hpp"

struct PBOFileHeader : public ContainerFileHeader
{
	bool isCompressed;
};

class BIS_PBO_ReadFileSystem : public CustomArchiveFileSystem<PBOFileHeader>
{
public:
	//Constructor
	inline BIS_PBO_ReadFileSystem(SeekableInputStream* containerInputStream) : CustomArchiveFileSystem(*containerInputStream), containerInputStream(containerInputStream)
	{
		this->ReadFileHeaders();
	}

private:
	//State
	UniquePointer<SeekableInputStream> containerInputStream;

	//Methods
	void AddTypedFilters(ChainedInputStream &chainedInputStream, const PBOFileHeader &header, bool verify) const override
	{
		if(header.isCompressed)
		{
			chainedInputStream.Add(new BufferedInputStream(chainedInputStream.GetEnd())); //add a buffer for performance
			chainedInputStream.Add(new BIS_PBO_Decompressor(chainedInputStream.GetEnd(), header.size, verify));
		}
	}

	void ReadFileHeaders()
	{
		BufferedInputStream bufferedInputStream(*this->containerInputStream);
		DataReader dataReader(false, bufferedInputStream);
		TextReader textReader(bufferedInputStream, TextCodecType::Latin1);

		//read in all pbo entries first of all
		PboHeaderEntry headerEntry;
		DynamicArray<PboHeaderEntry> fileEntries;
		while(true)
		{
			String filePath = textReader.ReadZeroTerminatedString();;
			headerEntry.entryType = static_cast<PboEntryType>(dataReader.ReadUInt32());
			headerEntry.uncompressedSize = dataReader.ReadUInt32();
			dataReader.Skip(4); //zero
			headerEntry.timeStamp = dataReader.ReadUInt32();
			headerEntry.blockSize = dataReader.ReadUInt32();

			if(headerEntry.entryType == PboEntryType::Version)
			{
				//skip version fields
				String string;
				do
				{
					string = textReader.ReadZeroTerminatedString();
				}
				while(!string.IsEmpty());
				continue;
			}
			if(filePath.IsEmpty())
				break;

			filePath = filePath.Replace(u8"\\", u8"/"); //convert from windows line endings
			filePath = u8"/" + filePath;
			headerEntry.filePath = filePath;

			fileEntries.Push(headerEntry);
		}

		//TODO: elite and arma pbos have a zero byte followed by a checksum at the end of the archive
		//TODO: for elite it is a 4 byte byte sum and for arma it is a sha1 hash

		//add files
		uint64 offset = this->containerInputStream->QueryCurrentOffset() - bufferedInputStream.GetBytesAvailable();
		for(const PboHeaderEntry& fileEntry : fileEntries)
		{
			PBOFileHeader fileHeader;

			fileHeader.type = FileType::File;
			fileHeader.storedSize = fileEntry.blockSize;
			fileHeader.offset = offset;
			fileHeader.size = fileEntry.uncompressedSize;
			fileHeader.isCompressed = (fileEntry.entryType == PboEntryType::Compressed);

			if((fileEntry.entryType == PboEntryType::Uncompressed) && (fileEntry.uncompressedSize == 0))
				fileHeader.size = fileEntry.blockSize;

			this->AddSourceFile(fileEntry.filePath, Move(fileHeader));

			offset += fileEntry.blockSize;
		}
	}
};