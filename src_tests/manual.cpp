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

static void CopyDir(AutoPointer<const Directory> dir, const Path& dirPath, FileSystem& targetFileSystem);
static void HashDir(AutoPointer<const Directory> dir, const Path& dirPath, const FileSystem& fileSystem, Map<Path, String>& hashes);

static void HashNode(AutoPointer<const FileSystemNode> node, const Path& nodePath, const FileSystem& fileSystem, Map<Path, String>& hashes)
{
	if(node.IsInstanceOf<const Directory>())
	{
		HashDir(node.Cast<const Directory>(), nodePath, fileSystem, hashes);
	}
	else if(node.IsInstanceOf<const Link>())
	{
		Path target = node.Cast<const Link>()->ReadTarget();
		if(target.IsRelative())
		{
			target = (nodePath.GetParent() / target).Normalized();
		}
		//HashNode(fileSystem.GetNode(target), target, fileSystem, hashes);
	}
	else
	{
		ASSERT(!hashes.Contains(nodePath), u8"? File apparently exists twice! ?");

		stdOut << u8"Hashing: " << nodePath << u8"...";

		UniquePointer<InputStream> input = node.Cast<const File>()->OpenForReading(true);
		Crypto::HashingInputStream hashingInputStream(*input, Crypto::HashAlgorithm::MD5);
		NullOutputStream nullOutputStream;
		hashingInputStream.FlushTo(nullOutputStream);

		UniquePointer<Crypto::HashFunction> hasher = hashingInputStream.Reset();
		hasher->Finish();
		hashes[nodePath] = hasher->GetDigestString();

		stdOut << u8" -> " << hashes[nodePath].ToLowercase() << endl;
	}
}

static void HashDir(AutoPointer<const Directory> dir, const Path& dirPath, const FileSystem& fileSystem, Map<Path, String>& hashes)
{
	for(const auto &childName : *dir)
	{
		auto child = dir->GetChild(childName);
		HashNode(child, dirPath / childName, fileSystem, hashes);
	}
}

static void CopyNode(AutoPointer<const FileSystemNode> node, const Path& nodePath, FileSystem& targetFileSystem)
{
	if(node.IsInstanceOf<const Directory>())
	{
		targetFileSystem.GetNode(nodePath.GetParent()).Cast<Directory>()->CreateSubDirectory(nodePath.GetName());
		CopyDir(node.Cast<const Directory>(), nodePath, targetFileSystem);
	}
	else if(node.IsInstanceOf<const Link>())
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}
	else
	{
		UniquePointer<InputStream> input = node.Cast<const File>()->OpenForReading(false);
		UniquePointer<OutputStream> output = targetFileSystem.CreateFile(nodePath);

		input->FlushTo(*output);
	}
}

static void CopyDir(AutoPointer<const Directory> dir, const Path& dirPath, FileSystem& targetFileSystem)
{
	for(const auto &childName : *dir)
	{
		auto child = dir->GetChild(childName);
		CopyNode(child, dirPath / childName, targetFileSystem);
	}
}

static void CreateCopy(const FileSystem& sourceFileSystem, const Path& copyFileSystemPath)
{
	UniquePointer<FileSystem> copyFileSystem = FileSystem::Create(FileTypes::UTI::zip, copyFileSystemPath); //TODO: get id from file extension in this case

	CopyDir(sourceFileSystem.GetRoot(), String(u8"/"), *copyFileSystem);
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
	UniquePointer<const FileSystem> fileSystem = FileSystem::LoadFromFileReadOnly(inputPath);

	Map<Path, String> orig;
	HashNode(fileSystem->GetRoot(), String(u8"/"), *fileSystem, orig);
	CreateCopy(*fileSystem, String(u8"/home/amir/Bilder/____TMP____COPY.zip")); //TODO: TEMP FOLDER

	return EXIT_SUCCESS;
}