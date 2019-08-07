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

const uint32 ea_unknown_signature_l219 = FOURCC(u8"L219");
const uint32 ea_unknown_signature_l225 = FOURCC(u8"L225");
const uint32 ea_unknown_signature_l231 = FOURCC(u8"L231");

class EA_BIG_FileSystem : public ContainerFileSystem
{
public:
	//Constructor
	inline EA_BIG_FileSystem(const FileSystemFormat *format, const Path &fileSystemPath) : ContainerFileSystem(format, fileSystemPath)
	{
		if (!this->containerInputStream.IsNull())
			this->ReadFileHeaders();
	}

	void Flush() override {
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}

private:
	//Methods
	void ReadFileHeaders()
	{
		BufferedInputStream bufferedInputStream(*this->containerInputStream);
		bufferedInputStream.Skip(4); //signature
		bufferedInputStream.Skip(4); //archive size

		DataReader beReader(true, bufferedInputStream);
		TextReader textReader(bufferedInputStream, TextCodecType::Latin1);

		uint32 nFiles = beReader.ReadUInt32();
		uint32 fileDataOffset = beReader.ReadUInt32();

		for(uint32 i = 0; i < nFiles; i++)
		{
			ContainerFileHeader header;
			header.offset = beReader.ReadUInt32();
			header.uncompressedSize = beReader.ReadUInt32();

			String filePath = textReader.ReadZeroTerminatedString();
			filePath = filePath.Replace(u8"\\", u8"/"); //convert from windows line endings
			this->AddSourceFile(u8"/" + filePath, header);
		}

		DataReader leReader(false, bufferedInputStream);
		uint32 unknownSignature = leReader.ReadUInt32();
		ASSERT((unknownSignature == ea_unknown_signature_l219) || (unknownSignature == ea_unknown_signature_l225) || (unknownSignature == ea_unknown_signature_l231), u8"REPORT THIS PLEASE!");

		//probably padding
		uint64 nPaddingBytes = fileDataOffset - (this->containerInputStream->GetCurrentOffset() - bufferedInputStream.GetBytesAvailable());
		ASSERT((nPaddingBytes == 3) || (nPaddingBytes == 4), u8"REPORT THIS PLEASE!");
		for(uint64 i = 0; i < nPaddingBytes; i++)
		{
			ASSERT(leReader.ReadByte() == 0, u8"REPORT THIS PLEASE!");
		}
	}
};