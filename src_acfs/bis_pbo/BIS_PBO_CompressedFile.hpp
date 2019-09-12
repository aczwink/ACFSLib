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

class BIS_PBO_Decompressor : public Decompressor
{
public:
	//Constructor
	inline BIS_PBO_Decompressor(InputStream &inputStream, uint32 uncompressedSize, bool verify) : Decompressor(inputStream), dict(4096), leftSize(uncompressedSize), verify(verify)
	{
		this->nBytesDecompressedTotal = 0;
		this->checksum = 0;
		this->signedChecksum = 0;
	}

	//Methods
	uint32 GetBytesAvailable() const override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
		return 0;
	}

	bool IsAtEnd() const override
	{
		return (this->leftSize == 0) && this->buffer.IsEmpty();
	}

	uint32 ReadBytes(void *destination, uint32 count) override
	{
		if(this->IsAtEnd())
			return 0;

		uint8* dest = static_cast<uint8 *>(destination);
		while(count)
		{
			if(this->buffer.IsEmpty())
			{
				if(this->leftSize == 0)
					break;
				this->DecompressNextBlock();
			}

			uint32 left = Math::Min(count, this->buffer.GetBytesAvailable());
			uint32 nBytesCopied = this->buffer.ReadBytes(dest, left);

			dest += nBytesCopied;
			count -= nBytesCopied;
		}

		uint32 nBytesReadTotal = static_cast<uint32>(dest - static_cast<uint8 *>(destination));
		if(this->verify)
		{
			this->AddToChecksum(static_cast<const uint8 *>(destination), nBytesReadTotal);
			if(this->IsAtEnd())
				this->Verify();
		}
		return nBytesReadTotal;
	}

	uint32 Skip(uint32 nBytes) override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
		return 0;
	}

private:
	//Members
	FIFOBuffer buffer;
	SlidingDictionary dict;
	uint32 leftSize;
	uint32 nBytesDecompressedTotal;
	bool verify;
	uint32 checksum;
	uint32 signedChecksum;

	//Methods
	void AddToChecksum(const uint8* data, uint32 count)
	{
		for(uint32 i = 0; i < count; i++)
		{
			this->checksum += data[i];
			this->signedChecksum += (int8)data[i]; //some tools or versions or whatever have a signed checksum-.-
		}
	}

	void DecompressNextBlock()
	{
		DataReader dataReader(false, this->inputStream); //only for ReadByte

		uint8 flagByte = dataReader.ReadByte();
		for(uint8 bits = 0; bits < 8; bits++, flagByte >>= 1)
		{
			if(flagByte & 1u)
			{
				//directly copy to output
				this->PassthroughByte(dataReader.ReadByte());
				this->leftSize--;
			}
			else
			{
				this->DistanceDecode(dataReader);
			}

			if(this->leftSize == 0)
				break;
		}
	}

	void DistanceDecode(DataReader& dataReader)
	{
		uint16 rpos = dataReader.ReadByte();
		uint8 b2 = dataReader.ReadByte();

		rpos |= uint16(b2 & 0xF0u) << 4u;
		uint8 rlen = static_cast<uint8>((b2 & 0xFu) + 3);
		ASSERT(rpos, u8"Relative pos can't be 0");

		while(rpos > this->nBytesDecompressedTotal)
		{
			this->PassthroughByte(0x20); //put in a space

			if(--this->leftSize == 0)
				return;
			if(--rlen == 0)
				return;
		}

		uint16 nBytesToCopyFromDict = static_cast<uint16>(Math::Min(this->leftSize, uint32(rlen)));

		this->dict.Copy(rpos, nBytesToCopyFromDict, this->buffer);
		this->nBytesDecompressedTotal += nBytesToCopyFromDict;
		this->leftSize -= nBytesToCopyFromDict;
	}

	//Inline
	inline bool PassthroughByte(uint8 byte)
	{
		this->buffer.WriteBytes(&byte, 1);
		this->dict.Append(byte);
		this->nBytesDecompressedTotal++;
	}

	inline void Verify()
	{
		DataReader dataReader(false, this->inputStream);
		uint32 readChecksum = dataReader.ReadUInt32();

		ASSERT((readChecksum == this->checksum) || (readChecksum == this->signedChecksum), u8"Checksum mismatch!");
	}
};

class BIS_PBO_CompressedFile : public ContainerFile
{
public:
	//Constructor
	inline BIS_PBO_CompressedFile(const ContainerFileHeader &header, ContainerFileSystem *fileSystem) : ContainerFile(header, fileSystem)
	{

	}

	//Methods
	UniquePointer<InputStream> OpenForReading(bool verify) const override
	{
		UniquePointer<InputStream> input = ContainerFile::OpenForReading(verify);
		ChainedInputStream* chain = new ChainedInputStream(Move(input));

		chain->Add(new BufferedInputStream(chain->GetEnd())); //add a buffer for performance
		chain->Add(new BIS_PBO_Decompressor(chain->GetEnd(), static_cast<uint32>(this->GetHeader().uncompressedSize), verify));
		return chain;
	}
};