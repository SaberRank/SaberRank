# include some defines automatically made by qpm
include(qpm_defines.cmake)

cmake_minimum_required(VERSION 3.22)
project(${COMPILE_ID})

macro(RECURSE_FILES return_list)
    file(GLOB_RECURSE file_list CONFIGURE_DEPENDS ${ARGN})
    list(REMOVE_DUPLICATES file_list)
    set(${return_list} ${file_list})
endmacro()

set(SNORESABER_COMPILER_CACHE "$ENV{SNORESABER_COMPILER_CACHE}" CACHE STRING "Compiler cache to use: auto, off, ccache, sccache, or an executable path")
if(SNORESABER_COMPILER_CACHE STREQUAL "")
    set(SNORESABER_COMPILER_CACHE auto CACHE STRING "Compiler cache to use: auto, off, ccache, sccache, or an executable path" FORCE)
endif()

string(TOLOWER "${SNORESABER_COMPILER_CACHE}" SNORESABER_COMPILER_CACHE_MODE)
unset(SNORESABER_COMPILER_CACHE_PROGRAM CACHE)
if(SNORESABER_COMPILER_CACHE_MODE MATCHES "^(off|none|false|0)$")
    unset(CMAKE_C_COMPILER_LAUNCHER CACHE)
    unset(CMAKE_CXX_COMPILER_LAUNCHER CACHE)
elseif(SNORESABER_COMPILER_CACHE_MODE STREQUAL "auto")
    find_program(SNORESABER_COMPILER_CACHE_PROGRAM NAMES sccache ccache)
else()
    find_program(SNORESABER_COMPILER_CACHE_PROGRAM NAMES "${SNORESABER_COMPILER_CACHE}")
    if(NOT SNORESABER_COMPILER_CACHE_PROGRAM AND EXISTS "${SNORESABER_COMPILER_CACHE}")
        set(SNORESABER_COMPILER_CACHE_PROGRAM "${SNORESABER_COMPILER_CACHE}")
    endif()

    if(NOT SNORESABER_COMPILER_CACHE_PROGRAM)
        message(FATAL_ERROR "requested compiler cache '${SNORESABER_COMPILER_CACHE}' was not found")
    endif()
endif()

if(SNORESABER_COMPILER_CACHE_PROGRAM)
    message(STATUS "Using compiler cache: ${SNORESABER_COMPILER_CACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${SNORESABER_COMPILER_CACHE_PROGRAM}" CACHE FILEPATH "C compiler cache launcher" FORCE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SNORESABER_COMPILER_CACHE_PROGRAM}" CACHE FILEPATH "C++ compiler cache launcher" FORCE)
else()
    unset(CMAKE_C_COMPILER_LAUNCHER CACHE)
    unset(CMAKE_CXX_COMPILER_LAUNCHER CACHE)
endif()

# export compile commands for significantly better intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

# define that stores the actual source directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# compile options used
add_compile_options(-frtti -fPIE -fPIC -fexceptions)
add_compile_options(-O3)
add_compile_options(-Wbitwise-op-parentheses)
add_compile_options(-fdata-sections)
add_compile_options(-ffunction-sections)
add_compile_options(-fvisibility-inlines-hidden)
add_compile_options(-fvisibility=hidden)
add_compile_options(-DHAVE_NEON=1)
add_compile_options(-Wno-inaccessible-base)

# RELEASE FLAGS
add_compile_options(-DNDEBUG)

# compile definitions used
add_compile_definitions(VERSION=\"${MOD_VERSION}\")
add_compile_definitions(MOD_ID=\"${MOD_ID}\")

foreach(upload_trust_var
    SnoreSaberOfficialBuildId
    SnoreSaberOfficialBuildCredential
    SnoreSaberOfficialArtifactSha256
    SnoreSaberDevelopmentUploadToken
    SnoreSaberDevelopmentAuthNonce
    SnoreSaberDevelopmentPlayerId
    SnoreSaberDevelopmentPlayerName)
    if(NOT DEFINED ${upload_trust_var})
        set(${upload_trust_var} "$ENV{${upload_trust_var}}" CACHE STRING "SnoreSaber upload trust metadata")
    endif()
endforeach()

add_compile_definitions(SNORESABER_OFFICIAL_BUILD_ID=\"${SnoreSaberOfficialBuildId}\")
add_compile_definitions(SNORESABER_OFFICIAL_BUILD_CREDENTIAL=\"${SnoreSaberOfficialBuildCredential}\")
add_compile_definitions(SNORESABER_OFFICIAL_ARTIFACT_SHA256=\"${SnoreSaberOfficialArtifactSha256}\")
add_compile_definitions(SNORESABER_DEVELOPMENT_UPLOAD_TOKEN=\"${SnoreSaberDevelopmentUploadToken}\")
add_compile_definitions(SNORESABER_DEVELOPMENT_AUTH_NONCE=\"${SnoreSaberDevelopmentAuthNonce}\")
add_compile_definitions(SNORESABER_DEVELOPMENT_PLAYER_ID=\"${SnoreSaberDevelopmentPlayerId}\")
add_compile_definitions(SNORESABER_DEVELOPMENT_PLAYER_NAME=\"${SnoreSaberDevelopmentPlayerName}\")

# recursively get all src files
RECURSE_FILES(cpp_file_list ${SOURCE_DIR}/*.cpp)
RECURSE_FILES(c_file_list ${SOURCE_DIR}/*.c)

RECURSE_FILES(inline_hook_c ${EXTERN_DIR}/includes/beatsaber-hook/shared/inline-hook/*.c)
RECURSE_FILES(inline_hook_cpp ${EXTERN_DIR}/includes/beatsaber-hook/shared/inline-hook/*.cpp)

# add all src files to compile
add_library(
    ${COMPILE_ID}
    SHARED
    ${cpp_file_list}
    ${c_file_list}
    ${inline_hook_c}
    ${inline_hook_cpp}
)

# add src dir as include dir
target_include_directories(${COMPILE_ID} PRIVATE ${SOURCE_DIR})

# add include dir as include dir
target_include_directories(${COMPILE_ID} PRIVATE ${INCLUDE_DIR})

# add shared dir as include dir
target_include_directories(${COMPILE_ID} PUBLIC ${SHARED_DIR})

# codegen includes
target_include_directories(${COMPILE_ID} PRIVATE ${EXTERN_DIR}/includes/${CODEGEN_ID}/include)

target_link_libraries(${COMPILE_ID} PRIVATE -llog -lz)

# add extern stuff like libs and other includes
include(extern.cmake)

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
    COMMAND ${CMAKE_STRIP} -g -S -d --strip-all
    "lib${COMPILE_ID}.so" -o "stripped_lib${COMPILE_ID}.so"
    COMMENT "Strip debug symbols done on final binary.")

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory debug
    COMMENT "Make directory for debug symbols"
)

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename lib${COMPILE_ID}.so debug/lib${COMPILE_ID}.so
    COMMENT "Rename the lib to debug_ since it has debug symbols"
)

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename stripped_lib${COMPILE_ID}.so lib${COMPILE_ID}.so
    COMMENT "Rename the stripped lib to regular"
)

foreach(so_file ${so_list})
    cmake_path(GET so_file FILENAME file)

    add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${so_file} debug/${file}
        COMMENT "Copy so files for ndk stack"
    )

    add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
        COMMAND ${CMAKE_STRIP} -g -S -d --strip-all ${so_file} -o ${file}
        COMMENT "Strip debug symbols from the dependencies")
endforeach()

foreach(a_file ${a_list})
    cmake_path(GET a_file FILENAME file)

    add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${a_file} debug/${file}
        COMMENT "Copy a files for ndk stack")
endforeach()

include(assets_include.cmake)
