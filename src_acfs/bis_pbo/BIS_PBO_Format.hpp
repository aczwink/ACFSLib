/*
 * Copyright (c) 2019 Amir Czwink (amir130@hotmail.de)
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
using namespace StdXX;
//Local
#include "PBO.hpp"
#include "BIS_PBO_FileSystem.hpp"

class BIS_PBO_Format : public FileSystemFormat
{
public:
	FileSystem *CreateFileSystem(const Path &fileSystemPath) const override
	{
		return new BIS_PBO_FileSystem(this, fileSystemPath);
	}

	String GetId() const override
	{
		return u8"pbo";
	}

	String GetName() const override
	{
		return u8"Bohemia Interactive Studios PBO";
	}

	float32 Matches(SeekableInputStream &inputStream) const override
	{
		BufferedInputStream bufferedInputStream(inputStream);

		//try to parse pbo headers
		bool gotLastHeader = false;
		while(!gotLastHeader)
		{
			if(!this->ReadNextPboHeader(bufferedInputStream, gotLastHeader))
				return 0;
		}

		return 1;
	}

private:
	//Methods
	bool ReadNextPboHeader(InputStream& inputStream, bool& last) const
	{
		DataReader dataReader(false, inputStream);
		TextReader textReader(inputStream, TextCodecType::Latin1);

		//read entry
		PboHeaderEntry pboEntry;
		pboEntry.filePath = textReader.ReadZeroTerminatedString();
		uint32 entryType = dataReader.ReadUInt32();
		pboEntry.uncompressedSize = dataReader.ReadUInt32();
		uint32 zero = dataReader.ReadUInt32();
		pboEntry.timeStamp = dataReader.ReadUInt32();
		pboEntry.blockSize = dataReader.ReadUInt32();

		//validate entry
		if(zero != 0)
			return false;

		switch(entryType)
		{
			case (uint32)PboEntryType::Uncompressed:
			case (uint32)PboEntryType::Compressed:
			{
				if(pboEntry.filePath.IsEmpty())
				{
					last = true;
				}
			}
			break;
			case (uint32)PboEntryType::Version:
			{
				ASSERT(pboEntry.filePath.IsEmpty(), u8"RERPORT THIS PLEASE!");
				String string;
				do
				{
					string = textReader.ReadZeroTerminatedString();
				}
				while(!string.IsEmpty());
			}
			break;
			default:
				return false;
		}

		return true;
	}
};