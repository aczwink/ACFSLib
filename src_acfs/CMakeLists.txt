set(SOURCE_FILES
	${SOURCE_FILES}

	#coh_sga
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_File.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_File.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_FileSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_Format.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/coh_sga/COH_SGA_Format.hpp

	#scct_umd
	${CMAKE_CURRENT_SOURCE_DIR}/scct_umd/SCCT_UMD_FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/scct_umd/SCCT_UMD_Format.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/scct_umd/UMD.hpp

	#.
	${CMAKE_CURRENT_SOURCE_DIR}/ACFSLib.cpp

	PARENT_SCOPE)