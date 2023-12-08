cmake_minimum_required(VERSION 3.16)

project(bjorn-ai VERSION 0.0.1 DESCRIPTION "Bjorn's AI MERVBot plugin")

# the plugin author's e-mail (gets embedded into the bot's !version output)
set(PROJECT_AUTHOR "bjorn@djurner.net")

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(dep/mervbot)

add_library(${PROJECT_NAME} SHARED
	src/command.cpp
	src/spawn.cpp
	dep/mervbot/src/dllcore.cpp
	dep/mervbot/src/datatypes.cpp
	dep/mervbot/src/algorithms.cpp
	dep/mervbot/src/player.cpp
	dep/mervbot/src/commtypes.cpp
	${CMAKE_CURRENT_BINARY_DIR}/src/pluginmeta.cpp
)

configure_file(src/pluginmeta.cpp.in src/pluginmeta.cpp.pregen)
file(GENERATE OUTPUT src/pluginmeta.cpp INPUT ${CMAKE_CURRENT_BINARY_DIR}/src/pluginmeta.cpp.pregen)

target_compile_definitions(${PROJECT_NAME} PRIVATE STRING_CAST_CHAR)
if (UNIX)
	target_sources(${PROJECT_NAME} PRIVATE dep/mervbot/src/windows_compat.cpp)

	target_compile_definitions(${PROJECT_NAME} PRIVATE __stdcall=)
	target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE dep/mervbot/src)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
#set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER source/convert.h)

add_custom_target(
	${PROJECT_NAME}-config ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/cfg/AI.ini ${CMAKE_CURRENT_SOURCE_DIR}/dep/mervbot/cfg/MERVBot.INI ${CMAKE_CURRENT_SOURCE_DIR}/dep/mervbot/cfg/Commands.txt ${CMAKE_CURRENT_SOURCE_DIR}/dep/mervbot/cfg/Operators.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying configuration files to plugin output directory..."
)

if (WIN32)
	# see https://github.com/ament/ament_cmake/issues/350
	add_custom_target(
		${PROJECT_NAME}-symlink ALL
		COMMAND ${CMAKE_COMMAND} -E create_hardlink $<TARGET_FILE:mervbot> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:mervbot>
		COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/lvl make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/get
		COMMENT "Symlinking MERVBot into plugin output directory..."
	)

	file(GENERATE OUTPUT $<TARGET_FILE_DIR:${PROJECT_NAME}>/Spawns.txt INPUT ${CMAKE_CURRENT_SOURCE_DIR}/cfg/Spawns_Windows.txt)
else()
	add_custom_target(
		${PROJECT_NAME}-symlink ALL
		COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:mervbot> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:mervbot>
		COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/lvl $<TARGET_FILE_DIR:${PROJECT_NAME}>/get
		COMMENT "Symlinking MERVBot into plugin output directory..."
	)

	file(GENERATE OUTPUT $<TARGET_FILE_DIR:${PROJECT_NAME}>/Spawns.txt INPUT ${CMAKE_CURRENT_SOURCE_DIR}/cfg/Spawns_Linux.txt)
endif()
