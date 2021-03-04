cmake_minimum_required(VERSION 3.18)
project(Rasterizer)

add_subdirectory(deps/sdl2)
add_subdirectory(deps/glm)
add_subdirectory(deps/tinyobjloader)

include_directories(
deps/sdl2/include
deps/glm
deps/stb
deps/tinyobjloader
)


add_executable(Rasterizer main.cpp Texture.cpp)

target_link_libraries(Rasterizer 
	deps/sdl2/Debug/SDL2d	
	deps/sdl2/Debug/SDL2maind	
	deps/tinyobjloader/Debug/tinyobjloader	
)

#SDL static deps : imagehlp dinput8 dxguid user32 gdi32 winmm imm32 ole32 oleaut32 shell32 version uuid Setupapi

set(OuputFile "$<TARGET_FILE_DIR:Rasterizer>")

#configure_file(${PROJECT_BINARY_DIR}/deps/sdl2/Debug/SDL2d.dll ${OutputFile}  COPYONLY)

