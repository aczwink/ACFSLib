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
#include <Std++.hpp>
using namespace StdXX;

class SCCT_UMD_FileSystem : public ContainerFileSystem
{
public:
	//Constructor
	SCCT_UMD_FileSystem(const FileSystemFormat *format, const Path &fileSystemPath) : ContainerFileSystem(format, fileSystemPath)
	{
		if(!this->containerInputStream.IsNull())
			this->ReadFileHeaders();
	}

	//Destructor
	~SCCT_UMD_FileSystem()
	{
		this->Flush();
	}

	//Methods
	void Flush() override
	{
		if(!this->isFlushed)
		{
			UniquePointer<FileOutputStream> tempFile = this->OpenTempContainer();
			DataWriter dataWriter(false, *tempFile);

			struct FileHeader
			{
				Path filePath;
				uint32 dataOffset;
				uint32 fileSize;
			};
			LinkedList<FileHeader> fileHeaders; //TODO: make this a DynamicArray

			//write file data first
			uint32 offset = 0;
			for(auto file : this->root->WalkFiles())
			{
				FileHeader fh;
				fh.filePath = file->GetPath();
				fh.dataOffset = offset;
				fh.fileSize = static_cast<uint32>(file->GetSize());
				offset += fh.fileSize;
				fileHeaders.InsertTail(fh);

				file->OpenForReading()->FlushTo(*tempFile);
			}

			const uint32 fileHeadersOffset = offset;
			uint32 totalSize = offset;
			//write file headers
			totalSize += this->WriteCompressedInt(fileHeaders.GetNumberOfElements(), *tempFile);
			FIFOBuffer nameBuffer;
			TextWriter textWriter(nameBuffer, TextCodecType::ASCII);
			for(const auto &fh : fileHeaders)
			{
				String fileName = fh.filePath.GetString();
				fileName = fileName.SubString(1); //remove the root "/"
				fileName = fileName.Replace(u8"/", u8"\\");
				textWriter.WriteString(fileName);
				uint32 nameLength = nameBuffer.GetNumberOfElements()+1;

				totalSize += this->WriteCompressedInt(nameLength, *tempFile);
				nameBuffer.FlushTo(*tempFile);
				byte b = 0;
				tempFile->WriteBytes(&b, 1);

				dataWriter << fh.dataOffset << fh.fileSize;
				dataWriter.WriteUInt32(0);

				totalSize += nameLength + 3*4;
			}

			//write footer
			totalSize += sizeof(UMDFooter);

			dataWriter.WriteUInt32(UMD_FOOTER_SIGNATURE);
			dataWriter.WriteUInt32(fileHeadersOffset);
			dataWriter.WriteUInt32(totalSize);
			dataWriter.WriteUInt32(UMD_FOOTER_UNKNOWN1);
			dataWriter.WriteUInt32(UMD_FOOTER_UNKNOWN2);

			//swap files
			this->SwapWithTempContainer(tempFile);
			this->isFlushed = true;
		}
	}

private:
	//Methods
	uint32 ReadCompressedInt(InputStream &inputStream) //reversed
	{
		byte b;

		inputStream.ReadBytes(&b, 1);
		if(b & 0x80) //value is negative
		{
			NOT_IMPLEMENTED_ERROR;
		}
		if(!(b & 0x40)) //no byte follows
			return b & 0x3F;

		uint32 result = b & 0x3F;
		inputStream.ReadBytes(&b, 1);
		result |= (b & 0x7F) << 6;

		if(b & 0x80)
		{
			inputStream.ReadBytes(&b, 1);
			result |= (b & 0x7F) << 13;
		}

		if(b & 0x80)
		{
			NOT_IMPLEMENTED_ERROR;
		}

		return result;
	}

	void ReadFileHeaders()
	{
		UMDFooter footer;
		this->containerInputStream->SetCurrentOffset(this->containerInputStream->GetSize() - sizeof(footer));
		this->containerInputStream->ReadBytes(&footer, sizeof(footer));

		//read file headers
		this->containerInputStream->SetCurrentOffset(footer.fileHeadersOffset);

		BufferedInputStream bufferedInputStream(*this->containerInputStream);
		DataReader dataReader(false, bufferedInputStream);
		TextReader textReader(bufferedInputStream, TextCodecType::ASCII);
		uint32 nFiles = this->ReadCompressedInt(bufferedInputStream);
		for(uint32 i = 0; i < nFiles; i++)
		{
			uint32 length = this->ReadCompressedInt(bufferedInputStream);
			length--; //the nullbyte is included in length

			String fileName = textReader.ReadString(length);
			bufferedInputStream.Skip(1); //skip nullbyte
			fileName = fileName.Replace(u8"\\", u8"/"); //convert from windows line endings

			uint32 dataOffset = dataReader.ReadUInt32();
			uint32 fileSize = dataReader.ReadUInt32();
			bufferedInputStream.Skip(4); //padding

			this->AddSourceFile(Path(fileName), dataOffset, fileSize); //add file
		}
	}

	uint32 WriteCompressedInt(uint32 value, OutputStream &outputStream)
	{
		byte b[3];
		uint8 nBytes = 1;

		b[0] = value & 0x3F;
		if(value & 0xFFFFFFC0)
		{
			b[0] |= 0x40;
			value >>= 6;
			b[1] = value & 0x7F;
			value >>= 7;
			nBytes++;

			if(value & 0x7FFFF)
			{
				b[1] |= 0x80;
				b[2] = value & 0x7F;
				nBytes++;
			}
		}

		outputStream.WriteBytes(b, nBytes);
		return nBytes;
	}
};