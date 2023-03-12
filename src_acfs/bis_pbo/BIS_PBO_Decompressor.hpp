/*
 * Copyright (c) 2019-2023 Amir Czwink (amir130@hotmail.de)
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

class BIS_PBO_Decompressor : public DictionaryDecompressor
{
public:
	//Constructor
	inline BIS_PBO_Decompressor(InputStream &inputStream, uint32 uncompressedSize, bool verify) : DictionaryDecompressor(inputStream, 4096), leftSize(uncompressedSize), verify(verify)
	{
		this->nBytesDecompressedTotal = 0;
		this->checksum = 0;
		this->signedChecksum = 0;
	}

private:
	//Members
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

	uint32 DecompressNextBlock(SlidingDictionary& dictionary) override
	{
		DataReader dataReader(false, this->inputStream);
		if(this->leftSize == 0)
		{
			uint32 readChecksum = dataReader.ReadUInt32();
			if(this->verify)
			{
				ASSERT((readChecksum == this->checksum) || (readChecksum == this->signedChecksum), u8"Checksum mismatch!");
			}
			return 0;
		}

		uint32 nBytesDecompressed = 0;
		uint8 flagByte = dataReader.ReadByte();
		for(uint8 bits = 0; bits < 8; bits++, flagByte >>= 1)
		{
			if(flagByte & 1u)
			{
				//directly copy to output
				this->PassthroughByte(dataReader.ReadByte(), dictionary);
				this->leftSize--;
				nBytesDecompressed++;
			}
			else
			{
				nBytesDecompressed += this->DistanceDecode(dataReader, dictionary);
			}

			if(this->leftSize == 0)
				break;
		}

		return nBytesDecompressed;
	}

	uint32 DistanceDecode(DataReader& dataReader, SlidingDictionary& dictionary)
	{
		uint16 rpos = dataReader.ReadByte();
		uint8 b2 = dataReader.ReadByte();

		rpos |= uint16(b2 & 0xF0u) << 4u;
		uint8 rlen = static_cast<uint8>((b2 & 0xFu) + 3);
		ASSERT(rpos, u8"Relative pos can't be 0");

		uint32 nBytesDecompressed = 0;
		while(rpos > this->nBytesDecompressedTotal)
		{
			this->PassthroughByte(0x20, dictionary); //put in a space
			nBytesDecompressed++;

			if(--this->leftSize == 0)
				return nBytesDecompressed;
			if(--rlen == 0)
				return nBytesDecompressed;
		}

		uint16 nBytesToCopyFromDict = static_cast<uint16>(Math::Min(this->leftSize, uint32(rlen)));
		dictionary.CopyToTail(rpos, nBytesToCopyFromDict);
		this->nBytesDecompressedTotal += nBytesToCopyFromDict;
		this->leftSize -= nBytesToCopyFromDict;

		if(this->verify)
			this->VerifyFromDict(dictionary, nBytesToCopyFromDict);

		return nBytesDecompressed + nBytesToCopyFromDict;
	}

	void VerifyFromDict(const SlidingDictionary& dictionary, uint16 nBytes)
	{
		uint8 buffer[18]; //maximum possible length for a backreference is 18

		dictionary.Read(buffer, nBytes, nBytes);
		this->AddToChecksum(buffer, nBytes);
	}

	//Inline
	inline void PassthroughByte(uint8 byte, SlidingDictionary& dictionary)
	{
		dictionary.Append(byte);
		this->nBytesDecompressedTotal++;

		if(this->verify)
			this->AddToChecksum(&byte, 1);
	}
};