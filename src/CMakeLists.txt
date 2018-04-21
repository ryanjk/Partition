file(GLOB_RECURSE ENGINE_SOURCE *.cpp)
file(GLOB_RECURSE ENGINE_HEADERS *.h *.inc)

macro(rmv_src src)
	list(REMOVE_ITEM ENGINE_SOURCE  ${CMAKE_CURRENT_SOURCE_DIR}/${src}.cpp)
	list(REMOVE_ITEM ENGINE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${src}.h)
endmacro(rmv_src)

rmv_src(Graphics/DirectX)
rmv_src(Graphics/GBuffer)
rmv_src(Graphics/CBuffer)
rmv_src(Graphics/RenderSystem)
rmv_src(Graphics/TextureLoadUtil)
rmv_src(Graphics/DebugDraw)
rmv_src(Application/ResourceDatabase)
rmv_src(Component/render_data_t)
rmv_src(imgui/imgui)
rmv_src(imgui/imgui_demo)
rmv_src(imgui/imgui_draw)
rmv_src(imgui/imgui_impl_dx11)
rmv_src(UI/UIUtil)
rmv_src(UI/EditorUI)
rmv_src(UI/EditStruct)

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
	D3d12
	json11
)

SET(${CXX_STANDARD_REQUIRED} ON)
ADD_LIBRARY(Partition ${ENGINE_SOURCE} ${ENGINE_HEADERS})
TARGET_LINK_LIBRARIES(Partition PUBLIC ${PARTITION_DEPENDENCIES})
SET_PROPERTY(TARGET Partition PROPERTY CXX_STANDARD 17)