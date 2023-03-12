/*
 * Copyright (c) 2018-2023 Amir Czwink (amir130@hotmail.de)
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

void Extract(const ReadableFileSystem& readableFileSystem, const Path& dirPath, const Path &outputPath)
{
	File outputDir(outputPath);
	outputDir.CreateDirectories();

	ReadOnlyFile dir(readableFileSystem, dirPath);
	for(const auto& childEntry : dir)
	{
		Path childPath = dirPath / childEntry.name;
		ReadOnlyFile child(readableFileSystem, childPath);

		Path currentOutputPath = outputPath / childEntry.name;

		if(child.Type() == FileType::Directory)
		{
			Extract(readableFileSystem, childPath, currentOutputPath);
		}
		else
		{
			stdOut << u8"Currently extracting " << childPath << u8" (" << String::FormatBinaryPrefixed(child.Info().size) << u8")" << endl;
			UniquePointer<InputStream> input = child.OpenForReading(true);
			FileOutputStream output(currentOutputPath);
			input->FlushTo(output);
		}
	}
}