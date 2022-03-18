set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

set(DATA_DIR "${ROOT_DIR}/data/Data")
set(DATA_FILES
	"${DATA_DIR}/Interface/Translations/WhoseQuestIsItAnyway_english.txt"
	"${DATA_DIR}/Interface/Translations/WhoseQuestIsItAnyway_french.txt"
	"${DATA_DIR}/SKSE/Plugins/WhoseQuestIsItAnyway.toml"
)

source_group(TREE "${ROOT_DIR}" FILES ${DATA_FILES})

set(SOURCE_DIR "${ROOT_DIR}/src")
set(SOURCE_FILES
	"${SOURCE_DIR}/Hooks.cpp"
	"${SOURCE_DIR}/Hooks.h"
	"${SOURCE_DIR}/main.cpp"
	"${SOURCE_DIR}/Notification.cpp"
	"${SOURCE_DIR}/Notification.h"
	"${SOURCE_DIR}/PCH.h"
	"${SOURCE_DIR}/Settings.h"
)

source_group(TREE "${ROOT_DIR}" FILES ${SOURCE_FILES})

set(VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/Plugin.h")
configure_file(
	"${ROOT_DIR}/cmake/Plugin.h.in"
	"${VERSION_HEADER}"
	@ONLY
)

source_group("src" FILES "${VERSION_HEADER}")

configure_file(
	"${ROOT_DIR}/cmake/version.rc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	@ONLY
)

add_library(
	"${PROJECT_NAME}"
	SHARED
	${DATA_FILES}
	${SOURCE_FILES}
	"${VERSION_HEADER}"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	"${ROOT_DIR}/.clang-format"
	"${ROOT_DIR}/.editorconfig"
)

target_compile_features(
	"${PROJECT_NAME}"
	PRIVATE
		cxx_std_20
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_compile_options(
		"${PROJECT_NAME}"
		PRIVATE
			"/sdl"	# Enable Additional Security Checks
			"/utf-8"	# Set Source and Executable character sets to UTF-8
			"/Zi"	# Debug Information Format

			"/permissive-"	# Standards conformance
			"/Zc:preprocessor"	# Enable preprocessor conformance mode

			"/wd4200" # nonstandard extension used : zero-sized array in struct/union

			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		"${PROJECT_NAME}"
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

target_include_directories(
	"${PROJECT_NAME}"
	PRIVATE
		"${CMAKE_CURRENT_BINARY_DIR}/src"
		"${SOURCE_DIR}"
)

set(SKSE_SUPPORT_XBYAK ON)
add_subdirectory("${ROOT_DIR}/external/CommonLibSSE" CommonLibSSE EXCLUDE_FROM_ALL)

find_package(AutoTOML REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(xbyak REQUIRED CONFIG)

target_link_libraries(
	"${PROJECT_NAME}"
	PRIVATE
	AutoTOML::AutoTOML
		CommonLibSSE::CommonLibSSE
		fmt::fmt
		spdlog::spdlog
		xbyak::xbyak
)

target_precompile_headers(
	"${PROJECT_NAME}"
	PRIVATE
		"${SOURCE_DIR}/PCH.h"
)

install(
	DIRECTORY
		"${DATA_DIR}/Interface"
		"${DATA_DIR}/SKSE"
	DESTINATION "/"
	COMPONENT "main"
)

install(
	FILES
		"$<TARGET_FILE:${PROJECT_NAME}>"
	DESTINATION "SKSE/Plugins"
	COMPONENT "main"
)

install(
	FILES
		"$<TARGET_PDB_FILE:${PROJECT_NAME}>"
	DESTINATION "/"
	COMPONENT "pdbs"
)

if("${COPY_OUTPUT}")
	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		COMMAND
			"${CMAKE_COMMAND}"
				-E
				copy_if_different
				"$<TARGET_FILE:${PROJECT_NAME}>"
				"$<TARGET_PDB_FILE:${PROJECT_NAME}>"
				"${Skyrim64Path}/Data/SKSE/Plugins/"
		COMMAND
			"${CMAKE_COMMAND}"
				-E
				copy_directory
				${DATA_DIR}
				"${Skyrim64Path}/Data/"
		VERBATIM
	)
endif()
