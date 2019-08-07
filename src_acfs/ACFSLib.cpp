/*
 * Copyright (c) 2018-2019 Amir Czwink (amir130@hotmail.de)
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
#include <ACFSLib.hpp>

#include "coh_sga/COH_SGA_Format.hpp"
#include "ea_big/EA_BIG_Format.hpp"
#include "g2_vdfs/Gothic2_VDFS_Format.hpp"
#include "g3_pak/Gothic3_PAK_Format.hpp"
#include "scct_umd/SCCT_UMD_Format.hpp"

void RegisterACFSFileSystemFormats()
{
	FileSystemFormat::Register(new COH_SGA_Format);
	FileSystemFormat::Register(new EA_BIG_Format);
	FileSystemFormat::Register(new G2_VDFS_Format);
	FileSystemFormat::Register(new G3_PAK_Format);
	FileSystemFormat::Register(new SCCT_UMD_Format);
}