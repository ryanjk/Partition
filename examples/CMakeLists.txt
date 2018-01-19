
SET(${CXX_STANDARD_REQUIRED} ON)

LINK_DIRECTORIES(../dll/x64/Release)
LINK_DIRECTORIES("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.16299.0/um/x64")

INCLUDE_DIRECTORIES(../dependencies)
INCLUDE_DIRECTORIES(../src)

FUNCTION(CreateExample exampleName)
	ADD_EXECUTABLE(${exampleName} WIN32 ${exampleName}.cpp)
	TARGET_LINK_LIBRARIES(${exampleName} partition)
	SET_PROPERTY(TARGET ${exampleName} PROPERTY CXX_STANDARD 17)
	SET_PROPERTY(TARGET ${exampleName} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${exampleName}")
ENDFUNCTION(CreateExample)

CreateExample(DetailMapping)
CreateExample(WaterSim)
CreateExample(ImageEffect)