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

#include "Gothic2_VDFS_FileSystem.hpp"

class G2_VDFS_Format : public FileSystemFormat
{
public:
	FileSystem *CreateFileSystem(const Path &fileSystemPath) const override
	{
		return new G2_VDFS_FileSystem(this, fileSystemPath);
	}

	String GetId() const override
	{
		return u8"vdf";
	}

	String GetName() const
	{
		return u8"Gothic 2 VDFS (Virtual disk filesystem)";
	}

	float32 Matches(SeekableInputStream &inputStream) const override
	{
		inputStream.Skip(256);

		byte readSignature[VDFS_SIGNATURE_SIZE];
		if(inputStream.ReadBytes(readSignature, sizeof(readSignature)) < sizeof(readSignature))
			return 0;
		if(MemCmp(VDFS_SIGNATURE, readSignature, VDFS_SIGNATURE_SIZE) == 0)
			return 1;
		return 0;
	}
};