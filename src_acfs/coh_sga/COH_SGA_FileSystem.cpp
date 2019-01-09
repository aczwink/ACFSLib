/*
 * Copyright (c) 2018 Amir Czwink (amir130@hotmail.de)
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
#include "COH_SGA_FileSystem.hpp"
//Local
#include "COH_SGA_File.hpp"
#include "SGA.hpp"

struct FileEntry
{
	uint32 nameOffset;
	uint32 flags;
	uint32 dataOffset;
	uint32 compressedSize;
	uint32 uncompressedSize;
};

//Public methods
void COH_SGA_FileSystem::Flush()
{
	if (this->isFlushed)
		return;

	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

//Private methods
void COH_SGA_FileSystem::ReadFileHeaders()
{
	DataReader reader(false, *this->containerInputStream);
	TextReader textReader(*this->containerInputStream, TextCodecType::ASCII);

	reader.Skip(SGA_SIGNATURE_LENGTH); //signature
	uint32 version = reader.ReadUInt32();
	reader.Skip(16); //md5 of archive
	reader.Skip(64 * 2); //64 ucs2 chars => comment
	reader.Skip(16); //md5 of data header
	uint32 dataHeaderSize = reader.ReadUInt32();
	uint32 dataOffset = reader.ReadUInt32();

	if (version == 4)
	{
		uint32 platform = reader.ReadUInt32();
		ASSERT(platform == 1, u8"?!"); //the only defined platform value so far (coh1)
		//has something to do with x86 vs x86-64... check this
	}

	uint32 dataHeaderOffset = this->containerInputStream->GetCurrentOffset();

	uint32 tocOffset = reader.ReadUInt32();
	uint16 nTocs = reader.ReadUInt16();
	ASSERT(nTocs == 1, u8"multiple tocs is currently not known");

	uint32 dirsOffset = reader.ReadUInt32();
	uint16 nDirs = reader.ReadUInt16();

	uint32 filesOffset = reader.ReadUInt32();
	uint16 nFiles = reader.ReadUInt16();

	uint32 stringsOffset = reader.ReadUInt32();
	uint16 nStrings = reader.ReadUInt16();

	//read toc
	this->containerInputStream->SetCurrentOffset(dataHeaderOffset + tocOffset);
	String tocName = textReader.ReadZeroTerminatedString(64);
	textReader.ReadString(64); //archive name or so... unimportant
	reader.Skip(2); //always zero
	reader.Skip(4); //nDirs
	reader.Skip(4); //nFiles

	//read dirs
	this->containerInputStream->SetCurrentOffset(dataHeaderOffset + dirsOffset);
	FixedArray<DirEntry> dirEntries(nDirs);
	for (uint16 i = 0; i < nDirs; i++)
	{
		DirEntry& entry = dirEntries[i];

		entry.nameOffset = reader.ReadUInt32();
		entry.subdirsStart = reader.ReadUInt16();
		entry.subdirsEnd = reader.ReadUInt16();
		entry.filesStart = reader.ReadUInt16();
		entry.filesEnd = reader.ReadUInt16();
	}

	//read files
	this->containerInputStream->SetCurrentOffset(dataHeaderOffset + filesOffset);
	FixedArray<FileEntry> fileEntries(nFiles);
	for (uint16 i = 0; i < nFiles; i++)
	{
		FileEntry& entry = fileEntries[i];
		
		entry.nameOffset = reader.ReadUInt32();
		if (version == 2)
			entry.flags = reader.ReadUInt32();
		entry.dataOffset = reader.ReadUInt32();
		entry.compressedSize = reader.ReadUInt32();
		entry.uncompressedSize = reader.ReadUInt32();

		if (version == 4)
		{
			reader.ReadUInt32(); //modification time
			entry.flags = reader.ReadUInt16();
		}
	}

	//read strings
	this->containerInputStream->SetCurrentOffset(dataHeaderOffset + stringsOffset);
	Map<uint32, String> stringMap;
	for (uint16 i = 0; i < nStrings; i++)
	{
		uint32 offset = this->containerInputStream->GetCurrentOffset() - stringsOffset - dataHeaderOffset;
		stringMap[offset] = textReader.ReadZeroTerminatedString();
	}

	//now add files
	for (uint16 i = 0; i < nDirs; i++)
	{
		const DirEntry& dirEntry = dirEntries[i];
		for (uint16 j = dirEntry.filesStart; j < dirEntry.filesEnd; j++)
		{
			const FileEntry& fileEntry = fileEntries[j];

			Path path = Path(tocName) / stringMap[dirEntry.nameOffset].Replace(u8"\\", u8"/") / stringMap[fileEntry.nameOffset]; //convert from windows line endings

			ContainerFileHeader header;
			ASSERT(fileEntry.flags == 0x100, u8"unknown flagss"); //compression method, 0x100 = zlib
			header.compression = CompressionAlgorithm::DEFLATE;
			header.offset = dataOffset + fileEntry.dataOffset;
			header.uncompressedSize = fileEntry.uncompressedSize;
			header.compressedSize = fileEntry.compressedSize;
			this->AddSourceFile(path, header);
		}
	}
}