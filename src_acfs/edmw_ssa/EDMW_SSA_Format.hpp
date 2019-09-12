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
#include "EDMW_SSA_FileSystem.hpp"

class EDMW_SSA_Format : public FileSystemFormat
{
public:
	FileSystem *CreateFileSystem(const Path &fileSystemPath) const override
	{
		return new EDMW_SSA_FileSystem(this, fileSystemPath);
	}

	String GetId() const override
	{
		return u8"ssa";
	}

	String GetName() const override
	{
		return u8"Empires Dawn of the Modern World SSA";
	}

	float32 Matches(SeekableInputStream &inputStream) const override
	{
		const uint32 c_ssa_signature = FOURCC(u8"rass");
		const uint32 c_ssa_versionMajor = 1;
		const uint32 c_ssa_versionMinor = 0;

		DataReader dataReader(false, inputStream);

		if(dataReader.ReadUInt32() != c_ssa_signature)
			return 0;
		if(dataReader.ReadUInt32() != c_ssa_versionMajor)
			return 0;
		if(dataReader.ReadUInt32() != c_ssa_versionMinor)
			return 0;
		return 1;
	}
};