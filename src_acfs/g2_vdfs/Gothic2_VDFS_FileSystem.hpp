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

#define VDFS_SIGNATURE u8"PSVDSC_V2.00\n\r\n\r"
#define VDFS_SIGNATURE_SIZE 16

enum VDFS_EntryType
{
	VDFS_ENTRYTYPE_DIRECTORY = 0x80000000,
	VDFS_ENTRYTYPE_DIRECTORY_LASTENTRY = 0x40000000,
};

struct VDFS_Entry
{
	String name;
	uint32 offsetOrChildrenIndex;
	uint32 size;
	uint32 type;
	uint32 attributes;
};

class G2_VDFS_FileSystem : public ContainerFileSystem
{
public:
	//Constructor
	inline G2_VDFS_FileSystem(const FileSystemFormat *format, const Path &fileSystemPath) : ContainerFileSystem(format, fileSystemPath)
	{
		if(!this->containerInputStream.IsNull())
			this->ReadFileHeaders();
	}

	void Flush() override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}

private:
	//Methods
	uint32 ReadArchiveHeader()
	{
		BufferedInputStream bufferedInputStream(*this->containerInputStream);

		bufferedInputStream.Skip(256); //skip comment
		bufferedInputStream.Skip(16); //skip signature

		DataReader dataReader(false, bufferedInputStream);
		uint32 nEntries = dataReader.ReadUInt32();
		dataReader.ReadUInt32(); //number of files
		dataReader.ReadUInt32(); //timestamp
		dataReader.ReadUInt32(); //data size
		uint32 rootOffset = dataReader.ReadUInt32();

		uint32 entrySize = dataReader.ReadUInt32();
		ASSERT(entrySize == 80, u8"REPORT THIS PLEASE!");

		this->containerInputStream->SetCurrentOffset(rootOffset);

		return nEntries;
	}

	void ReadFileHeaders()
	{
		uint32 nEntries = this->ReadArchiveHeader();

		BufferedInputStream bufferedInputStream(*this->containerInputStream);
		DataReader dataReader(false, bufferedInputStream);
		TextReader textReader(bufferedInputStream, TextCodecType::ASCII);

		FixedArray<VDFS_Entry> entries(nEntries);
		for(uint32 i = 0; i < nEntries; i++)
		{
			VDFS_Entry& entry = entries[i];

			entry.name = textReader.ReadString(64).Trim();
			entry.offsetOrChildrenIndex = dataReader.ReadUInt32();
			entry.size = dataReader.ReadUInt32();
			entry.type = dataReader.ReadUInt32();
			entry.attributes = dataReader.ReadUInt32();

			const uint32 knownTypes = VDFS_ENTRYTYPE_DIRECTORY
			                          | VDFS_ENTRYTYPE_DIRECTORY_LASTENTRY;
			ASSERT((entry.type & (~knownTypes)) == 0, u8"UNKNOWN TYPES");
		}

		if(nEntries > 0)
			this->RestoreDirectoryContents(0, String(u8"/"), entries);
	}

	void RestoreDirectoryContents(uint32 index, const Path& parentPath, const FixedArray<VDFS_Entry>& entries)
	{
		for(; true; index++)
		{
			const VDFS_Entry& entry = entries[index];

			Path ownPath = parentPath / entry.name;
			if(entry.type & VDFS_ENTRYTYPE_DIRECTORY)
			{
				this->RestoreDirectoryContents(entry.offsetOrChildrenIndex, ownPath, entries);
			}
			else
			{
				ContainerFileHeader header;
				header.uncompressedSize = entry.size;
				header.offset = entry.offsetOrChildrenIndex;
				this->AddSourceFile(ownPath, header);
			}

			if((entry.type & VDFS_ENTRYTYPE_DIRECTORY_LASTENTRY) == VDFS_ENTRYTYPE_DIRECTORY_LASTENTRY)
				break;
		}
	}
};