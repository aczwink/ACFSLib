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
#include <ACFSLib.hpp>
using namespace StdXX;

void Extract(AutoPointer<Directory> dir, const Path &outputPath)
{
	OSFileSystem::GetInstance().CreateDirectoryTree(outputPath);
	/*
	 * if(!currentPath.GetParent().CreateDirectoryTree())
		{
			stdErr << endl << "Could not create folder \"" << currentPath << "\"." << endl;
			return false;
		}
	 */

	for(auto child : *dir)
	{
		Path currentPath = outputPath / Path(child->GetName());
		if(child.IsInstanceOf<Directory>())
		{
			Extract(child.Cast<Directory>(), currentPath);
		}
		else
		{
			stdOut << u8"Currently extracting " << dir->GetPath() / child->GetName() << u8" (" << String::FormatBinaryPrefixed(child->GetSize()) << u8")" << endl;
			UniquePointer<InputStream> input = child.Cast<File>()->OpenForReading();
			/*
			 * if(!currentFile.Open(output + '\\' + CString(data.fileHeaders.pFileHeaders[i].pFileName)))
		{
			stdErr << endl << "Could not create file \"" << data.fileHeaders.pFileHeaders[i].pFileName << "\" in directory \"" << output << '"' << endl;
			return false;
		}
			 */
			FileOutputStream output(currentPath);
			input->FlushTo(output);
		}
	}
}

void PrintManual()
{
	stdOut
		<< u8"Usage: "
		<< endl
		<< u8"  " << u8"ArC archive [command...]"
		<< endl
		<< endl
		<< u8"   archive:      path to a file system"
		<< endl
		<< u8"   command:"
		<< endl
		<< u8"     e     extract archive"
		<< endl
		<< u8"     o     open archive"
		<< endl
		<< endl;
}

int32 Main(const String &programName, const FixedArray<String> &args)
{
	RegisterACFSFileSystemFormats();

	//At least the archive must be mentioned
	if(args.GetNumberOfElements() < 1)
	{
		PrintManual();
		return EXIT_SUCCESS;
	}

	//load file system
	Path inputPath = OSFileSystem::GetInstance().FromNativePath(args[0]);
	UniquePointer<FileSystem> fileSystem;

	//execute commands
	for(uint32 i = 1; i < args.GetNumberOfElements(); i++)
	{
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

			fileSystem = FileSystem::Create(fsId, inputPath);
			/*
	 * if(!file.Open(output))
	{
		stdErr << "Couldn't open output file: '" << output << '\'' << endl;
		return false;
	}
	 */
		}
		else if(args[i] == u8"e")
		{
			if(fileSystem.IsNull())
			{
				stdErr << u8"No filesystem is loaded. Can't extract..." << endl;
				return EXIT_FAILURE;
			}

			Path outputPath = OSFileSystem::GetInstance().ToAbsolutePath(inputPath).GetParent() / inputPath.GetTitle();
			Extract(fileSystem->GetRoot(), outputPath);
		}
		else if(args[i] == u8"o")
		{
			fileSystem = FileSystem::LoadFromFile(inputPath);
			/*
			 * * if(!file.Open(input))
			 * {
			 * stdErr << "Error could not open input file" << endl;
			 * return false;
			 * }
			 * */

			if(fileSystem.IsNull())
			{
				stdOut << u8"Couldn't open archive. Probably unknown file system." << endl;
				return EXIT_SUCCESS;
			}

			stdOut << u8"Found file system: " << fileSystem->GetFormat()->GetName() << endl;
			stdOut << u8"File system size: " << String::FormatBinaryPrefixed(fileSystem->GetSize()) << endl;
		}
		else
		{
			stdErr << u8"Invalid command: " << args[i] << endl;
			PrintManual();
			return EXIT_FAILURE;
		}
	}

	stdOut << endl << u8"Done." << endl;
	return EXIT_SUCCESS;
}