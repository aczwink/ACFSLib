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
#include "BIS_PBO_CompressedFile.hpp"

class BIS_PBO_FileSystem : public ContainerFileSystem
{
public:
	//Constructor
	inline BIS_PBO_FileSystem(const Format *format, const Path &fileSystemPath) : ContainerFileSystem(fileSystemPath)
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
		uint64 offset = this->containerInputStream->GetCurrentOffset() - bufferedInputStream.GetBytesAvailable();
		for(const PboHeaderEntry& fileEntry : fileEntries)
		{
			ContainerFileHeader fileHeader;

			fileHeader.compressedSize = fileEntry.blockSize;
			fileHeader.offset = offset;
			fileHeader.uncompressedSize = fileEntry.uncompressedSize;

			if((fileEntry.entryType == PboEntryType::Uncompressed) && (fileEntry.uncompressedSize == 0))
				fileHeader.uncompressedSize = fileEntry.blockSize;

			if(fileEntry.entryType == PboEntryType::Compressed)
				this->AddSourceFile(fileEntry.filePath, new BIS_PBO_CompressedFile(fileHeader, this));
			else
			this->AddSourceFile(fileEntry.filePath, fileHeader);

			offset += fileEntry.blockSize;
		}
	}
};