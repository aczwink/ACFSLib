set(SOURCE_FILES
	${SOURCE_FILES}

	#coh_sga
		${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_FileSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_Format.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_Format.hpp

	#ea_big
	${CMAKE_CURRENT_SOURCE_DIR}/ea_big/EA_BIG_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/ea_big/EA_BIG_Format.hpp

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