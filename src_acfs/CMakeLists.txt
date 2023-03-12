set(SOURCE_FILES
	${SOURCE_FILES}

	#bis_pbo
	${CMAKE_CURRENT_SOURCE_DIR}/bis_pbo/BIS_PBO_Decompressor.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/bis_pbo/BIS_PBO_ReadFileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/bis_pbo/BIS_PBO_Format.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/bis_pbo/PBO.hpp

	#coh_sga
		${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_FileSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_Format.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_Format.hpp

	#ea_big
	${CMAKE_CURRENT_SOURCE_DIR}/ea_big/EA_BIG_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/ea_big/EA_BIG_Format.hpp

	#edmw_ssa
	${CMAKE_CURRENT_SOURCE_DIR}/edmw_ssa/EDMW_SSA_CompressedFile.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/edmw_ssa/EDMW_SSA_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/edmw_ssa/EDMW_SSA_Format.hpp

	#g2_vdfs
	${CMAKE_CURRENT_SOURCE_DIR}/g2_vdfs/Gothic2_VDFS_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/g2_vdfs/Gothic2_VDFS_Format.hpp

	#g3_pak
	${CMAKE_CURRENT_SOURCE_DIR}/g3_pak/Gothic3_PAK_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/g3_pak/Gothic3_PAK_Format.hpp

	#scct_umd
	${CMAKE_CURRENT_SOURCE_DIR}/scct_umd/SCCT_UMD_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/scct_umd/SCCT_UMD_Format.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/scct_umd/UMD.hpp

	#.
	${CMAKE_CURRENT_SOURCE_DIR}/ACFSLib.cpp

	PARENT_SCOPE)