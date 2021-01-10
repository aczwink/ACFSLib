/*
 * Copyright (c) 2018-2019,2021 Amir Czwink (amir130@hotmail.de)
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
//Class header
#include "COH_SGA_Format.hpp"
//Local
#include "COH_SGA_FileSystem.hpp"
#include "SGA.hpp"

//Public methods
RWFileSystem *COH_SGA_Format::CreateFileSystem(const Path & fileSystemPath) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return nullptr;
}

String COH_SGA_Format::GetId() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return String();
}

String COH_SGA_Format::GetName() const
{
	return u8"SGA - Company of Heroes";
}

float32 COH_SGA_Format::Matches(SeekableInputStream & inputStream) const
{
	DataReader reader(false, inputStream);
	TextReader textReader(inputStream, TextCodecType::ASCII);

	try
	{
		if (textReader.ReadString(SGA_SIGNATURE_LENGTH) != SGA_SIGNATURE)
			return 0;
	}
	catch(const ErrorHandling::IllegalEncodedCharException& e)
	{
		return 0;
	}

	uint32 version = reader.ReadUInt32();
	switch (version)
	{
		//we know how to read these
	case 2:
	case 4:
		return 1;
	}

	return 0;
}

RWFileSystem *COH_SGA_Format::OpenFileSystem(const Path &fileSystemPath, const OpenOptions& openOptions) const
{
	return new COH_SGA_FileSystem(this, fileSystemPath);
}

ReadableFileSystem *COH_SGA_Format::OpenFileSystemReadOnly(const Path &fileSystemPath, const OpenOptions& openOptions) const
{
	return this->OpenFileSystem(fileSystemPath, openOptions);
}
