/*
 * Copyright (c) 2018-2019,2021 Amir Czwink (amir130@hotmail.de)
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

class COH_SGA_Format : public Format
{
public:
	//Methods
	RWFileSystem * CreateFileSystem(const Path & fileSystemPath) const override;
	String GetId() const override;
	String GetName() const override;
	float32 Matches(SeekableInputStream & inputStream) const override;
	RWFileSystem *OpenFileSystem(const Path &fileSystemPath, const OpenOptions& openOptions) const override;
	ReadableFileSystem *OpenFileSystemReadOnly(const Path &fileSystemPath, const OpenOptions& openOptions) const override;
};