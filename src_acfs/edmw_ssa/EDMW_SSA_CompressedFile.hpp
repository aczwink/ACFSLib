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

/*
I got this algorithm through reverse engineering.
Also all the tables are from there.
Source file: Low-Level Engine.dll from Empires DMW v1.3
Root function: UDataCompression::Decompress
*/

//Distance tables
static const uint8 distanceBitLengthTable[64] = {2, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
static const uint8 distanceInitIdxTable[64] = {3, 13, 5, 25, 9, 17, 1, 62, 30, 46, 14, 54, 22, 38, 6, 58, 26, 42, 10, 50, 18, 34, 66, 2, 124, 60, 92, 28, 108, 44, 76, 12, 116, 52, 84, 20, 100, 36, 68, 4, 120, 56, 88, 24, 104, 40, 72, 8, 240, 112, 176, 48, 208, 80, 144, 16, 224, 96, 160, 32, 192, 64, 128, 0}; //i don't understand this table

//Length tables
static const uint8 lengthBitLengthTable[16] = {3, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7};
static const uint8 lengthExtraBitsTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};
static const uint16 lengthExtraTable[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 14, 22, 38, 70, 134, 262};
static const uint8 lengthInitIdxTable[16] = {5, 3, 1, 6, 10, 2, 12, 20, 4, 24, 8, 48, 16, 32, 64, 0}; //i don't understand this table

//computed tables
static uint8 distanceMap[256];
static uint8 lengthMap[256];

static void ConstructCodingTable(uint8 size, const uint8 *pSrcTable1, const uint8 *pSrcTable2, uint8 *pDestTable)
{
	int8 i;
	uint16 value, idx;

	for(i = size - 1; i >= 0; i--)
	{
		value = pSrcTable1[i];
		idx = pSrcTable2[i];

		while(idx < 265)
		{
			pDestTable[idx] = i;
			idx += (1 << value);
		}
	}
}

class EDMW_SSA_Decompressor : public Decompressor
{
public:
	//Constructor
	inline EDMW_SSA_Decompressor(InputStream &inputStream) : Decompressor(inputStream), bitInput(this->inputStream), dict(4096)
	{
		this->readHeader = false;
	}

	//Methods
	uint32 GetBytesAvailable() const override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
		return 0;
	}

	bool IsAtEnd() const override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
		return false;
	}

	uint32 ReadBytes(void *destination, uint32 count) override
	{
		if(!this->readHeader)
		{
			this->inputStream.Skip(4); //PK01 signature
			DataReader dataReader(false, this->inputStream);
			this->leftSize = dataReader.ReadUInt32();
			ASSERT(dataReader.ReadUInt32() == 0, u8"Report this plEASE!");

			uint8 isVariableLength = dataReader.ReadByte();
			ASSERT(isVariableLength == 0, u8"REPORT THIS PLEASE!"); //1 = variable length; 0 = fixed length;

			this->dictionarySizeCoding = dataReader.ReadByte();
			ASSERT(this->dictionarySizeCoding == 6, u8"REPORT THIS PLEASE!"); //6 indicates dictionary size of 4096

			static bool g_tablesComputed = false;
			if(!g_tablesComputed)
			{
				//some kind of constant length shannon fano tree
				ConstructCodingTable(sizeof(distanceBitLengthTable), distanceBitLengthTable, distanceInitIdxTable, distanceMap);
				ConstructCodingTable(sizeof(lengthBitLengthTable), lengthBitLengthTable, lengthInitIdxTable, lengthMap);

				g_tablesComputed = true;
			}

			this->readHeader = true;
		}

		if(this->leftSize == 0)
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

		return static_cast<uint32>(dest - static_cast<uint8 *>(destination));
	}

	uint32 Skip(uint32 nBytes) override
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
		return 0;
	}

private:
	//Members
	bool readHeader;
	BitInputStreamBitReversed bitInput;
	uint32 leftSize;
	uint8 dictionarySizeCoding;
	FIFOBuffer buffer;
	SlidingDictionary dict;

	//Methods
	void DecompressNextBlock()
	{
		if(!this->bitInput.Read(1)) //opposite of zip implode-.-
		{
			//literal data
			uint8 literal = static_cast<uint8>(this->bitInput.Read(8));
			this->PassthroughByte(literal);
			this->leftSize--;
		}
		else
		{
			//sliding dictionary match

			//Decode the length first, also opposite of zip-.-
			uint8 readLength = lengthMap[(uint8)this->bitInput.Get(8)];
			this->bitInput.Skip(lengthBitLengthTable[readLength]);

			uint16 decodedLength;
			if(lengthExtraBitsTable[readLength]) //check if the length is extra long
				decodedLength = (uint16)this->bitInput.Read(lengthExtraBitsTable[readLength]) + lengthExtraTable[readLength];
			else //no extra length
				decodedLength = readLength;

			//Decode the distance
			uint8 readDistance = distanceMap[(uint8)this->bitInput.Get(8)];
			this->bitInput.Skip(distanceBitLengthTable[readDistance]);

			uint16 decodedDistance;
			if(decodedLength == 0) //2bit extra distance
				decodedDistance = (uint16)this->bitInput.Read(2) | ((uint16)readDistance << 2);
			else //'dictionarySizeCoding' extra bits, this time they are lower bits
				decodedDistance = (readDistance << this->dictionarySizeCoding) | (uint8)this->bitInput.Read(this->dictionarySizeCoding);

			decodedLength += 2; //minimum match length

			//Write into dictionary
			this->dict.Copy(decodedDistance + 1, decodedLength, this->buffer);
			this->leftSize -= decodedLength;
		}
	}

	//Inline
	inline void PassthroughByte(uint8 byte)
	{
		this->buffer.WriteBytes(&byte, 1);
		this->dict.Append(byte);
	}
};

class EDMW_SSA_CompressedFile : public ContainerFile
{
public:
	//Constructor
	inline EDMW_SSA_CompressedFile(const ContainerFileHeader &header, ContainerFileSystem *fileSystem) : ContainerFile(header, fileSystem)
	{
	}

	//Methods
	UniquePointer<InputStream> OpenForReading(bool verify) const override
	{
		UniquePointer<InputStream> input = ContainerFile::OpenForReading(verify);
		ChainedInputStream* chain = new ChainedInputStream(Move(input));

		chain->Add(new BufferedInputStream(chain->GetEnd())); //add a buffer for performance
		chain->Add(new EDMW_SSA_Decompressor(chain->GetEnd()));
		return chain;
	}
};