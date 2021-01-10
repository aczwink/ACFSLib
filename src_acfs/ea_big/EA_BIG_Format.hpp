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

#include "EA_BIG_FileSystem.hpp"

const uint32 ea_big_signature = FOURCC(u8"BIGF");
const uint32 ea_big4_signature = FOURCC(u8"BIG4");

class EA_BIG_Format : public Format
{
public:
	RWFileSystem * CreateFileSystem(const Path &fileSystemPath) const override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
		return nullptr;
	}

	String GetId() const override
	{
		return u8"big";
	}

	String GetName() const override
	{
		return u8"Electronic Arts BIG";
	}

	float32 Matches(SeekableInputStream &inputStream) const override
	{
		DataReader dataReader(false, inputStream);

		uint32 readSignature = dataReader.ReadUInt32();
		if((readSignature == ea_big_signature) || (readSignature == ea_big4_signature))
		{
			uint64 readSize = dataReader.ReadUInt32();
			uint64 archiveSize = inputStream.QuerySize();
			if(archiveSize > readSize)
				return 1.0f - ((archiveSize - readSize) / float32(archiveSize));
			return 1.0f - ((readSize - archiveSize + 1) / float32(readSize + 1));
		}

		return 0;
	}

	RWFileSystem *OpenFileSystem(const Path &fileSystemPath, const OpenOptions& openOptions) const override
	{
		return new EA_BIG_FileSystem(this, fileSystemPath);
	}

	ReadableFileSystem *OpenFileSystemReadOnly(const Path &fileSystemPath, const OpenOptions& openOptions) const override
	{
		return this->OpenFileSystem(fileSystemPath, openOptions);
	}
};