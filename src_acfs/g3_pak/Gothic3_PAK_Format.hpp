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
#include "Gothic3_PAK_FileSystem.hpp"

class G3_PAK_Format : public FileSystemFormat
{
public:
	FileSystem * CreateFileSystem(const Path &fileSystemPath) const override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}

	String GetId() const override
	{
		return u8"pak";
	}

	String GetName() const
	{
		return u8"Gothic 3 PAK";
	}

	float32 Matches(SeekableInputStream &inputStream) const override
	{
		DataReader dataReader(false, inputStream);
		if(dataReader.ReadUInt32() != 0)
			return 0;
		if(dataReader.ReadUInt32() != FOURCC(u8"G3V0"))
			return 0;
		return 1;
	}

	FileSystem *OpenFileSystem(const Path &fileSystemPath, bool writable) const override
	{
		return new G3_PAK_FileSystem(this, fileSystemPath);
	}
};