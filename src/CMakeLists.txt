file(GLOB_RECURSE ENGINE_SOURCE *.cpp)
file(GLOB_RECURSE ENGINE_HEADERS *.h)

macro(GroupSources curdir)
	file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

    foreach(child ${children})

		if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			GroupSources(${curdir}/${child})
		else()
			string(REPLACE "/" "\\" groupname ${curdir})

			source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
		endif()
    endforeach()
endmacro()

GroupSources(src)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(../dependencies/)
LINK_DIRECTORIES(../dll/)

SET(PARTITION_DEPENDENCIES
	directXTK 
	assimp 
	dxguid 
	d3dcompiler 
	dxgi 
	d3d11
)

SET(${CXX_STANDARD_REQUIRED} ON)
ADD_LIBRARY(partition ${ENGINE_SOURCE} ${ENGINE_HEADERS})
TARGET_LINK_LIBRARIES(partition ${PARTITION_DEPENDENCIES})
SET_PROPERTY(TARGET partition PROPERTY CXX_STANDARD 17)