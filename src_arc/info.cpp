/*
 * Copyright (c) 2023 Amir Czwink (amir130@hotmail.de)
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

static void DumpDirInfo(const ReadOnlyFile& dir)
{
	for(const auto& childEntry : dir)
	{
		ReadOnlyFile child = dir.Child(childEntry.name);

		if(childEntry.type == FileType::Directory)
			DumpDirInfo(child);
		else
		{
			stdOut << "  " << child.Path() << " (" << String::FormatBinaryPrefixed(child.Size()) << ")";
			if(child.Size() != child.Info().storedSize)
			{
				stdOut << " (stored size: " << String::FormatBinaryPrefixed(child.Info().storedSize) << ")";
			}
			stdOut << endl;
		}
	}
}

void DumpInfo(const ReadableFileSystem& readableFileSystem)
{
	ReadOnlyFile root(readableFileSystem, Path(u8"/"));
	DumpDirInfo(root);
}