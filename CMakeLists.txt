cmake_minimum_required(VERSION 3.16)
project(Rasterizer)

add_subdirectory(deps/glm)

find_package(SDL2 REQUIRED)
find_package(TINYOBJLOADER REQUIRED)

include_directories(
		"deps/stb"
		${SDL2_INCLUDE_DIRS}
		${TINYOBJLOADER_INCLUDE_DIR}
)

add_executable(Rasterizer main.cpp Texture.cpp)

target_link_libraries(Rasterizer 	
	${SDL2_LIBRARIES}
)

set(OuputFile "$<TARGET_FILE_DIR:Rasterizer>")

