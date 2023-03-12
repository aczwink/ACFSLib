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
#include <ACFSLib.hpp>
using namespace StdXX;
using namespace StdXX::CommandLine;
using namespace StdXX::FileSystem;

//Prototypes
void DumpInfo(const ReadableFileSystem& readableFileSystem);
void Extract(const ReadableFileSystem& readableFileSystem, const Path& dirPath, const Path &outputPath);
void Pack(const Path& inputPath, WritableFileSystem& writableFileSystem);

UniquePointer<WritableFileSystem> CreateFileSystem(const Path& outputPath, bool withPw)
{
	const Format* fsFormat = FileSystemsManager::Instance().FindFormatById(outputPath.GetFileExtension());
	if(fsFormat == nullptr)
	{
		stdErr << "Error: Could not determine a format for the output file" << endl;
		return nullptr;
	}

	stdOut << u8"Creating filesystem of type: " << fsFormat->GetName() << endl;

	OpenOptions options;
	if(withPw)
	{
		stdOut << u8"Enter password: ";
		options.password = stdIn.ReadUnechoedLine();
	}

	UniquePointer<WritableFileSystem> fileSystem = fsFormat->CreateFileSystem(outputPath, options);
	if(fileSystem.IsNull())
	{
		stdOut << u8"Couldn't create filesystem." << endl;
		return nullptr;
	}

	return fileSystem;
}

UniquePointer<ReadableFileSystem> OpenFileSystemReadOnly(const Path& inputPath, bool withPw)
{
	const Format* fsFormat = FileSystemsManager::Instance().ProbeFormat(inputPath);
	if(fsFormat == nullptr)
	{
		stdErr << "Error: Could not determine a format for the input file" << endl;
		return nullptr;
	}

	stdOut << u8"Found file system: " << fsFormat->GetName() << endl;

	OpenOptions options;
	if(withPw)
	{
		stdOut << u8"Enter password: ";
		options.password = stdIn.ReadUnechoedLine();
	}

	UniquePointer<ReadableFileSystem> fileSystem = fsFormat->OpenFileSystemReadOnly(inputPath, options);
	if(fileSystem.IsNull())
	{
		stdOut << u8"Couldn't open archive. Probably unknown file system." << endl;
		return nullptr;
	}

	stdOut << u8"File system size: " << String::FormatBinaryPrefixed(fileSystem->QuerySpace().totalSize) << endl;
	return fileSystem;
}

int32 Main(const String &programName, const FixedArray<String> &args)
{
	RegisterACFSFileSystemFormats();

	Parser commandLineParser(programName);
	commandLineParser.AddHelpOption();

	SubCommandArgument subCommandArgument(u8"command", u8"The command that should be executed");

	Option passwordOption(u8'p', u8"password", u8"Specify password for opening an encrypted filesystem");
	commandLineParser.AddOption(passwordOption);

	Group extract(u8"extract", u8"Extract a filesystem");
	subCommandArgument.AddCommand(extract);

	Group info(u8"info", u8"Dump information about a filesystem");
	subCommandArgument.AddCommand(info);

	Group mount(u8"mount", u8"Mount a filesystem");
	PathArgument mountPointArg(u8"mountPoint", u8"Path where the filesystem should be mounted");
	mount.AddPositionalArgument(mountPointArg);
	subCommandArgument.AddCommand(mount);

	Group pack(u8"pack", u8"Pack a directory into a filesystem");
	PathArgument outputArg(u8"output", u8"Path to the target filesystem");
	pack.AddPositionalArgument(outputArg);
	subCommandArgument.AddCommand(pack);

	PathArgument inputPathArg(u8"fsPath", u8"path to the filesystem");

	commandLineParser.AddPositionalArgument(subCommandArgument);
	commandLineParser.AddPositionalArgument(inputPathArg);

	if(!commandLineParser.Parse(args))
	{
		stdErr << commandLineParser.GetErrorText() << endl;
		return EXIT_FAILURE;
	}

	if(commandLineParser.IsHelpActivated())
	{
		commandLineParser.PrintHelp();
		return EXIT_SUCCESS;
	}

	const MatchResult& matchResult = commandLineParser.ParseResult();

	if(matchResult.IsActivated(extract))
	{
		Path inputPath = inputPathArg.Value(matchResult);
		UniquePointer<ReadableFileSystem> fileSystem = OpenFileSystemReadOnly(inputPath, matchResult.IsActivated(passwordOption));
		if(!fileSystem.IsNull())
		{
			Path outputPath = FileSystemsManager::Instance().OSFileSystem().ToAbsolutePath(inputPath).GetParent() / inputPath.GetTitle();
			Path root = String(u8"/");
			Extract(*fileSystem, root, outputPath);

			return EXIT_SUCCESS;
		}
	}
	else if(matchResult.IsActivated(info))
	{
		Path inputPath = inputPathArg.Value(matchResult);
		UniquePointer<ReadableFileSystem> fileSystem = OpenFileSystemReadOnly(inputPath, matchResult.IsActivated(passwordOption));
		if(!fileSystem.IsNull())
		{
			Path root = String(u8"/");
			DumpInfo(*fileSystem);

			return EXIT_SUCCESS;
		}
	}
	else if(matchResult.IsActivated(mount))
	{
		Path inputPath = inputPathArg.Value(matchResult);
		Path mountPoint = mountPointArg.Value(matchResult);
		UniquePointer<ReadableFileSystem> fileSystem = OpenFileSystemReadOnly(inputPath, matchResult.IsActivated(passwordOption));
		if(!fileSystem.IsNull())
		{
			FileSystemsManager::Instance().OSFileSystem().MountReadOnly(mountPoint, *fileSystem);
			return EXIT_SUCCESS;
		}
	}
	else if(matchResult.IsActivated(pack))
	{
		Path inputPath = inputPathArg.Value(matchResult);
		Path outputPath = outputArg.Value(matchResult);
		UniquePointer<WritableFileSystem> fileSystem = CreateFileSystem(outputPath, matchResult.IsActivated(passwordOption));
		if(!fileSystem.IsNull())
		{
			Pack(inputPath, *fileSystem);
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;
}