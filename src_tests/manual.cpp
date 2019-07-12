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
#include <ACFSLib.hpp>
using namespace StdXX;

static void HashDir(AutoPointer<const Directory> dir, const Path& dirPath, Map<Path, String>& hashes)
{
	/*
	stdOut << "THE TEST" << endl;
	UniquePointer<InputStream> input = dir->GetChild(u8"G3_Hero_Body_FatSmith.xact").Cast<const File>()->OpenForReading(true);
	Crypto::HashingInputStream hashingInputStream(*input, Crypto::HashAlgorithm::MD5);
	hashingInputStream.FlushTo(nullOutputStream);

	UniquePointer<Crypto::HashFunction> hasher = hashingInputStream.Reset();
	stdOut << hasher->GetDigestString() << endl;
	return;*/

	NullOutputStream nullOutputStream;

	for(const auto &childName : *dir)
	{
		auto child = dir->GetChild(childName);
		if(child.IsInstanceOf<const Directory>())
		{
			HashDir(child.Cast<const Directory>(), dirPath / childName, hashes);
		}
		else
		{
			Path filePath = dirPath / childName;
			ASSERT(!hashes.Contains(filePath), u8"? File apparently exists twice! ?");

			stdOut << u8"Hashing: " << filePath << u8"...";

			UniquePointer<InputStream> input = child.Cast<const File>()->OpenForReading(true);
			Crypto::HashingInputStream hashingInputStream(*input, Crypto::HashAlgorithm::MD5);
			hashingInputStream.FlushTo(nullOutputStream);

			UniquePointer<Crypto::HashFunction> hasher = hashingInputStream.Reset();
			hasher->Finish();
			hashes[filePath] = hasher->GetDigestString();

			stdOut << u8" -> " << hashes[filePath].ToLowercase() << endl;
		}
	}
}

int32 Main(const String &programName, const FixedArray<String> &args)
{
	RegisterACFSFileSystemFormats();

	//check
	if(args.GetNumberOfElements() != 1)
	{
		stdErr << u8"Invalid commandline. Pass only path to archive." << endl;
		return EXIT_FAILURE;
	}

	//load file system
	Path inputPath = OSFileSystem::GetInstance().FromNativePath(args[0]);
	UniquePointer<FileSystem> fileSystem = FileSystem::LoadFromFile(inputPath);

	Map<Path, String> orig;
	HashDir(fileSystem->GetRoot(), String(u8"/"), orig);

	return EXIT_SUCCESS;
}