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
using namespace StdPlusPlus;
//Local
#include "UMD.hpp"
#include "SCCT_UMD_FileSystem.hpp"

class SCCT_UMD_Format : public FileSystemFormat
{
public:
	FileSystem *CreateFileSystem(const Path &fileSystemPath) const override
	{
		return new SCCT_UMD_FileSystem(this, fileSystemPath);
	}

	String GetId() const override
	{
		return u8"umd";
	}

	String GetName() const
	{
		return u8"Splinter Cell Chaos Theory UMD (dynamic-pc.umd)";
	}

	float32 Matches(SeekableInputStream &inputStream) const override
	{
		UMDFooter footer;

		inputStream.SetCurrentOffset(inputStream.GetSize() - sizeof(footer));
		inputStream.ReadBytes(&footer, sizeof(footer));

		if(footer.signature == UMD_FOOTER_SIGNATURE)
		{
			if(footer.fileSize != inputStream.GetSize())
			{
				stdErr << "Warning: File size does not match" << endl;
				return 0.75f;
			}
			if(footer.fileHeadersOffset >= inputStream.GetSize() - sizeof(footer))
			{
				stdOut << "Error: Offset to file headers is out of range. Is this a umd file?" << endl;
				return 0.25f;
			}

			return 1;
		}

		return 0;
	}
};