cmake_minimum_required(VERSION 3.30)

project(RetroRush)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
		SFML
		GIT_REPOSITORY https://github.com/SFML/SFML.git
		GIT_TAG 3.0.0
		GIT_SHALLOW ON
		EXCLUDE_FROM_ALL
		SYSTEM
)
FetchContent_MakeAvailable(SFML)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

set(SOURCES
		${CMAKE_SOURCE_DIR}/main.cpp
		${SOURCE_DIR}/Engine.cpp
		${SOURCE_DIR}/World.cpp
		${SOURCE_DIR}/Entity.cpp
		${SOURCE_DIR}/Car.cpp
		${SOURCE_DIR}/Track.cpp
		${SOURCE_DIR}/CheckpointManager.cpp
		${SOURCE_DIR}/AssetsManager.cpp
		${SOURCE_DIR}/Player.cpp
		${SOURCE_DIR}/CollisionMask.cpp
		${SOURCE_DIR}/Menu.cpp
		src/Hud.cpp
		src/Camera.cpp
		src/GameManager.cpp
)

set(HEADERS
		${INCLUDE_DIR}/Engine.h
		${INCLUDE_DIR}/World.h
		${INCLUDE_DIR}/Entity.h
		${INCLUDE_DIR}/Car.h
		${INCLUDE_DIR}/Track.h
		${INCLUDE_DIR}/CheckpointManager.h
		${INCLUDE_DIR}/AssetsManager.h
		${INCLUDE_DIR}/Player.h
		${INCLUDE_DIR}/Config.h
		${INCLUDE_DIR}/CollisionMask.h
		${INCLUDE_DIR}/Menu.h
		include/Hud.h
		include/Camera.h
		include/GameManager.h
		include/GhostManager.h
		src/GhostManager.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)