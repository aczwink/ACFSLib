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

static void PackDir(const Path& relativeDirPath, const Path& inputBasePath, WritableFileSystem& writableFileSystem)
{
	Path dirPath = inputBasePath / relativeDirPath.String().SubString(1);
	File dir(dirPath);

	stdOut << endl << u8"Packing dir: " << relativeDirPath << endl;

	for(const auto& childEntry : dir)
	{
		Path relativeChildPath = relativeDirPath / childEntry.name;
		Path absoluteChildPath = inputBasePath / relativeChildPath.String().SubString(1);

		if(childEntry.type == FileType::Directory)
		{
			writableFileSystem.CreateDirectory(relativeChildPath);
			PackDir(relativeChildPath, inputBasePath, writableFileSystem);
		}
		else
		{
			stdOut << u8"Packing: " << relativeChildPath << endl;

			FileInputStream inputStream(absoluteChildPath);
			auto outputStream = writableFileSystem.CreateFile(relativeChildPath);

			inputStream.FlushTo(*outputStream);
		}
	}
}

void Pack(const Path& inputPath, WritableFileSystem& writableFileSystem)
{
	PackDir({u8"/"}, inputPath, writableFileSystem);

	stdOut << u8"Flushing filesystem to disk. This might take a while" << endl;
	writableFileSystem.Flush();
}