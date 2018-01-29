
SET(${CXX_STANDARD_REQUIRED} ON)

LINK_DIRECTORIES(../dll/x64/Release)
LINK_DIRECTORIES("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.16299.0/um/x64")

INCLUDE_DIRECTORIES(../dependencies)
INCLUDE_DIRECTORIES(../src)

FUNCTION(CreateExample exampleName)
	ADD_EXECUTABLE(${exampleName} WIN32 ${exampleName}.cpp)
	TARGET_LINK_LIBRARIES(${exampleName} Partition)

	SET_PROPERTY(TARGET ${exampleName} PROPERTY CXX_STANDARD 17)
	SET_PROPERTY(TARGET ${exampleName} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${exampleName}")
	SET_PROPERTY(TARGET ${exampleName} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
	
	ADD_CUSTOM_COMMAND(
        TARGET ${exampleName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/dll/x64/assimp-vc140-mt.dll
                $<TARGET_FILE_DIR:${exampleName}>/assimp-vc140-mt.dll)

	IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/partition.json)
		MESSAGE(SEND_ERROR "Missing configuration file `${CMAKE_SOURCE_DIR}/partition.json`. \n"
			"You can use `${CMAKE_SOURCE_DIR}/partition.config.json` as template ")
	ENDIF()

	ADD_CUSTOM_COMMAND(
        TARGET ${exampleName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/partition.json
                $<TARGET_FILE_DIR:${exampleName}>/partition.json)
				
	
ENDFUNCTION(CreateExample)

CreateExample(DetailMapping)
CreateExample(WaterSim)
CreateExample(ImageEffect)