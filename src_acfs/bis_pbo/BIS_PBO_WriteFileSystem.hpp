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
using namespace StdXX::FileSystem;

class BIS_PBO_WriteFileSystem : public ArchiveCreationFileSystem
{
public:
	//Constructor
	inline BIS_PBO_WriteFileSystem(UniquePointer<FileOutputStream> &&outputStream) : ArchiveCreationFileSystem(StdXX::Move(outputStream))
	{
	}

private:
	//Methods
	void WriteContainer(SeekableOutputStream& outputStream) override
	{
		DataWriter dataWriter(false, outputStream);
		TextWriter textWriter(outputStream, TextCodecType::Latin1);

		auto fileEntries = this->TraverseFiles();
		for(const auto& entry : fileEntries)
		{
			textWriter.WriteStringZeroTerminated(entry.key.String().SubString(1).Replace(u8"/", u8"\\"));
			dataWriter.WriteUInt32(static_cast<uint32>(PboEntryType::Uncompressed));
			dataWriter.WriteUInt32(static_cast<uint32>(entry.value->Info().size));
			dataWriter.WriteUInt32(0);
			dataWriter.WriteUInt32(0);
			dataWriter.WriteUInt32(static_cast<uint32>(entry.value->Info().size));
		}

		dataWriter.WriteByte(0);
		dataWriter.WriteUInt32(0);
		dataWriter.WriteUInt32(0);
		dataWriter.WriteUInt32(0);
		dataWriter.WriteUInt32(0);
		dataWriter.WriteUInt32(0);

		for(const auto& entry : fileEntries)
		{
			entry.value->OpenForReading()->FlushTo(outputStream);
		}
	}
};