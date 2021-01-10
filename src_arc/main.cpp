/*
 * Copyright (c) 2018,2021 Amir Czwink (amir130@hotmail.de)
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

void Extract(AutoPointer<const Directory> dir, const Path& dirPath, const Path &outputPath)
{
	OSFileSystem::GetInstance().CreateDirectoryTree(outputPath);
	/*
	 * if(!currentPath.GetParent().CreateDirectoryTree())
		{
			stdErr << endl << "Could not create folder \"" << currentPath << "\"." << endl;
			return false;
		}
	 */

	for(const auto &childName : *dir)
	{
		Path currentOutputPath = outputPath / childName;

		auto child = dir->GetChild(childName);
		if(child.IsInstanceOf<const Directory>())
		{
			Extract(child.Cast<const Directory>(), dirPath / childName, currentOutputPath);
		}
		else
		{
			AutoPointer<const File> file = child.Cast<const File>();
			stdOut << u8"Currently extracting " << dirPath / childName << u8" (" << String::FormatBinaryPrefixed(file->QueryInfo().size) << u8")" << endl;
			UniquePointer<InputStream> input = file->OpenForReading(true);
			/*
			 * if(!currentFile.Open(output + '\\' + CString(data.fileHeaders.pFileHeaders[i].pFileName)))
		{
			stdErr << endl << "Could not create file \"" << data.fileHeaders.pFileHeaders[i].pFileName << "\" in directory \"" << output << '"' << endl;
			return false;
		}
			 */
			FileOutputStream output(currentOutputPath);
			input->FlushTo(output);
		}
	}
}

UniquePointer<ReadableFileSystem> OpenFileSystemReadOnly(const Path& inputPath, bool withPw)
{
	const Format* fsFormat = Format::FindBestFormat(inputPath);
	if(fsFormat == nullptr)
	{
		stdErr << "Error could not open input file" << endl;
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

	Group mount(u8"mount", u8"Mount a filesystem");
	PathArgument mountPointArg(u8"mountPoint", u8"Path where the filesystem should be mounted");
	mount.AddPositionalArgument(mountPointArg);
	subCommandArgument.AddCommand(mount);

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
			Path outputPath = OSFileSystem::GetInstance().ToAbsolutePath(inputPath).GetParent() / inputPath.GetTitle();
			Path root = String(u8"/");
			Extract(fileSystem->GetDirectory(root), root, outputPath);

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
			OSFileSystem::GetInstance().MountReadOnly(mountPoint, *fileSystem);
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;

		/*
		if(args[i] == u8"af")
		{
			if(fileSystem.IsNull())
			{
				stdErr << u8"No filesystem is loaded. Can't add..." << endl;
				return EXIT_FAILURE;
			}

			String fileName = args[++i];

			UniquePointer<OutputStream> targetFile = fileSystem->CreateFile(fileName);
			FileInputStream origFile(fileName);
			origFile.FlushTo(*targetFile);
		}
		else if(args[i] == u8"c")
		{
			String fsId = args[++i];

			fileSystem = RWFileSystem::Create(fsId, inputPath);*/
			/*
	 * if(!file.Open(output))
	{
		stdErr << "Couldn't open output file: '" << output << '\'' << endl;
		return false;
	}
	 */
		//}
}